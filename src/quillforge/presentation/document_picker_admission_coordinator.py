"""Qt-free admission for choosing a document before asynchronous opening."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from .notification_contract import NotificationSink


@dataclass(frozen=True, slots=True)
class DocumentPickerAdmissionPorts:
    """Typed shell callbacks required to admit one native file picker flow."""

    is_busy: Callable[[], bool]
    startup_restore_inflight: Callable[[], bool]
    choose_document: Callable[[], Path | None]
    start_open: Callable[[Path], None]
    notify: NotificationSink


class DocumentPickerAdmissionCoordinator:
    """Gate one user file selection before the existing open boundary."""

    def __init__(self, ports: DocumentPickerAdmissionPorts) -> None:
        self._ports = ports

    def admit(self) -> bool:
        """Choose and submit one document when the shell permits it."""
        if self._ports.is_busy():
            return False
        if self._ports.startup_restore_inflight():
            self._ports.notify("Restoring the previous session...", level="warning")
            return False
        selected = self._ports.choose_document()
        if selected is None:
            return False
        self._ports.start_open(selected)
        return True


__all__ = ["DocumentPickerAdmissionCoordinator", "DocumentPickerAdmissionPorts"]
