"""Windows DPAPI-protected retry storage for local Gateway usage reports.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Persist only encrypted, bounded usage DTOs across Gateway restarts.
Module: Local integration / encrypted queue infrastructure
"""

from __future__ import annotations

from contextlib import contextmanager
import ctypes
from dataclasses import dataclass
import json
import os
from pathlib import Path
import sqlite3
from typing import Iterator

from .gateway_contracts import UsageReport


MAX_MODEL_LENGTH = 200
MAX_IDEMPOTENCY_KEY_LENGTH = 160
MAX_NOTE_LENGTH = 200
MAX_ENCRYPTED_PAYLOAD_BYTES = 16 * 1024
QUEUE_SCHEMA_VERSION = "windows-dpapi-v1"
_CRYPTPROTECT_UI_FORBIDDEN = 0x1


class QueueProtectionError(RuntimeError):
    """Raised when the platform cannot protect or recover queue data safely."""


class QueueStorageError(RuntimeError):
    """Raised when the encrypted queue cannot be opened or updated safely."""


@dataclass(frozen=True)
class QueueItem:
    """A decrypted queue item held only for one delivery attempt."""

    item_id: int
    report: UsageReport
    attempts: int
    next_attempt_at: float


class EncryptedUsageQueue:
    """Store retry payloads in SQLite after Windows-user DPAPI protection."""

    def __init__(self, path: str | os.PathLike[str], max_pending: int) -> None:
        if os.name != "nt":
            raise QueueProtectionError("持久化 Gateway 队列需要 Windows DPAPI；请显式使用内存队列")
        self._path = _normalize_path(path)
        self._max_pending = _positive_bound(max_pending, "max_pending", 10000)
        try:
            self._path.parent.mkdir(parents=True, exist_ok=True)
            self._initialize()
        except QueueProtectionError:
            raise
        except (OSError, sqlite3.Error) as exc:
            raise QueueStorageError("Gateway 加密队列无法初始化") from exc

    @property
    def pending_count(self) -> int:
        """Return a bounded operational count without decrypting payloads."""

        with self._connection() as connection:
            row = connection.execute("SELECT COUNT(*) FROM gateway_usage_queue").fetchone()
        return int(row[0]) if row else 0

    def enqueue(self, report: UsageReport, attempts: int, next_attempt_at: float) -> bool:
        """Encrypt and append one report unless the bounded queue is full."""

        payload = _protect_payload(report)
        with self._connection() as connection:
            row = connection.execute("SELECT COUNT(*) FROM gateway_usage_queue").fetchone()
            if row and int(row[0]) >= self._max_pending:
                return False
            connection.execute(
                """
                INSERT INTO gateway_usage_queue (attempts, next_attempt_at, payload)
                VALUES (?, ?, ?)
                """,
                (_positive_bound(attempts, "attempts", 20), float(next_attempt_at), payload),
            )
            connection.commit()
        return True

    def next_due(self, now: float) -> QueueItem | None:
        """Read and decrypt the oldest item whose retry time has arrived."""

        with self._connection() as connection:
            row = connection.execute(
                """
                SELECT id, attempts, next_attempt_at, payload
                FROM gateway_usage_queue
                WHERE next_attempt_at <= ?
                ORDER BY id ASC
                LIMIT 1
                """,
                (float(now),),
            ).fetchone()
        if row is None:
            return None
        return QueueItem(
            item_id=int(row[0]),
            attempts=int(row[1]),
            next_attempt_at=float(row[2]),
            report=_decode_payload(bytes(row[3])),
        )

    def next_attempt_at(self) -> float | None:
        """Return the next scheduled epoch without exposing report contents."""

        with self._connection() as connection:
            row = connection.execute("SELECT MIN(next_attempt_at) FROM gateway_usage_queue").fetchone()
        if not row or row[0] is None:
            return None
        return float(row[0])

    def remove(self, item_id: int) -> None:
        """Delete an item only after delivery or an explicit terminal failure."""

        with self._connection() as connection:
            connection.execute("DELETE FROM gateway_usage_queue WHERE id = ?", (int(item_id),))
            connection.commit()

    def reschedule(self, item_id: int, attempts: int, next_attempt_at: float) -> None:
        """Persist retry state before the worker waits or the process exits."""

        with self._connection() as connection:
            connection.execute(
                """
                UPDATE gateway_usage_queue
                SET attempts = ?, next_attempt_at = ?
                WHERE id = ?
                """,
                (_positive_bound(attempts, "attempts", 20), float(next_attempt_at), int(item_id)),
            )
            connection.commit()

    def _initialize(self) -> None:
        with self._connection() as connection:
            connection.executescript(
                """
                CREATE TABLE IF NOT EXISTS gateway_queue_meta (
                    key TEXT PRIMARY KEY,
                    value TEXT NOT NULL
                );
                CREATE TABLE IF NOT EXISTS gateway_usage_queue (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    attempts INTEGER NOT NULL,
                    next_attempt_at REAL NOT NULL,
                    payload BLOB NOT NULL
                );
                CREATE INDEX IF NOT EXISTS idx_gateway_usage_queue_due
                    ON gateway_usage_queue (next_attempt_at, id);
                """
            )
            row = connection.execute(
                "SELECT value FROM gateway_queue_meta WHERE key = 'schema'").fetchone()
            if row is None:
                connection.execute(
                    "INSERT INTO gateway_queue_meta (key, value) VALUES ('schema', ?)",
                    (QUEUE_SCHEMA_VERSION,),
                )
            elif row[0] != QUEUE_SCHEMA_VERSION:
                raise QueueStorageError("Gateway 队列保护版本不受支持")
            connection.commit()
            rows = connection.execute("SELECT payload FROM gateway_usage_queue").fetchall()
        for row in rows:
            _decode_payload(bytes(row[0]))

    @contextmanager
    def _connection(self) -> Iterator[sqlite3.Connection]:
        try:
            connection = sqlite3.connect(self._path, timeout=5.0)
            connection.execute("PRAGMA busy_timeout = 5000")
        except sqlite3.Error as exc:
            raise QueueStorageError("Gateway 加密队列无法打开") from exc
        try:
            yield connection
        except sqlite3.Error as exc:
            raise QueueStorageError("Gateway 加密队列操作失败") from exc
        finally:
            connection.close()


