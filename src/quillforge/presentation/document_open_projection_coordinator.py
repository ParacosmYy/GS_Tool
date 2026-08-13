"""Qt-free projection orchestration for valid document-open results."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import TypeVar

from ..application.documents import OpenedDocument
from ..domain.models import DocumentState, SessionDocument

TabT = TypeVar("TabT")


@dataclass(frozen=True, slots=True)
class DocumentOpenProjectionPorts[TabT]:
    """Typed callbacks required by valid document-open projection policy."""

    find_existing: Callable[[Path | None], TabT | None]
    record_restored_tab: Callable[[TabT], None]
    show_duplicate_error: Callable[[], None]
    add_tab: Callable[[OpenedDocument], TabT]
    go_to_line: Callable[[TabT, int], None]
    set_cursor_position: Callable[[TabT, int, int], None]
    publish_opened: Callable[[DocumentState], None]
    notify_opened: Callable[[Path | None], None]
    continue_session_restore: Callable[[], None]


class DocumentOpenProjectionCoordinator[TabT]:
    """Project a valid open result without owning Qt or application services."""

    def __init__(
        self,
        ports: DocumentOpenProjectionPorts[TabT],
    ) -> None:
        self._ports = ports

    def project(
        self,
        result: OpenedDocument,
        line_number: int | None,
        session_restore: bool,
        session_document: SessionDocument | None,
    ) -> None:
        """Project one valid result while preserving open/restore ordering."""
        existing = self._ports.find_existing(result.state.path)
        if existing is not None:
            if session_restore:
                self._ports.record_restored_tab(existing)
                self._ports.continue_session_restore()
                return
            self._ports.show_duplicate_error()
            return

        tab = self._ports.add_tab(result)
        if line_number is not None:
            self._ports.go_to_line(tab, line_number)
        if session_restore and session_document is not None:
            self._ports.set_cursor_position(tab, session_document.line, session_document.column)
            self._ports.record_restored_tab(tab)
        self._ports.publish_opened(result.state)
        self._ports.notify_opened(result.state.path)
        if session_restore:
            self._ports.continue_session_restore()


__all__ = ["DocumentOpenProjectionCoordinator", "DocumentOpenProjectionPorts"]
