"""Qt-free projection of editor mutations into document-shell state."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class DocumentChangeProjectionPorts[EditorT, TabT]:
    """Application callbacks required to project one editor mutation."""

    find_tab: Callable[[EditorT], TabT | None]
    is_dirty: Callable[[TabT], bool]
    mark_dirty: Callable[[TabT, bool], None]
    increment_content_version: Callable[[TabT], None]
    invalidate_find_match: Callable[[], None]
    update_tab_title: Callable[[TabT], None]
    sync_status: Callable[[], None]
    request_session_save: Callable[[], None]


class DocumentChangeProjectionCoordinator[EditorT, TabT]:
    """Keep editor-change guards and shell projection order deterministic."""

    def __init__(self, ports: DocumentChangeProjectionPorts[EditorT, TabT]) -> None:
        self._ports = ports

    def project_modified(self, editor: EditorT, dirty: bool) -> None:
        """Project a dirty transition while preserving stale/unchanged guards."""
        self._ports.invalidate_find_match()
        tab = self._ports.find_tab(editor)
        if tab is None or self._ports.is_dirty(tab) == dirty:
            return
        self._ports.mark_dirty(tab, dirty)
        self._ports.update_tab_title(tab)
        self._ports.sync_status()
        self._ports.request_session_save()

    def project_content_changed(self, editor: EditorT) -> None:
        """Advance content identity before invalidating any Find match."""
        tab = self._ports.find_tab(editor)
        if tab is None:
            return
        self._ports.increment_content_version(tab)
        self._ports.invalidate_find_match()


__all__ = [
    "DocumentChangeProjectionCoordinator",
    "DocumentChangeProjectionPorts",
]
