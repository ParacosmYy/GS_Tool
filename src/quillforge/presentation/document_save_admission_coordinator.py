"""Qt-free admission and dispatch orchestration for document saves."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import TypeVar

from ..domain.models import DocumentState
from .document_save_coordinator import (
    SaveContinuation,
    SaveDispatcher,
    SaveOperation,
)
from .notification_contract import NotificationSink

TabT = TypeVar("TabT")
SaveDocument = Callable[[DocumentState, str, Path], object]
SaveSubmission = Callable[
    [SaveOperation, int, TabT, SaveContinuation | None, SaveDispatcher],
    None,
]


@dataclass(frozen=True, slots=True)
class DocumentSaveAdmissionPorts[TabT]:
    """Typed callbacks required to admit and dispatch one document save."""

    is_busy: Callable[[], bool]
    startup_restore_inflight: Callable[[], bool]
    find_existing: Callable[[Path, TabT], TabT | None]
    show_duplicate_error: Callable[[], None]
    state_snapshot: Callable[[TabT], DocumentState]
    text_snapshot: Callable[[TabT], str]
    set_read_only: Callable[[TabT, bool], None]
    begin_operation: Callable[[str], int]
    save_document: SaveDocument
    submit_save: SaveSubmission[TabT]
    dispatch: SaveDispatcher
    notify: NotificationSink


class DocumentSaveAdmissionCoordinator[TabT]:
    """Admit one save while preserving the existing asynchronous save boundary."""

    def __init__(self, ports: DocumentSaveAdmissionPorts[TabT]) -> None:
        self._ports = ports

    def admit(
        self,
        tab: TabT,
        target: Path,
        *,
        after: SaveContinuation | None = None,
    ) -> bool:
        """Start one accepted save request and report whether it was admitted."""
        if self._ports.is_busy() or self._ports.startup_restore_inflight():
            if self._ports.startup_restore_inflight():
                self._ports.notify("Restoring the previous session...", level="warning")
            return False
        if self._ports.find_existing(target, tab) is not None:
            self._ports.show_duplicate_error()
            return False

        state_snapshot = self._ports.state_snapshot(tab)
        text_snapshot = self._ports.text_snapshot(tab)
        self._ports.set_read_only(tab, True)
        operation_id = self._ports.begin_operation(f"Saving {target.name}...")
        self._ports.submit_save(
            lambda: self._ports.save_document(state_snapshot, text_snapshot, target),
            operation_id,
            tab,
            after,
            self._ports.dispatch,
        )
        return True


__all__ = ["DocumentSaveAdmissionCoordinator", "DocumentSaveAdmissionPorts"]
