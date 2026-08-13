"""Framework-neutral latest-wins state for local session persistence."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal

from ..domain.models import SessionSnapshot

SessionSaveCompletion = Literal["stale", "invalid", "valid"]


@dataclass(slots=True)
class SessionSaveTracker:
    """Own debounce output and one in-flight session-save identity.

    The session-save coordinator owns request/begin/complete sequencing around
    this tracker. MainWindow remains responsible for the timer, snapshot
    capture, SessionService, TaskRunner dispatch, notifications, and close
    policy. This tracker only makes latest-wins persistence state explicit and
    framework-neutral.
    """

    _last_saved: SessionSnapshot | None = None
    _requested: SessionSnapshot | None = None
    _operation_id: int | None = None
    _inflight: bool = False

    @property
    def last_saved(self) -> SessionSnapshot | None:
        """Return the most recently accepted session snapshot."""
        return self._last_saved

    @property
    def has_request(self) -> bool:
        """Return whether the latest UI snapshot is waiting to be saved."""
        return self._requested is not None

    @property
    def inflight(self) -> bool:
        """Return whether a save callback is still outstanding."""
        return self._inflight

    @property
    def operation_id(self) -> int | None:
        """Return the operation ID bound to the current save callback."""
        return self._operation_id

    def set_last_saved(self, snapshot: SessionSnapshot) -> None:
        """Record the baseline loaded during startup restore."""
        _require_snapshot(snapshot)
        self._last_saved = snapshot

    def request(self, snapshot: SessionSnapshot) -> bool:
        """Queue the newest snapshot unless it is already fully persisted."""
        _require_snapshot(snapshot)
        if snapshot == self._last_saved and self._requested is None and not self._inflight:
            return False
        self._requested = snapshot
        return True

    def begin(self, operation_id: int) -> SessionSnapshot | None:
        """Bind one queued snapshot to a TaskRunner operation."""
        _require_operation_id(operation_id)
        if self._inflight or self._operation_id is not None or self._requested is None:
            return None
        snapshot = self._requested
        self._requested = None
        self._operation_id = operation_id
        self._inflight = True
        return snapshot

    def complete(
        self,
        operation_id: int,
        result: object,
    ) -> SessionSaveCompletion:
        """Classify a callback without disturbing a newer save operation."""
        _require_operation_id(operation_id)
        if self._operation_id != operation_id:
            return "stale"
        self._operation_id = None
        self._inflight = False
        if not isinstance(result, SessionSnapshot):
            return "invalid"
        self._last_saved = result
        return "valid"

    def fail(self, operation_id: int) -> bool:
        """Consume a matching failed callback and preserve the queued latest state."""
        _require_operation_id(operation_id)
        if self._operation_id != operation_id:
            return False
        self._operation_id = None
        self._inflight = False
        return True


def _require_snapshot(snapshot: SessionSnapshot) -> None:
    if not isinstance(snapshot, SessionSnapshot):
        raise TypeError("session snapshot must be a SessionSnapshot")


def _require_operation_id(operation_id: int) -> None:
    if type(operation_id) is not int or operation_id < 1:
        raise ValueError("session save operation ID must be a positive integer")
