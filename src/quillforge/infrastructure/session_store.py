"""Atomic bounded JSON storage for local editor-session continuity."""

import json
import os
import tempfile
from pathlib import Path

from ..application.ports import SessionLoadResult, SessionStore
from ..application.session import (
    CURRENT_SESSION_VERSION,
    MAX_SESSION_DOCUMENTS,
    MAX_SESSION_PATH_CHARS,
)
from ..domain.models import SessionDocument, SessionSnapshot

_MAX_SESSION_BYTES = 65_536


class JsonSessionStore(SessionStore):
    """Persist a small path-only session manifest in the user-local app directory."""

    def __init__(self, path: Path, *, max_bytes: int = _MAX_SESSION_BYTES) -> None:
        if max_bytes < 1:
            raise ValueError("Session size bound must be positive")
        self._path = path.expanduser().resolve()
        self._max_bytes = max_bytes

    def load(self) -> SessionLoadResult:
        """Read one bounded manifest and preserve invalid-state provenance."""
        try:
            with self._path.open("rb") as session_file:
                raw = session_file.read(self._max_bytes + 1)
            if len(raw) > self._max_bytes:
                return SessionLoadResult("invalid")
            return SessionLoadResult(
                "valid",
                _decode_session(json.loads(raw.decode("utf-8"))),
            )
        except FileNotFoundError:
            return SessionLoadResult("absent")
        except (OSError, UnicodeError, TypeError, ValueError):
            return SessionLoadResult("invalid")

    def save(self, session: SessionSnapshot) -> None:
        """Flush a bounded JSON manifest and replace the final path atomically."""
        payload = _encode_session(session)
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
                raise ValueError("Session manifest exceeds its configured byte bound")
            os.replace(temporary_path, self._path)
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass


def default_session_path() -> Path:
    """Return the local session path without depending on Qt."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / "session.json"


def _encode_session(session: SessionSnapshot) -> dict[str, object]:
    return {
        "schema_version": session.schema_version,
        "workspace_root": str(session.workspace_root)
        if session.workspace_root is not None
        else None,
        "documents": [
            {"path": str(document.path), "line": document.line, "column": document.column}
            for document in session.documents
        ],
        "active_index": session.active_index,
    }


def _decode_session(payload: object) -> SessionSnapshot:
    if not isinstance(payload, dict) or payload.get("schema_version") != CURRENT_SESSION_VERSION:
        raise ValueError("Unsupported session manifest")
    workspace_value = payload.get("workspace_root")
    workspace_root = _decode_path(workspace_value, allow_none=True)
    raw_documents = payload.get("documents")
    if not isinstance(raw_documents, list) or len(raw_documents) > MAX_SESSION_DOCUMENTS:
        raise ValueError("Invalid session document list")
    documents: list[SessionDocument] = []
    for raw_document in raw_documents:
        if not isinstance(raw_document, dict):
            raise ValueError("Invalid session document")
        path = _decode_path(raw_document.get("path"), allow_none=False)
        line = raw_document.get("line")
        column = raw_document.get("column")
        if type(line) is not int or line < 0 or type(column) is not int or column < 0:
            raise ValueError("Invalid session cursor")
        documents.append(SessionDocument(path, line, column))
    active_index = payload.get("active_index")
    if type(active_index) is not int or active_index < 0:
        raise ValueError("Invalid session active index")
    return SessionSnapshot(
        schema_version=CURRENT_SESSION_VERSION,
        workspace_root=workspace_root,
        documents=tuple(documents),
        active_index=active_index,
    )


def _decode_path(value: object, *, allow_none: bool) -> Path | None:
    if value is None and allow_none:
        return None
    if not isinstance(value, str) or not value or len(value) > MAX_SESSION_PATH_CHARS:
        raise ValueError("Invalid session path")
    path = Path(value)
    if not path.is_absolute():
        raise ValueError("Session paths must be absolute")
    return path