def default_queue_path() -> Path:
    """Return the ignored, project-local queue path used by the Windows CLI."""

    return Path(__file__).resolve().parents[1] / "data" / "gateway-usage-queue.sqlite3"


def _protect_payload(report: UsageReport) -> bytes:
    encoded = json.dumps(report.to_payload(), ensure_ascii=False, separators=(",", ":")).encode("utf-8")
    protected = _dpapi_protect(encoded)
    if len(protected) > MAX_ENCRYPTED_PAYLOAD_BYTES:
        raise QueueProtectionError("Gateway 队列 payload 超过安全上限")
    return protected


def _decode_payload(payload: bytes) -> UsageReport:
    if not payload or len(payload) > MAX_ENCRYPTED_PAYLOAD_BYTES:
        raise QueueProtectionError("Gateway 队列 payload 无效")
    try:
        decoded = json.loads(_dpapi_unprotect(payload).decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError, QueueProtectionError) as exc:
        raise QueueProtectionError("Gateway 队列 payload 无法解密") from exc
    if not isinstance(decoded, dict):
        raise QueueProtectionError("Gateway 队列 payload 结构无效")
    model = decoded.get("model")
    key = decoded.get("idempotency_key")
    note = decoded.get("note", "本地 Gateway 自动采集")
    input_tokens = decoded.get("input_tokens")
    output_tokens = decoded.get("output_tokens")
    if not isinstance(model, str) or not 1 <= len(model) <= MAX_MODEL_LENGTH:
        raise QueueProtectionError("Gateway 队列 model 无效")
    if not isinstance(key, str) or not 1 <= len(key) <= MAX_IDEMPOTENCY_KEY_LENGTH:
        raise QueueProtectionError("Gateway 队列 idempotency_key 无效")
    if not isinstance(note, str) or len(note) > MAX_NOTE_LENGTH:
        raise QueueProtectionError("Gateway 队列 note 无效")
    if not _is_non_negative_int(input_tokens) or not _is_non_negative_int(output_tokens):
        raise QueueProtectionError("Gateway 队列 token 数无效")
    return UsageReport(model, input_tokens, output_tokens, key, note)


