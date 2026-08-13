"""Qt-free close-readiness classification for the desktop shell."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal

CloseBlockReason = Literal[
    "operation",
    "workspace-search",
    "dirty",
    "background",
    "pending",
]


@dataclass(frozen=True, slots=True)
class CloseGuardDecision:
    """The result of one ordered close-readiness evaluation."""

    block_reason: CloseBlockReason | None = None

    @property
    def allowed(self) -> bool:
        """Return whether the shell may accept its close event."""
        return self.block_reason is None


@dataclass(frozen=True, slots=True)
class CloseGuardPorts:
    """Typed callbacks required by close-readiness classification."""

    is_busy: Callable[[], bool]
    workspace_search_in_flight: Callable[[], bool]
    cancel_workspace_search: Callable[[], None]
    has_dirty_tabs: Callable[[], bool]
    has_background_operations: Callable[[], bool]
    request_immediate_session_save: Callable[[], None]
    has_pending_work: Callable[[], bool]
    stop_timers: Callable[[], None]


class CloseGuardCoordinator:
    """Preserve close precedence without owning Qt events or UI projection."""

    def __init__(
        self,
        ports: CloseGuardPorts,
    ) -> None:
        self._ports = ports

    def evaluate(self) -> CloseGuardDecision:
        """Evaluate close gates in the established order and preserve side effects."""
        if self._ports.is_busy():
            return CloseGuardDecision("operation")
        if self._ports.workspace_search_in_flight():
            self._ports.cancel_workspace_search()
            return CloseGuardDecision("workspace-search")
        if self._ports.has_dirty_tabs():
            return CloseGuardDecision("dirty")
        if self._ports.has_background_operations():
            return CloseGuardDecision("background")
        self._ports.request_immediate_session_save()
        if self._ports.has_pending_work():
            return CloseGuardDecision("pending")
        self._ports.stop_timers()
        return CloseGuardDecision()


__all__ = [
    "CloseBlockReason",
    "CloseGuardCoordinator",
    "CloseGuardDecision",
    "CloseGuardPorts",
]
