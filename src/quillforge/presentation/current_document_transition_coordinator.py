"""Qt-free projection of current-document transition side effects."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class CurrentDocumentTransitionPorts[TabT]:
    """Application callbacks required for one active-tab transition."""

    invalidate_find_match: Callable[[], None]
    reset_find_session: Callable[[], None]
    active_tab: Callable[[], TabT | None]
    title_of: Callable[[TabT], str]
    notify_info: Callable[[str], None]
    sync_status: Callable[[], None]
    request_session_save: Callable[[], None]


class CurrentDocumentTransitionCoordinator[TabT]:
    """Preserve current-tab transition ordering without Qt dependencies."""

    def __init__(self, ports: CurrentDocumentTransitionPorts[TabT]) -> None:
        self._ports = ports

    def project(self) -> None:
        """Project the active-document change in the established order."""
        self._ports.invalidate_find_match()
        self._ports.reset_find_session()
        tab = self._ports.active_tab()
        if tab is not None:
            self._ports.notify_info(self._ports.title_of(tab).lstrip("*"))
        self._ports.sync_status()
        self._ports.request_session_save()


__all__ = [
    "CurrentDocumentTransitionCoordinator",
    "CurrentDocumentTransitionPorts",
]
