"""Framework-neutral lifecycle state for workspace-search callbacks."""

from __future__ import annotations

from dataclasses import dataclass
from threading import Event
from typing import Literal

WorkspaceSearchCompletion = Literal["stale", "invalidated", "current"]


@dataclass(slots=True)
class WorkspaceSearchOperationTracker:
    """Own search identity, generation invalidation, and cooperative cancel state.

    The tracker does not know about Qt, TaskRunner, the search service, the
    dialog, notifications, containment, or startup policy. It only makes the
    callback-lifecycle invariant explicit for the presentation owner.
    """

    _generation: int = 0
    _active_operation_id: int | None = None
    _cancellation_event: Event | None = None

    @property
    def active_operation_id(self) -> int | None:
        """Return the operation whose callback is still allowed to complete."""
        return self._active_operation_id

    @property
    def cancellation_event(self) -> Event | None:
        """Return the current worker cancellation event, if a search is active."""
        return self._cancellation_event

    def begin(self, operation_id: int) -> tuple[int, Event]:
        """Start one search generation and return its immutable callback inputs."""
        if type(operation_id) is not int or operation_id < 1:
            raise ValueError("workspace search operation ID must be a positive integer")
        if self._active_operation_id is not None:
            raise RuntimeError("workspace search already has an active operation")
        self._generation += 1
        cancellation_event = Event()
        self._active_operation_id = operation_id
        self._cancellation_event = cancellation_event
        return self._generation, cancellation_event

    def in_flight(self) -> bool:
        """Return whether a completion callback is still outstanding."""
        return self._active_operation_id is not None

    def cancel(self) -> bool:
        """Request cooperative cancellation without invalidating the generation."""
        if self._active_operation_id is None or self._cancellation_event is None:
            return False
        self._cancellation_event.set()
        return True

    def invalidate(self) -> bool:
        """Invalidate the current root generation and request worker cancellation."""
        if self._active_operation_id is None:
            return False
        self._generation += 1
        return self.cancel()

    def finish(self, operation_id: int, generation: int) -> WorkspaceSearchCompletion:
        """Classify and consume one callback without clearing a newer operation."""
        if self._active_operation_id != operation_id:
            return "stale"
        current = generation == self._generation
        self._active_operation_id = None
        self._cancellation_event = None
        return "current" if current else "invalidated"
