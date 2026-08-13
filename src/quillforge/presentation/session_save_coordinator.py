"""Qt-free presentation orchestration for latest-wins session persistence."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..domain.models import SessionSnapshot
from .notification_contract import NotificationSink
from .session_save_tracker import SessionSaveTracker

SessionSaveOperation = Callable[[], object]
SessionSaveSuccess = Callable[[object, int], None]
SessionSaveFailure = Callable[[Exception, int], None]
SessionSaveDispatcher = Callable[
    [SessionSaveOperation, int, SessionSaveSuccess, SessionSaveFailure],
    None,
]


@dataclass(frozen=True, slots=True)
class SessionSavePorts:
    """Typed callbacks required by latest-wins session persistence."""

    can_save: Callable[[], bool]
    capture_snapshot: Callable[[], SessionSnapshot]
    next_operation_id: Callable[[], int]
    save_snapshot: Callable[[SessionSnapshot], object]
    dispatch: SessionSaveDispatcher
    notify: NotificationSink


class SessionSaveCoordinator:
    """Own latest-wins session-save admission and callback sequencing."""

    def __init__(
        self,
        tracker: SessionSaveTracker,
        ports: SessionSavePorts,
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def request_latest(self) -> None:
        """Capture and queue the newest UI snapshot, then start it if idle."""
        if not self._ports.can_save():
            return
        snapshot = self._ports.capture_snapshot()
        if self._tracker.request(snapshot):
            self.drain()

    def drain(self) -> None:
        """Submit the queued snapshot when no save operation is in flight."""
        if not self._ports.can_save():
            return
        if not self._tracker.has_request or self._tracker.inflight:
            return
        operation_id = self._ports.next_operation_id()
        snapshot = self._tracker.begin(operation_id)
        if snapshot is not None:
            self.submit(
                operation=lambda snapshot=snapshot: self._ports.save_snapshot(snapshot),
                operation_id=operation_id,
            )

    def submit(self, *, operation: SessionSaveOperation, operation_id: int) -> None:
        """Bind one admitted save operation to the coordinator callbacks."""
        self._ports.dispatch(operation, operation_id, self.complete, self.fail)

    def complete(self, result: object, operation_id: int) -> None:
        """Classify a save callback and drain the latest queued request."""
        completion = self._tracker.complete(operation_id, result)
        if completion == "stale":
            return
        if completion == "invalid":
            self._ports.notify("Session state returned an invalid result", level="error")
        self.drain()

    def fail(self, _error: Exception, operation_id: int) -> None:
        """Consume a matching failed save callback and preserve queued state."""
        if not self._tracker.fail(operation_id):
            return
        self._ports.notify(
            "Session state could not be saved; the previous manifest was kept",
            level="error",
        )
        self.drain()


__all__ = ["SessionSaveCoordinator", "SessionSavePorts"]
