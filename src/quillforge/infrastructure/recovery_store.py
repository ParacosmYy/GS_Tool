"""Atomic JSON recovery snapshot storage."""

import json
import os
import tempfile
from collections.abc import Callable, Iterable
from pathlib import Path
from typing import TextIO

from ..application.ports import RecoverySnapshotChunkStore, RecoverySnapshotStore
from ..domain.models import DocumentRevision, RecoverySnapshot, RecoverySnapshotMetadata

_SCHEMA_VERSION = 1
_SNAPSHOT_SUFFIX = ".qfrecovery"


class JsonRecoverySnapshotStore(RecoverySnapshotStore, RecoverySnapshotChunkStore):
    """Store one self-contained snapshot per file in an app-owned directory."""

    def __init__(self, root: Path) -> None:
        self._root = root.expanduser().resolve()

    def list_snapshots(self) -> tuple[RecoverySnapshot, ...]:
        """Read valid snapshots and leave malformed files untouched for diagnosis."""
        if not self._root.exists():
            return ()
        snapshots: list[RecoverySnapshot] = []
        for path in sorted(self._root.glob(f"*{_SNAPSHOT_SUFFIX}")):
            try:
                snapshot = _decode_snapshot(json.loads(path.read_text(encoding="utf-8")))
                expected_id = path.name[: -len(_SNAPSHOT_SUFFIX)]
                if snapshot.snapshot_id != expected_id:
                    continue
                snapshots.append(snapshot)
            except (OSError, UnicodeError, TypeError, ValueError, json.JSONDecodeError):
                continue
        snapshots.sort(key=lambda snapshot: snapshot.created_at_ns, reverse=True)
        return tuple(snapshots)

    def save_snapshot(self, snapshot: RecoverySnapshot) -> None:
        """Write JSON beside the final file, flush it, then replace atomically."""
        self._save_atomically(
            snapshot.snapshot_id,
            lambda temporary: json.dump(
                _encode_snapshot(snapshot), temporary, ensure_ascii=False, separators=(",", ":")
            ),
        )

    def save_snapshot_chunks(
        self,
        metadata: RecoverySnapshotMetadata,
        chunks: Iterable[str],
    ) -> None:
        """Write JSON text incrementally without joining all chunks first."""
        self._save_atomically(
            metadata.snapshot_id,
            lambda temporary: _write_snapshot_chunks(temporary, metadata, chunks),
        )

    def _save_atomically(self, snapshot_id: str, writer: Callable[[TextIO], None]) -> None:
        """Run one writer against a same-directory temporary file and replace atomically."""
        self._root.mkdir(parents=True, exist_ok=True)
        target = self._path_for(snapshot_id)
        temporary_path: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                dir=self._root,
                prefix=f".{target.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary_path = Path(temporary.name)
                writer(temporary)
                temporary.flush()
                os.fsync(temporary.fileno())
            os.replace(temporary_path, target)
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass

    def delete_snapshot(self, snapshot_id: str) -> None:
        """Delete only the validated snapshot path; missing files are already clean."""
        path = self._path_for(snapshot_id)
        try:
            path.unlink()
        except FileNotFoundError:
            pass

    def _path_for(self, snapshot_id: str) -> Path:
        if not snapshot_id or Path(snapshot_id).name != snapshot_id:
            raise ValueError("Invalid recovery snapshot ID")
        return self._root / f"{snapshot_id}{_SNAPSHOT_SUFFIX}"


def default_recovery_directory() -> Path:
    """Return the per-user recovery directory without depending on Qt."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / "recovery"


def _encode_snapshot(snapshot: RecoverySnapshot) -> dict[str, object]:
    revision = snapshot.source_revision
    return {
        "schema_version": _SCHEMA_VERSION,
        "snapshot_id": snapshot.snapshot_id,
        "document_id": snapshot.document_id,
        "path": str(snapshot.path) if snapshot.path is not None else None,
        "text": snapshot.text,
        "encoding": snapshot.encoding,
        "line_ending": snapshot.line_ending,
        "source_revision": (
            {"modified_ns": revision.modified_ns, "size": revision.size}
            if revision is not None
            else None
        ),
        "created_at_ns": snapshot.created_at_ns,
    }


def _write_snapshot_chunks(
    temporary: TextIO,
    metadata: RecoverySnapshotMetadata,
    chunks: Iterable[str],
) -> None:
    """Encode one JSON string from independent chunks without concatenating them."""
    revision = metadata.source_revision
    header = {
        "schema_version": _SCHEMA_VERSION,
        "snapshot_id": metadata.snapshot_id,
        "document_id": metadata.document_id,
        "path": str(metadata.path) if metadata.path is not None else None,
        "encoding": metadata.encoding,
        "line_ending": metadata.line_ending,
        "source_revision": (
            {"modified_ns": revision.modified_ns, "size": revision.size}
            if revision is not None
            else None
        ),
        "created_at_ns": metadata.created_at_ns,
    }
    encoded_header = json.dumps(header, ensure_ascii=False, separators=(",", ":"))
    temporary.write(encoded_header[:-1])
    temporary.write(',"text":"')
    for chunk in chunks:
        if not isinstance(chunk, str):
            raise TypeError("Recovery snapshot chunks must be strings")
        encoded_chunk = json.dumps(chunk, ensure_ascii=False)
        temporary.write(encoded_chunk[1:-1])
    temporary.write('"}')


def _decode_snapshot(payload: object) -> RecoverySnapshot:
    if not isinstance(payload, dict) or payload.get("schema_version") != _SCHEMA_VERSION:
        raise ValueError("Unsupported recovery snapshot")
    snapshot_id = _required_string(payload, "snapshot_id")
    if Path(snapshot_id).name != snapshot_id:
        raise ValueError("Invalid recovery snapshot ID")
    document_id = _required_string(payload, "document_id")
    path_value = payload.get("path")
    if path_value is not None and not isinstance(path_value, str):
        raise ValueError("Invalid recovery path")
    text = payload.get("text")
    if not isinstance(text, str):
        raise ValueError("Invalid recovery text")
    encoding = _required_string(payload, "encoding")
    line_ending = payload.get("line_ending")
    if line_ending not in {"LF", "CRLF", "CR"}:
        raise ValueError("Invalid recovery line ending")
    source_revision = _decode_revision(payload.get("source_revision"))
    created_at_ns = payload.get("created_at_ns")
    if not isinstance(created_at_ns, int) or created_at_ns < 0:
        raise ValueError("Invalid recovery timestamp")
    return RecoverySnapshot(
        snapshot_id=snapshot_id,
        document_id=document_id,
        path=Path(path_value) if path_value is not None else None,
        text=text,
        encoding=encoding,
        line_ending=line_ending,
        source_revision=source_revision,
        created_at_ns=created_at_ns,
    )


def _required_string(payload: dict[str, object], key: str) -> str:
    value = payload.get(key)
    if not isinstance(value, str) or not value:
        raise ValueError(f"Invalid recovery field: {key}")
    return value


def _decode_revision(value: object) -> DocumentRevision | None:
    if value is None:
        return None
    if not isinstance(value, dict):
        raise ValueError("Invalid recovery revision")
    modified_ns = value.get("modified_ns")
    size = value.get("size")
    if not isinstance(modified_ns, int) or not isinstance(size, int):
        raise ValueError("Invalid recovery revision values")
    return DocumentRevision(modified_ns=modified_ns, size=size)
