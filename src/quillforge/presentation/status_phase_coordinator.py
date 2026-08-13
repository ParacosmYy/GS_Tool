"""Qt-free projection of application activity into the shell phase."""

from __future__ import annotations

from dataclasses import dataclass

from .status_phase_contract import StatusPhase


@dataclass(frozen=True, slots=True)
class StatusPhaseInput:
    """Application-owned facts used to choose a non-error shell phase."""

    busy: bool
    pending_work: bool
    active_document_dirty: bool


class StatusPhaseCoordinator:
    """Keep shell phase priority deterministic and framework-neutral."""

    def project(self, state: StatusPhaseInput) -> StatusPhase:
        """Prefer retained work, then dirty attention, then clean readiness."""
        if state.busy or state.pending_work:
            return "working"
        if state.active_document_dirty:
            return "attention"
        return "ready"


__all__ = ["StatusPhaseCoordinator", "StatusPhaseInput"]
