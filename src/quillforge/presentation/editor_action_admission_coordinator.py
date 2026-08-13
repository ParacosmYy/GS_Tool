"""Qt-free admission of one editor-local action against the active tab."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class EditorActionAdmissionPorts[EditorT, TabT]:
    """Application callbacks required to admit one editor-local action."""

    active_tab: Callable[[], TabT | None]
    is_busy: Callable[[], bool]
    editor_of: Callable[[TabT], EditorT]
    focus_editor: Callable[[TabT], None]


class EditorActionAdmissionCoordinator[EditorT, TabT]:
    """Preserve the active-tab guard and focus handoff without Qt types."""

    def __init__(self, ports: EditorActionAdmissionPorts[EditorT, TabT]) -> None:
        self._ports = ports

    def admit(self, action: Callable[[EditorT], None]) -> None:
        """Run an admitted editor action once, then restore editor focus."""
        tab = self._ports.active_tab()
        if tab is None or self._ports.is_busy():
            return
        action(self._ports.editor_of(tab))
        self._ports.focus_editor(tab)


__all__ = [
    "EditorActionAdmissionCoordinator",
    "EditorActionAdmissionPorts",
]
