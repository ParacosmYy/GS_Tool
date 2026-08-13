"""Qt-free presentation orchestration for Replace All completion cleanup."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from .replace_all_tracker import ReplaceAllJob, ReplaceAllTracker


@dataclass(frozen=True, slots=True)
class ReplaceAllCompletionPorts[TabT, SessionT, ProgressT]:
    """Typed callbacks required to release and project one Replace All job."""

    contains_tab: Callable[[TabT], bool]
    set_operation_locked: Callable[[TabT, bool], None]
    set_tab_bar_enabled: Callable[[bool], None]
    set_operation_active: Callable[[bool], None]
    complete_operation: Callable[[int], bool]
    project_outcome: Callable[
        [ReplaceAllJob[TabT, SessionT], ProgressT | None, Exception | None, str | None], None
    ]


class ReplaceAllCompletionCoordinator[TabT, SessionT, ProgressT]:
    """Release one Replace All lifecycle before projecting its outcome."""

    def __init__(
        self,
        tracker: ReplaceAllTracker[TabT, SessionT],
        ports: ReplaceAllCompletionPorts[TabT, SessionT, ProgressT],
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def finish(
        self,
        job: ReplaceAllJob[TabT, SessionT],
        progress: ProgressT | None,
        error: Exception | None,
        message: str | None = None,
    ) -> None:
        """Release a current job and delegate its product-specific outcome."""
        if not self._tracker.finish(job):
            return
        if self._ports.contains_tab(job.tab):
            self._ports.set_operation_locked(job.tab, False)
        self._ports.set_tab_bar_enabled(True)
        self._ports.set_operation_active(False)
        self._ports.complete_operation(job.operation_id)
        self._ports.project_outcome(job, progress, error, message)


__all__ = ["ReplaceAllCompletionCoordinator", "ReplaceAllCompletionPorts"]
