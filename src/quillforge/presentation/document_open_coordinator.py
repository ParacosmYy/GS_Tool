"""Qt-free presentation orchestration for asynchronous document opening."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..application.documents import OpenedDocument
from ..domain.models import SessionDocument
from .notification_contract import NotificationSink

OpenOperation = Callable[[], object]
OpenSuccess = Callable[[object, int], None]
OpenFailure = Callable[[Exception, int], None]
OpenDispatcher = Callable[[OpenOperation, int, OpenSuccess, OpenFailure], None]


@dataclass(frozen=True, slots=True)
class DocumentOpenPorts:
    """Typed callbacks required by ordinary and restored document opening."""

    complete_operation: Callable[[int], bool]
    is_session_restore: Callable[[int], bool]
    take_session_document: Callable[[int], SessionDocument | None]
    apply_opened: Callable[[OpenedDocument, int | None, bool, SessionDocument | None], None]
    continue_session_restore: Callable[[], None]
    show_error: Callable[[str, str | Exception], None]
    notify: NotificationSink


class DocumentOpenCoordinator:
    """Classify document-open callbacks without owning tab or editor policy."""

    def __init__(
        self,
        ports: DocumentOpenPorts,
    ) -> None:
        self._ports = ports

    def submit(
        self,
        *,
        operation: OpenOperation,
        operation_id: int,
        line_number: int | None,
        dispatch: OpenDispatcher,
    ) -> None:
        """Bind open navigation before invoking a generic worker dispatcher."""

        def on_opened(result: object, current_operation_id: int) -> None:
            self.complete(result, current_operation_id, line_number=line_number)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(error, current_operation_id)

        dispatch(operation, operation_id, on_opened, on_failed)

    def complete(
        self,
        result: object,
        operation_id: int,
        *,
        line_number: int | None = None,
    ) -> None:
        """Classify one document-open result and delegate valid policy."""
        if not self._ports.complete_operation(operation_id):
            return
        session_restore = self._ports.is_session_restore(operation_id)
        session_document = (
            self._ports.take_session_document(operation_id) if session_restore else None
        )
        if not isinstance(result, OpenedDocument):
            if session_restore:
                self._ports.notify(
                    "Session document returned an invalid result; continuing",
                    level="warning",
                )
                self._ports.continue_session_restore()
                return
            self._ports.show_error(
                "Open failed",
                "The document service returned an invalid result.",
            )
            return
        self._ports.apply_opened(result, line_number, session_restore, session_document)

    def fail(self, error: Exception, operation_id: int) -> None:
        """Classify one document-open worker failure."""
        if not self._ports.complete_operation(operation_id):
            return
        session_restore = self._ports.is_session_restore(operation_id)
        session_document = (
            self._ports.take_session_document(operation_id) if session_restore else None
        )
        if session_restore:
            if session_document is not None:
                self._ports.notify(
                    f"Session document skipped: {session_document.path.name}",
                    level="warning",
                )
            self._ports.continue_session_restore()
            return
        self._ports.show_error("Operation failed", error)


__all__ = ["DocumentOpenCoordinator", "DocumentOpenPorts"]
