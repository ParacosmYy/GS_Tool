"""Qt-free orchestration for finalizing a document-tab removal."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class DocumentTabRemovalPorts[TabT, CaptureT]:
    """Typed callbacks required by document-tab removal finalization."""

    contains: Callable[[TabT], bool]
    document_id: Callable[[TabT], str]
    capture_for_document: Callable[[str], CaptureT | None]
    cancel_capture: Callable[[CaptureT], None]
    clear_recovery_snapshot: Callable[[TabT], None]
    remove_tab: Callable[[TabT], bool]
    delete_editor: Callable[[TabT], None]
    publish_closed: Callable[[TabT], None]
    request_session_save: Callable[[], None]
    tab_count: Callable[[], int]
    ensure_initial_document: Callable[[], None]


class DocumentTabRemovalCoordinator[TabT, CaptureT]:
    """Finalize an already-approved tab removal without owning close policy."""

    def __init__(
        self,
        ports: DocumentTabRemovalPorts[TabT, CaptureT],
    ) -> None:
        self._ports = ports

    def remove(self, tab: TabT) -> bool:
        """Finalize one live tab and return whether it was eligible for removal."""
        if not self._ports.contains(tab):
            return False
        capture = self._ports.capture_for_document(self._ports.document_id(tab))
        if capture is not None:
            self._ports.cancel_capture(capture)
        self._ports.clear_recovery_snapshot(tab)
        self._ports.remove_tab(tab)
        self._ports.delete_editor(tab)
        self._ports.publish_closed(tab)
        self._ports.request_session_save()
        if self._ports.tab_count() == 0:
            self._ports.ensure_initial_document()
        return True


__all__ = ["DocumentTabRemovalCoordinator", "DocumentTabRemovalPorts"]
