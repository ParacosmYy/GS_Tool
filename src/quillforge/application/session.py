"""Local editor-session continuity use cases."""

from pathlib import Path

from ..domain.models import SESSION_SCHEMA_VERSION, SessionDocument, SessionSnapshot
from ..domain.path_identity import path_key
from .ports import SessionLoadResult, SessionStore

CURRENT_SESSION_VERSION = SESSION_SCHEMA_VERSION
MAX_SESSION_DOCUMENTS = 32
MAX_SESSION_PATH_CHARS = 4096
MAX_SESSION_CURSOR_LINE = 10_000_000
MAX_SESSION_CURSOR_COLUMN = 1_000_000
DEFAULT_SESSION = SessionSnapshot(CURRENT_SESSION_VERSION)


class SessionService:
    """Validate and persist bounded UI continuity without owning document text."""

    def __init__(self, store: SessionStore) -> None:
        self._store = store

    def load(self) -> SessionLoadResult:
        """Return normalized state without allowing bad input to be overwritten."""
        try:
            candidate = self._store.load()
        except (OSError, TypeError, ValueError):
            return SessionLoadResult("invalid")
        if not isinstance(candidate, SessionLoadResult):
            return SessionLoadResult("invalid")
        if candidate.state != "valid" or not isinstance(candidate.snapshot, SessionSnapshot):
            return SessionLoadResult(candidate.state)
        return SessionLoadResult("valid", normalize_session(candidate.snapshot))

    def save(self, session: SessionSnapshot) -> SessionSnapshot:
        """Normalize before writing and return the exact persisted contract."""
        normalized = normalize_session(session)
        self._store.save(normalized)
        return normalized


def normalize_session(session: SessionSnapshot) -> SessionSnapshot:
    """Bound paths, remove duplicates, and preserve a valid active-document identity."""
    if not isinstance(session, SessionSnapshot):
        return DEFAULT_SESSION
    if session.schema_version != CURRENT_SESSION_VERSION:
        return DEFAULT_SESSION

    workspace_root = _normalize_path(session.workspace_root)
    active_key = None
    if session.documents and 0 <= session.active_index < len(session.documents):
        active_key = path_key(session.documents[session.active_index].path)

    documents: list[SessionDocument] = []
    seen: set[str] = set()
    for candidate in session.documents:
        if len(documents) >= MAX_SESSION_DOCUMENTS:
            break
        if not isinstance(candidate, SessionDocument):
            continue
        path = _normalize_path(candidate.path)
        if path is None:
            continue
        key = path_key(path)
        if key in seen:
            continue
        seen.add(key)
        documents.append(
            SessionDocument(
                path=path,
                line=min(candidate.line, MAX_SESSION_CURSOR_LINE),
                column=min(candidate.column, MAX_SESSION_CURSOR_COLUMN),
            )
        )

    active_index = 0
    if active_key is not None:
        for index, document in enumerate(documents):
            if path_key(document.path) == active_key:
                active_index = index
                break
    return SessionSnapshot(
        schema_version=CURRENT_SESSION_VERSION,
        workspace_root=workspace_root,
        documents=tuple(documents),
        active_index=active_index,
    )


def _normalize_path(path: Path | None) -> Path | None:
    if path is None:
        return None
    if not isinstance(path, Path) or not path.is_absolute():
        return None
    try:
        resolved = path.expanduser().resolve()
    except (OSError, RuntimeError):
        return None
    if len(str(resolved)) > MAX_SESSION_PATH_CHARS:
        return None
    return resolved
