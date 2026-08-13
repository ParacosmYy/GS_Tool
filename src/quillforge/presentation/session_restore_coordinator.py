"""Qt-free orchestration for ordered session restoration."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from .session_restore_tracker import SessionRestoreTracker


@dataclass(frozen=True, slots=True)
class SessionRestorePorts[TabT]:
    """Typed presentation callbacks required by ordered session restoration."""

    is_active: Callable[[], bool]
    set_current: Callable[[TabT], None]
    tab_count: Callable[[], int]
    ensure_initial_document: Callable[[], None]
    finish_restore: Callable[[], None]
    request_session_save: Callable[[], None]
    path_of: Callable[[TabT], Path | None]
    find_tab: Callable[[Path], TabT | None]
    start_open: Callable[[Path], None]
    notify_deferred: Callable[[Path], None]


class SessionRestoreCoordinator[TabT]:
    """Advance restore state without owning tabs, services, or startup policy."""

    def __init__(
        self,
        tracker: SessionRestoreTracker[TabT],
        ports: SessionRestorePorts[TabT],
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def continue_restore(self) -> None:
        """Advance synchronously until an async document open is required."""
        if not self._ports.is_active() or self._tracker.workspace_pending:
            return
        while self._tracker.has_remaining_documents:
            session_document = self._tracker.next_document()
            if session_document is None:
                return
            path = session_document.path
            if self._tracker.is_deferred(path):
                self._ports.notify_deferred(path)
                continue
            existing = self._ports.find_tab(path)
            if existing is not None:
                self._tracker.record_restored_tab(existing)
                continue
            self._tracker.set_pending_document(session_document)
            self._ports.start_open(path)
            return

        target = self._tracker.select_restored_tab(self._ports.path_of)
        if target is not None:
            self._ports.set_current(target)
        elif self._ports.tab_count() == 0:
            self._ports.ensure_initial_document()
        self._ports.finish_restore()
        self._ports.request_session_save()


__all__ = ["SessionRestoreCoordinator", "SessionRestorePorts"]
