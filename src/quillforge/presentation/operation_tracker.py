"""Framework-neutral lifecycle tracker for MainWindow-owned operations."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(slots=True)
class OperationTracker:
    """Allocate monotonic IDs and guard one active UI-owned operation.

    TaskRunner, busy gating, status phases, and use-case policy remain owned by
    their existing callers. This class only centralizes the ID/lifecycle
    invariant that a completion or cancellation can clear the current active
    operation only when its ID still matches.
    """

    _next_operation_id: int = 0
    _active_operation_id: int | None = None

    @property
    def active_operation_id(self) -> int | None:
        """Return the currently tracked active operation, if any."""
        return self._active_operation_id

    def reserve(self) -> int:
        """Return a fresh ID for operations with their own lifecycle state."""
        self._next_operation_id += 1
        return self._next_operation_id

    def begin(self) -> int:
        """Reserve and mark one operation as the current active operation."""
        operation_id = self.reserve()
        self._active_operation_id = operation_id
        return operation_id

    def complete(self, operation_id: int) -> bool:
        """Clear the active operation only when the completion is current."""
        if self._active_operation_id != operation_id:
            return False
        self._active_operation_id = None
        return True

    def cancel(self, operation_id: int) -> bool:
        """Clear the active operation using the same stale-ID guard as finish."""
        return self.complete(operation_id)
