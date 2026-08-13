"""Qt-free admission and dispatch orchestration for document opening."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from .document_open_coordinator import OpenDispatcher, OpenOperation
from .notification_contract import NotificationSink

OpenDocument = Callable[[Path], object]
OpenSubmission = Callable[[OpenOperation, int, int | None, OpenDispatcher], None]


@dataclass(frozen=True, slots=True)
class DocumentOpenAdmissionPorts:
    """Typed callbacks required to admit and dispatch one document-open request."""

    is_busy: Callable[[], bool]
    startup_restore_inflight: Callable[[], bool]
    begin_operation: Callable[[str], int]
    bind_session_restore: Callable[[int], None]
    open_document: OpenDocument
    submit_open: OpenSubmission
    dispatch: OpenDispatcher
    notify: NotificationSink


class DocumentOpenAdmissionCoordinator:
    """Admit one path while preserving the existing asynchronous open boundary."""

    def __init__(self, ports: DocumentOpenAdmissionPorts) -> None:
        self._ports = ports

    def admit(
        self,
        path: Path,
        *,
        line_number: int | None = None,
        session_restore: bool = False,
    ) -> bool:
        """Start one accepted open request and report whether it was admitted."""
        if self._ports.is_busy() or (
            self._ports.startup_restore_inflight() and not session_restore
        ):
            if self._ports.startup_restore_inflight() and not session_restore:
                self._ports.notify("Restoring the previous session...", level="warning")
            return False

        operation_id = self._ports.begin_operation(f"Opening {path.name}...")
        if session_restore:
            self._ports.bind_session_restore(operation_id)
        self._ports.submit_open(
            lambda: self._ports.open_document(path),
            operation_id,
            line_number,
            self._ports.dispatch,
        )
        return True


__all__ = ["DocumentOpenAdmissionCoordinator", "DocumentOpenAdmissionPorts"]