def _dpapi_protect(payload: bytes) -> bytes:
    crypt32, kernel32 = _windows_crypto()
    source, source_buffer = _data_blob(payload)
    target = _DataBlob()
    if not crypt32.CryptProtectData(
        ctypes.byref(source), None, None, None, None, _CRYPTPROTECT_UI_FORBIDDEN, ctypes.byref(target)
    ):
        raise QueueProtectionError(f"Windows DPAPI protect failed ({ctypes.get_last_error()})")
    try:
        return ctypes.string_at(target.pbData, target.cbData)
    finally:
        kernel32.LocalFree(target.pbData)


def _dpapi_unprotect(payload: bytes) -> bytes:
    crypt32, kernel32 = _windows_crypto()
    source, source_buffer = _data_blob(payload)
    target = _DataBlob()
    if not crypt32.CryptUnprotectData(
        ctypes.byref(source), None, None, None, None, _CRYPTPROTECT_UI_FORBIDDEN, ctypes.byref(target)
    ):
        raise QueueProtectionError(f"Windows DPAPI unprotect failed ({ctypes.get_last_error()})")
    try:
        return ctypes.string_at(target.pbData, target.cbData)
    finally:
        kernel32.LocalFree(target.pbData)


class _DataBlob(ctypes.Structure):
    _fields_ = [("cbData", ctypes.c_uint32), ("pbData", ctypes.POINTER(ctypes.c_byte))]


def _windows_crypto() -> tuple[ctypes.WinDLL, ctypes.WinDLL]:
    if os.name != "nt":
        raise QueueProtectionError("Windows DPAPI is unavailable")
    crypt32 = ctypes.WinDLL("Crypt32.dll", use_last_error=True)
    kernel32 = ctypes.WinDLL("Kernel32.dll", use_last_error=True)
    blob_pointer = ctypes.POINTER(_DataBlob)
    crypt32.CryptProtectData.argtypes = [blob_pointer, ctypes.c_wchar_p, blob_pointer, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32, blob_pointer]
    crypt32.CryptProtectData.restype = ctypes.c_int
    crypt32.CryptUnprotectData.argtypes = [blob_pointer, ctypes.c_void_p, blob_pointer, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32, blob_pointer]
    crypt32.CryptUnprotectData.restype = ctypes.c_int
    kernel32.LocalFree.argtypes = [ctypes.c_void_p]
    kernel32.LocalFree.restype = ctypes.c_void_p
    return crypt32, kernel32


def _data_blob(payload: bytes) -> tuple[_DataBlob, ctypes.Array[ctypes.c_char]]:
    buffer = ctypes.create_string_buffer(payload)
    blob = _DataBlob(len(payload), ctypes.cast(buffer, ctypes.POINTER(ctypes.c_byte)))
    return blob, buffer


def _normalize_path(value: str | os.PathLike[str]) -> Path:
    text = os.fspath(value)
    if "\x00" in text:
        raise QueueStorageError("Gateway 队列路径包含非法字符")
    path = Path(text).expanduser()
    if not path.is_absolute():
        path = (Path.cwd() / path).resolve()
    if path.exists() and path.is_dir():
        raise QueueStorageError("Gateway 队列路径不能是目录")
    return path


def _positive_bound(value: int, field_name: str, maximum: int) -> int:
    try:
        parsed = int(value)
    except (TypeError, ValueError) as exc:
        raise QueueStorageError(f"{field_name} must be a positive integer") from exc
    if not 1 <= parsed <= maximum:
        raise QueueStorageError(f"{field_name} must be between 1 and {maximum}")
    return parsed


def _is_non_negative_int(value: object) -> bool:
    return isinstance(value, int) and not isinstance(value, bool) and value >= 0
