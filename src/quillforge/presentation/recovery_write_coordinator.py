"""Qt-free presentation orchestration for recovery snapshot writes."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from .recovery_capture_tracker import RecoveryCaptureTracker

WriteOperation = Callable[[], object]
WriteSuccess = Callable[[object, int], None]
WriteFailure = Callable[[Exception, int], None]
WriteDispatcher = Callable[[WriteOperation, int, WriteSuccess, WriteFailure], None]


@dataclass(frozen=True, slots=True)
class RecoveryWritePorts[JobT, OwnerT]:
    """Typed callbacks required to project one recovery-write result."""

    document_id: Callable[[OwnerT], str]
    abort_capture: Callable[[JobT, Exception], None]
    schedule_pending_delete: Callable[[str, OwnerT | None, str | None], None]
    project_saved: Callable[[OwnerT, int, str, bool], None]
    project_failed: Callable[[OwnerT, str, Exception, bool], None]


class RecoveryWriteCoordinator[JobT, OwnerT]:
    """Classify recovery-write callbacks without owning tab policy."""

    def __init__(
        self,
        tracker: RecoveryCaptureTracker[JobT, OwnerT],
        ports: RecoveryWritePorts[JobT, OwnerT],
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def submit(
        self,
        *,
        operation: WriteOperation,
        operation_id: int,
        owner: OwnerT,
        content_version: int,
        snapshot_id: str,
        dispatch: WriteDispatcher,
    ) -> None:
        """Bind one write identity before invoking a generic worker dispatcher."""

        def on_saved(_result: object, _operation_id: int) -> None:
            self.complete(owner, content_version, snapshot_id)

        def on_failed(error: Exception, _operation_id: int) -> None:
            self.fail(owner, snapshot_id, error)

        dispatch(operation, operation_id, on_saved, on_failed)

    def complete(self, owner: OwnerT, content_version: int, snapshot_id: str) -> None:
        """Release write lifecycle and delegate saved-result policy."""
        discarded = self._tracker.consume_discarded(snapshot_id)
        self._tracker.complete_document(self._ports.document_id(owner))
        self._finish_write(snapshot_id)
        self._ports.project_saved(owner, content_version, snapshot_id, discarded)

    def fail(self, owner: OwnerT, snapshot_id: str, error: Exception) -> None:
        """Abort a bound capture, release write state, and delegate failure policy."""
        discarded = self._tracker.consume_discarded(snapshot_id)
        capture_job = self._tracker.capture_for_snapshot(snapshot_id)
        if capture_job is not None:
            self._ports.abort_capture(capture_job, error)
        self._tracker.complete_document(self._ports.document_id(owner))
        self._finish_write(snapshot_id)
        self._ports.project_failed(owner, snapshot_id, error, discarded)

    def _finish_write(self, snapshot_id: str) -> None:
        """Release the write and forward any deferred delete request."""
        pending = self._tracker.finish_write(snapshot_id)
        if pending is not None:
            self._ports.schedule_pending_delete(
                snapshot_id,
                pending.tab,
                pending.success_message,
            )


__all__ = ["RecoveryWriteCoordinator", "RecoveryWritePorts"]
