"""Qt-free presentation orchestration for recovery inventory scan results."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..application.recovery import RecoveryCandidate
from ..domain.models import SessionSnapshot
from .notification_contract import NotificationSink
from .recovery_scan_tracker import RecoveryScanJob, RecoveryScanTracker

ScanOperation = Callable[[], object]
ScanSuccess = Callable[[object, int], None]
ScanFailure = Callable[[Exception, int], None]
ScanDispatcher = Callable[[ScanOperation, int, ScanSuccess, ScanFailure], None]


@dataclass(frozen=True, slots=True)
class RecoveryScanPorts:
    """Typed callbacks required to project one recovery inventory result."""

    get_session_snapshot: Callable[[], SessionSnapshot]
    prompt_recovery: Callable[[RecoveryCandidate], None]
    continue_session_restore: Callable[[SessionSnapshot], None]
    notify: NotificationSink


class RecoveryScanCoordinator:
    """Classify one recovery inventory without owning recovery decisions."""

    def __init__(
        self,
        tracker: RecoveryScanTracker,
        ports: RecoveryScanPorts,
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def submit(
        self,
        *,
        operation: ScanOperation,
        operation_id: int,
        job: RecoveryScanJob,
        dispatch: ScanDispatcher,
    ) -> None:
        """Bind one scan job before invoking a generic worker dispatcher."""

        def on_scanned(result: object, _operation_id: int) -> None:
            self.complete(result, job)

        def on_failed(error: Exception, _operation_id: int) -> None:
            self.fail(error, job)

        dispatch(operation, operation_id, on_scanned, on_failed)

    def complete(self, result: object, job: RecoveryScanJob) -> None:
        """Validate one inventory, project candidates, and continue startup."""
        if not self._tracker.finish(job):
            return
        if not isinstance(result, tuple) or not all(
            isinstance(candidate, RecoveryCandidate) for candidate in result
        ):
            self._project_failure(
                RuntimeError("The recovery service returned an invalid inventory"),
                job.startup,
            )
            return
        if not result:
            if not job.startup:
                self._ports.notify("No recovery snapshots found", level="info")
        else:
            for candidate in result:
                self._ports.prompt_recovery(candidate)
        if job.startup:
            self._ports.continue_session_restore(self._ports.get_session_snapshot())

    def fail(self, error: Exception, job: RecoveryScanJob) -> None:
        """Project a failed inventory and continue startup when required."""
        if not self._tracker.finish(job):
            return
        self._project_failure(error, job.startup)

    def _project_failure(self, error: Exception, startup: bool) -> None:
        self._ports.notify(f"Recovery scan failed: {error}", level="error")
        if startup:
            self._ports.continue_session_restore(self._ports.get_session_snapshot())


__all__ = ["RecoveryScanCoordinator", "RecoveryScanPorts"]
