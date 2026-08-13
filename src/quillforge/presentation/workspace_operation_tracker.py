"""Framework-neutral lifecycle state for workspace navigation callbacks."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal

WorkspaceOperationCompletion = Literal["stale", "invalidated", "current"]


@dataclass(slots=True)
class WorkspaceOperationTracker:
    """Own workspace operation identity and generation invalidation only.

    Busy state, TaskRunner lifetime, status projection, workspace service
    policy, and session-restore barriers remain owned by MainWindow.
    """

    _generation: int = 0
    _active_operation_id: int | None = None

    @property
    def active_operation_id(self) -> int | None:
        """Return the current workspace operation, if any."""
        return self._active_operation_id

    def begin(self, operation_id: int) -> int:
        """Start one navigation generation for a positive operation ID."""
        if type(operation_id) is not int or operation_id < 1:
            raise ValueError("workspace operation ID must be a positive integer")
        if self._active_operation_id is not None:
            raise RuntimeError("workspace already has an active operation")
        self._generation += 1
        self._active_operation_id = operation_id
        return self._generation

    def in_flight(self) -> bool:
        """Return whether a workspace callback is still outstanding."""
        return self._active_operation_id is not None

    def invalidate(self) -> int | None:
        """Invalidate the current generation and consume its active identity."""
        operation_id = self._active_operation_id
        if operation_id is None:
            return None
        self._generation += 1
        self._active_operation_id = None
        return operation_id

    def finish(self, operation_id: int, generation: int) -> WorkspaceOperationCompletion:
        """Classify one callback without clearing a different current operation."""
        if self._active_operation_id != operation_id:
            return "stale"
        current = generation == self._generation
        self._active_operation_id = None
        return "current" if current else "invalidated"
