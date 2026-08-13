"""Qt-free presentation orchestration for session-load result projection."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..application.ports import SessionLoadResult
from ..application.session import DEFAULT_SESSION
from ..domain.models import SessionSnapshot
from .notification_contract import NotificationSink

SessionLoadOperation = Callable[[], object]
SessionLoadSuccess = Callable[[object, int], None]
SessionLoadFailure = Callable[[Exception, int], None]
SessionLoadDispatcher = Callable[
    [SessionLoadOperation, int, SessionLoadSuccess, SessionLoadFailure], None
]


@dataclass(frozen=True, slots=True)
class SessionLoadPorts:
    """Typed callbacks required to project one session-load result."""

    set_last_saved: Callable[[SessionSnapshot], None]
    set_snapshot: Callable[[SessionSnapshot], None]
    schedule_recovery_scan: Callable[[], None]
    notify: NotificationSink


class SessionLoadCoordinator:
    """Classify one load result without owning persistence or restore policy."""

    def __init__(
        self,
        ports: SessionLoadPorts,
    ) -> None:
        self._ports = ports

    def submit(
        self,
        *,
        operation: SessionLoadOperation,
        operation_id: int,
        dispatch: SessionLoadDispatcher,
    ) -> None:
        """Bind session-load callbacks before invoking a worker dispatcher."""

        def on_loaded(result: object, current_operation_id: int) -> None:
            self.complete(result, current_operation_id)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(error, current_operation_id)

        dispatch(operation, operation_id, on_loaded, on_failed)

    def complete(self, result: object, _operation_id: int) -> None:
        """Project a typed load result and continue recovery-first startup."""
        if not isinstance(result, SessionLoadResult):
            self._project_default_failure()
            return
        snapshot = result.snapshot or DEFAULT_SESSION
        self._set_baseline(snapshot)
        if result.state == "invalid":
            self._ports.notify(
                "Previous session could not be read; original manifest was kept",
                level="error",
            )
        self._ports.schedule_recovery_scan()

    def fail(self, _error: Exception, _operation_id: int) -> None:
        """Project a failed load while preserving the original manifest."""
        self._project_default_failure()

    def _project_default_failure(self) -> None:
        self._set_baseline(DEFAULT_SESSION)
        self._ports.notify(
            "Previous session could not be read; original manifest was kept",
            level="error",
        )
        self._ports.schedule_recovery_scan()

    def _set_baseline(self, snapshot: SessionSnapshot) -> None:
        self._ports.set_last_saved(snapshot)
        self._ports.set_snapshot(snapshot)


__all__ = ["SessionLoadCoordinator", "SessionLoadPorts"]
