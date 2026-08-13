"""Qt-free presentation orchestration for asynchronous document saves."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..domain.models import DocumentState

SaveOperation = Callable[[], object]
SaveContinuation = Callable[[], None]
SaveSuccess = Callable[[object, int], None]
SaveFailure = Callable[[Exception, int], None]
SaveDispatcher = Callable[[SaveOperation, int, SaveSuccess, SaveFailure], None]


@dataclass(frozen=True, slots=True)
class DocumentSavePorts[TabT]:
    """Typed callbacks required by document-save completion policy."""

    complete_operation: Callable[[int], bool]
    contains_tab: Callable[[TabT], bool]
    set_read_only: Callable[[TabT, bool], None]
    apply_saved: Callable[[TabT, DocumentState, Callable[[], None] | None], None]
    show_error: Callable[[str, str | Exception], None]


class DocumentSaveCoordinator[TabT]:
    """Classify document-save callbacks without owning document policy."""

    def __init__(
        self,
        ports: DocumentSavePorts[TabT],
    ) -> None:
        self._ports = ports

    def submit(
        self,
        *,
        operation: SaveOperation,
        operation_id: int,
        tab: TabT,
        after: SaveContinuation | None,
        dispatch: SaveDispatcher,
    ) -> None:
        """Bind one save tab/continuation before invoking a worker dispatcher."""

        def on_saved(result: object, current_operation_id: int) -> None:
            self.complete(tab, result, current_operation_id, after)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(tab, error, current_operation_id)

        dispatch(operation, operation_id, on_saved, on_failed)

    def complete(
        self,
        tab: TabT,
        result: object,
        operation_id: int,
        after: Callable[[], None] | None,
    ) -> None:
        """Classify one save result and delegate only valid live-tab policy."""
        if not self._ports.complete_operation(operation_id):
            return
        if not self._ports.contains_tab(tab):
            return
        self._ports.set_read_only(tab, False)
        if not isinstance(result, DocumentState):
            self._ports.show_error(
                "Save failed",
                "The document service returned an invalid result.",
            )
            return
        self._ports.apply_saved(tab, result, after)

    def fail(self, tab: TabT, error: Exception, operation_id: int) -> None:
        """Classify one save failure and restore a live tab's editability."""
        if not self._ports.complete_operation(operation_id):
            return
        if not self._ports.contains_tab(tab):
            return
        self._ports.set_read_only(tab, False)
        self._ports.show_error("Save failed", error)


__all__ = ["DocumentSaveCoordinator", "DocumentSavePorts"]
