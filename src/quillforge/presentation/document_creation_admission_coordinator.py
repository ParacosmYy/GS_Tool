"""Qt-free admission and projection orchestration for new documents."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..application.documents import OpenedDocument
from ..domain.models import DocumentState
from .notification_contract import NotificationSink


@dataclass(frozen=True, slots=True)
class DocumentCreationAdmissionPorts:
    """Typed callbacks required to admit and project one new document."""

    is_busy: Callable[[], bool]
    startup_restore_inflight: Callable[[], bool]
    new_document: Callable[[], OpenedDocument]
    add_tab: Callable[[OpenedDocument], object]
    publish_opened: Callable[[DocumentState], None]
    notify: NotificationSink


class DocumentCreationAdmissionCoordinator:
    """Admit one new document while preserving tab and lifecycle ordering."""

    def __init__(self, ports: DocumentCreationAdmissionPorts) -> None:
        self._ports = ports

    def admit(self, *, allow_during_startup: bool = False) -> bool:
        """Create and project one new document when the shell permits it."""
        restoring = self._ports.startup_restore_inflight()
        if self._ports.is_busy() or (restoring and not allow_during_startup):
            if restoring and not allow_during_startup:
                self._ports.notify("Restoring the previous session...", level="warning")
            return False

        opened = self._ports.new_document()
        self._ports.add_tab(opened)
        self._ports.publish_opened(opened.state)
        self._ports.notify("New document", level="success")
        return True


__all__ = ["DocumentCreationAdmissionCoordinator", "DocumentCreationAdmissionPorts"]
