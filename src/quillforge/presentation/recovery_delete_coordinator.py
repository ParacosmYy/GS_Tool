"""Qt-free presentation orchestration for recovery snapshot deletion."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from .notification_contract import NotificationSink
from .recovery_capture_tracker import RecoveryCaptureTracker

DeleteOperation = Callable[[], object]
DeleteSuccess = Callable[[object, int], None]
DeleteFailure = Callable[[Exception, int], None]
DeleteDispatcher = Callable[[DeleteOperation, int, DeleteSuccess, DeleteFailure], None]


@dataclass(frozen=True, slots=True)
class RecoveryDeletePorts[OwnerT]:
    """Typed callbacks required to project one recovery-delete result."""

    clear_owner_snapshot: Callable[[OwnerT, str], None]
    schedule_pending_delete: Callable[[str, OwnerT | None, str | None], None]
    notify: NotificationSink


class RecoveryDeleteCoordinator[JobT, OwnerT]:
    """Classify recovery-delete callbacks without owning recovery policy."""

    def __init__(
        self,
        tracker: RecoveryCaptureTracker[JobT, OwnerT],
        ports: RecoveryDeletePorts[OwnerT],
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def submit(
        self,
        *,
        operation: DeleteOperation,
        operation_id: int,
        snapshot_id: str,
        owner: OwnerT | None,
        success_message: str | None,
        dispatch: DeleteDispatcher,
    ) -> None:
        """Bind one delete identity before invoking a generic worker dispatcher."""

        def on_deleted(_result: object, _operation_id: int) -> None:
            self.complete(snapshot_id, owner, success_message)

        def on_failed(error: Exception, _operation_id: int) -> None:
            self.fail(snapshot_id, error)

        dispatch(operation, operation_id, on_deleted, on_failed)

    def complete(
        self,
        snapshot_id: str,
        owner: OwnerT | None,
        success_message: str | None,
    ) -> None:
        """Release one delete and drain any request queued behind its write."""
        pending = self._tracker.complete_delete(snapshot_id)
        if owner is not None:
            self._ports.clear_owner_snapshot(owner, snapshot_id)
        if success_message is not None:
            self._ports.notify(success_message, level="success")
        if pending is not None:
            self._ports.schedule_pending_delete(
                snapshot_id,
                pending.tab,
                pending.success_message,
            )

    def fail(self, snapshot_id: str, error: Exception) -> None:
        """Release one failed delete and preserve the existing error notice."""
        self._tracker.fail_delete(snapshot_id)
        self._ports.notify(f"Recovery cleanup failed: {error}", level="error")


__all__ = ["RecoveryDeleteCoordinator", "RecoveryDeletePorts"]
