"""Qt-free projection of close-guard blocking feedback."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from .close_guard_coordinator import CloseBlockReason, CloseGuardDecision


@dataclass(frozen=True, slots=True)
class CloseGuardFeedbackPorts:
    """Application callbacks required to project a close-block message."""

    pending_count: Callable[[], int]
    show_error: Callable[[str, str], None]


class CloseGuardFeedbackCoordinator:
    """Map close-readiness reasons to the existing recoverable error copy."""

    def __init__(self, ports: CloseGuardFeedbackPorts) -> None:
        self._ports = ports

    def project(self, decision: CloseGuardDecision) -> None:
        """Project one blocked decision and ignore an allowed close decision."""
        messages: dict[CloseBlockReason, tuple[str, str]] = {
            "operation": (
                "Operation in progress",
                "Wait for the current document operation to finish.",
            ),
            "workspace-search": (
                "Background operation in progress",
                "Wait for the workspace search to cancel before quitting.",
            ),
            "dirty": (
                "Unsaved changes",
                "Save or close modified tabs before quitting.",
            ),
            "background": (
                "Background operation in progress",
                "Wait for the current background operation to finish before quitting.",
            ),
            "pending": (
                "Background operation in progress",
                (
                    f"Wait for {self._ports.pending_count()} background operation or "
                    "completion callback(s) to finish before quitting."
                ),
            ),
        }
        reason = decision.block_reason
        if reason is not None:
            self._ports.show_error(*messages[reason])


__all__ = ["CloseGuardFeedbackCoordinator", "CloseGuardFeedbackPorts"]
