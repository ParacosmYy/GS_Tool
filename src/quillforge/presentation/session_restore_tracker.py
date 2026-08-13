"""Framework-neutral state boundary for ordered session restoration."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field
from pathlib import Path

from ..domain.models import SessionDocument, SessionSnapshot
from ..domain.path_identity import path_key


@dataclass(slots=True)
class SessionRestoreTracker[TabT]:
    """Own restore values without owning Qt widgets or application policy.

    MainWindow remains responsible for startup barriers, service calls,
    TaskRunner dispatch, tab projection, notifications, and close behavior.
    This tracker only makes the ordered restore state and one pending document
    callback binding and restored-tab projection explicit.
    """

    _snapshot: SessionSnapshot | None = None
    _workspace_pending: bool = False
    _next_index: int = 0
    _active_path: Path | None = None
    _deferred_paths: set[str] = field(default_factory=set)
    _pending_document: SessionDocument | None = None
    _operation_id: int | None = None
    _restored_tabs: list[TabT] = field(default_factory=list)

    @property
    def snapshot(self) -> SessionSnapshot | None:
        """Return the loaded or currently restoring immutable session."""
        return self._snapshot

    @property
    def workspace_pending(self) -> bool:
        """Return whether workspace restoration still gates document restore."""
        return self._workspace_pending

    @property
    def active_path(self) -> Path | None:
        """Return the path that should become the restored active tab."""
        return self._active_path

    @property
    def operation_id(self) -> int | None:
        """Return the document-open operation bound to session restoration."""
        return self._operation_id

    @property
    def has_remaining_documents(self) -> bool:
        """Return whether ordered restoration still has an unconsumed path."""
        snapshot = self._snapshot
        return snapshot is not None and self._next_index < len(snapshot.documents)

    def set_snapshot(self, snapshot: SessionSnapshot) -> None:
        """Store a loaded snapshot before recovery decisions finish."""
        _require_snapshot(snapshot)
        self._snapshot = snapshot

    def begin(self, snapshot: SessionSnapshot) -> None:
        """Start ordered restoration from one immutable snapshot."""
        _require_snapshot(snapshot)
        if self._operation_id is not None or self._pending_document is not None:
            raise RuntimeError("Cannot begin session restore during a document open")
        self._snapshot = snapshot
        self._workspace_pending = False
        self._next_index = 0
        self._active_path = None
        self._restored_tabs.clear()
        if snapshot.documents and 0 <= snapshot.active_index < len(snapshot.documents):
            self._active_path = snapshot.documents[snapshot.active_index].path

    def set_workspace_pending(self, pending: bool) -> None:
        """Set the workspace barrier without invoking a workspace service."""
        if type(pending) is not bool:
            raise TypeError("workspace pending must be a bool")
        self._workspace_pending = pending

    def next_document(self) -> SessionDocument | None:
        """Consume the next path in order, or return ``None`` at completion."""
        if self._operation_id is not None or self._pending_document is not None:
            raise RuntimeError("Cannot advance session restore during a document open")
        snapshot = self._snapshot
        if snapshot is None or self._next_index >= len(snapshot.documents):
            return None
        document = snapshot.documents[self._next_index]
        self._next_index += 1
        return document

    def record_restored_tab(self, tab: TabT) -> None:
        """Append one tab in the order in which session restoration handled it."""
        self._restored_tabs.append(tab)

    def select_restored_tab(
        self,
        path_of: Callable[[TabT], Path | None],
    ) -> TabT | None:
        """Return the active-path tab, or the first restored tab as a fallback."""
        if self._active_path is not None:
            active_identity = path_key(self._active_path)
            for tab in self._restored_tabs:
                path = path_of(tab)
                if path is None:
                    continue
                _require_path(path)
                if path_key(path) == active_identity:
                    return tab
        return self._restored_tabs[0] if self._restored_tabs else None

    def is_deferred(self, path: Path) -> bool:
        """Return whether recovery decisions deferred this path."""
        _require_path(path)
        return path_key(path) in self._deferred_paths

    def defer(self, path: Path) -> None:
        """Remember one path that recovery must handle before session opening."""
        _require_path(path)
        self._deferred_paths.add(path_key(path))

    def clear_deferred(self) -> None:
        """Clear recovery-deferred paths at the start of a new startup restore."""
        self._deferred_paths.clear()

    def set_pending_document(self, document: SessionDocument) -> None:
        """Bind the document that the next session restore open will request."""
        if not isinstance(document, SessionDocument):
            raise TypeError("pending session document must be a SessionDocument")
        if self._operation_id is not None or self._pending_document is not None:
            raise RuntimeError("A session document open is already pending")
        self._pending_document = document

    def bind_open_operation(self, operation_id: int) -> None:
        """Bind a TaskRunner operation to the pending restore document."""
        _require_operation_id(operation_id)
        if self._pending_document is None:
            raise RuntimeError("A pending session document is required")
        if self._operation_id is not None:
            raise RuntimeError("A session document operation is already bound")
        self._operation_id = operation_id

    def take_open_document(self, operation_id: int) -> SessionDocument | None:
        """Consume the matching restore callback binding, rejecting stale IDs."""
        _require_operation_id(operation_id)
        if self._operation_id != operation_id:
            return None
        document = self._pending_document
        self._operation_id = None
        self._pending_document = None
        return document

    def finish(self) -> None:
        """Clear the active restore while retaining deferred-path history."""
        self._snapshot = None
        self._workspace_pending = False
        self._next_index = 0
        self._active_path = None
        self._pending_document = None
        self._operation_id = None
        self._restored_tabs.clear()


def _require_snapshot(snapshot: SessionSnapshot) -> None:
    if not isinstance(snapshot, SessionSnapshot):
        raise TypeError("session snapshot must be a SessionSnapshot")


def _require_path(path: Path) -> None:
    if not isinstance(path, Path):
        raise TypeError("session restore path must be a Path")


def _require_operation_id(operation_id: int) -> None:
    if type(operation_id) is not int or operation_id < 1:
        raise ValueError("session restore operation ID must be a positive integer")
