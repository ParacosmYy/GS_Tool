"""Atomic, bounded JSON storage for explicit descriptor approvals."""

import json
import os
import re
import tempfile
from pathlib import Path

from ..application.ports import PluginApprovalReadResult, PluginApprovalRecord, PluginApprovalStore
from ..plugins.api import PLUGIN_ID_PATTERN, PLUGIN_MANIFEST_MAX_FIELD_LENGTH

PLUGIN_APPROVAL_SCHEMA_VERSION = 1
DEFAULT_PLUGIN_APPROVAL_MAX_RECORDS = 256
DEFAULT_PLUGIN_APPROVAL_MAX_BYTES = 64 * 1024
_DIGEST_PATTERN = re.compile(r"^[0-9a-f]{64}$")
_RECORD_KEYS = frozenset({"plugin_id", "descriptor_sha256", "approved_at_ns"})


class JsonPluginApprovalStore(PluginApprovalStore):
    """Persist only digest-bound approvals; unreadable state fails closed."""

    def __init__(
        self,
        path: Path,
        *,
        max_records: int = DEFAULT_PLUGIN_APPROVAL_MAX_RECORDS,
        max_bytes: int = DEFAULT_PLUGIN_APPROVAL_MAX_BYTES,
    ) -> None:
        if max_records < 1:
            raise ValueError("Plugin approval record limit must be positive")
        if max_bytes < 1:
            raise ValueError("Plugin approval byte limit must be positive")
        self._path = path.expanduser().resolve()
        self._max_records = max_records
        self._max_bytes = max_bytes

    def load(self) -> PluginApprovalReadResult:
        """Read a valid ledger or return no approvals with a diagnostic."""
        if not self._path.exists():
            return PluginApprovalReadResult(())
        try:
            with self._path.open("rb") as source:
                raw_payload = source.read(self._max_bytes + 1)
            if len(raw_payload) > self._max_bytes:
                return PluginApprovalReadResult((), self._oversize_message())
            payload = json.loads(raw_payload)
            records = _decode_records(payload, max_records=self._max_records)
        except (OSError, UnicodeError, TypeError, ValueError, json.JSONDecodeError) as error:
            return PluginApprovalReadResult((), f"Approval ledger is invalid: {error}")
        return PluginApprovalReadResult(records)

    def save(self, records: tuple[PluginApprovalRecord, ...]) -> None:
        """Atomically replace the complete bounded ledger."""
        validated = _validate_records(records, max_records=self._max_records)
        payload = {
            "schema_version": PLUGIN_APPROVAL_SCHEMA_VERSION,
            "approvals": [
                {
                    "plugin_id": record.plugin_id,
                    "descriptor_sha256": record.descriptor_sha256,
                    "approved_at_ns": record.approved_at_ns,
                }
                for record in validated
            ],
        }
        self._path.parent.mkdir(parents=True, exist_ok=True)
        temporary_path: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                dir=self._path.parent,
                prefix=f".{self._path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary_path = Path(temporary.name)
                json.dump(payload, temporary, ensure_ascii=False, separators=(",", ":"))
                temporary.flush()
                os.fsync(temporary.fileno())
            if temporary_path.stat().st_size > self._max_bytes:
                raise ValueError(self._oversize_message())
            os.replace(temporary_path, self._path)
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass

    def _oversize_message(self) -> str:
        return f"Approval ledger exceeds {self._max_bytes} bytes"


def default_plugin_approval_path() -> Path:
    """Return the user-local approval path without creating it."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / "plugin-approvals.json"


def _decode_records(payload: object, *, max_records: int) -> tuple[PluginApprovalRecord, ...]:
    if not isinstance(payload, dict) or set(payload) != {"schema_version", "approvals"}:
        raise ValueError("Approval ledger schema is invalid")
    if payload["schema_version"] != PLUGIN_APPROVAL_SCHEMA_VERSION:
        raise ValueError(f"Unsupported approval ledger schema: {payload['schema_version']!r}")
    approvals = payload["approvals"]
    if not isinstance(approvals, list):
        raise ValueError("Approval ledger approvals must be an array")
    records = []
    for item in approvals:
        if not isinstance(item, dict) or set(item) != _RECORD_KEYS:
            raise ValueError("Approval record schema is invalid")
        plugin_id = item["plugin_id"]
        digest = item["descriptor_sha256"]
        approved_at_ns = item["approved_at_ns"]
        if (
            type(plugin_id) is not str
            or not plugin_id
            or len(plugin_id) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
            or PLUGIN_ID_PATTERN.fullmatch(plugin_id) is None
        ):
            raise ValueError("Approval record plugin ID is invalid")
        if type(digest) is not str or _DIGEST_PATTERN.fullmatch(digest) is None:
            raise ValueError("Approval record descriptor digest is invalid")
        if type(approved_at_ns) is not int or approved_at_ns < 0:
            raise ValueError("Approval record timestamp is invalid")
        records.append(PluginApprovalRecord(plugin_id, digest, approved_at_ns))
    return _validate_records(tuple(records), max_records=max_records)


def _validate_records(
    records: tuple[PluginApprovalRecord, ...],
    *,
    max_records: int,
) -> tuple[PluginApprovalRecord, ...]:
    if not isinstance(records, tuple):
        raise ValueError("Approval records must be a tuple")
    if len(records) > max_records:
        raise ValueError(f"Approval ledger exceeds {max_records} records")
    keys: set[tuple[str, str]] = set()
    for record in records:
        if not isinstance(record, PluginApprovalRecord):
            raise ValueError("Approval record type is invalid")
        if (
            type(record.plugin_id) is not str
            or not record.plugin_id
            or len(record.plugin_id) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
            or PLUGIN_ID_PATTERN.fullmatch(record.plugin_id) is None
        ):
            raise ValueError("Approval record plugin ID is invalid")
        if (
            type(record.descriptor_sha256) is not str
            or _DIGEST_PATTERN.fullmatch(record.descriptor_sha256) is None
        ):
            raise ValueError("Approval record descriptor digest is invalid")
        if type(record.approved_at_ns) is not int or record.approved_at_ns < 0:
            raise ValueError("Approval record timestamp is invalid")
        key = (record.plugin_id, record.descriptor_sha256)
        if key in keys:
            raise ValueError("Approval records must not contain duplicates")
        keys.add(key)
    return tuple(sorted(records, key=lambda record: (record.plugin_id, record.descriptor_sha256)))
