"""Framework-neutral lifecycle state for one recovery inventory scan."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class RecoveryScanJob:
    """Identity and startup context carried by one scan callback chain."""

    operation_id: int
    startup: bool


@dataclass(slots=True)
class RecoveryScanTracker:
    """Guard scan completion without owning RecoveryService or UI policy."""

    _active_job: RecoveryScanJob | None = None

    @property
    def active_job(self) -> RecoveryScanJob | None:
        """Return the currently bound scan job, if any."""
        return self._active_job

    @property
    def inflight(self) -> bool:
        """Return whether a recovery inventory scan is awaiting a callback."""
        return self._active_job is not None

    def begin(self, *, operation_id: int, startup: bool) -> RecoveryScanJob | None:
        """Bind one operation and context without overwriting an active scan."""
        if self._active_job is not None:
            return None
        if type(operation_id) is not int or operation_id < 1:
            raise ValueError("operation_id must be a positive integer")
        job = RecoveryScanJob(operation_id, startup)
        self._active_job = job
        return job

    def is_current(self, job: RecoveryScanJob) -> bool:
        """Return whether a callback still belongs to the active scan."""
        return self._active_job is job

    def finish(self, job: RecoveryScanJob) -> bool:
        """Release only the matching scan callback identity."""
        if not self.is_current(job):
            return False
        self._active_job = None
        return True
