"""Framework-neutral lifecycle state for one cooperative Replace All job."""

from __future__ import annotations

from dataclasses import dataclass
from typing import TypeVar

TabT = TypeVar("TabT")
SessionT = TypeVar("SessionT")


@dataclass(slots=True)
class ReplaceAllJob[TabT, SessionT]:
    """Opaque presentation state shared by one Replace All callback chain."""

    tab: TabT
    session: SessionT
    operation_id: int
    expected_content_version: int
    was_dirty: bool
    last_report_ns: int = 0
    last_report_count: int = -1


@dataclass(slots=True)
class ReplaceAllTracker[TabT, SessionT]:
    """Own the active job identity without owning editor, timer, or UI policy.

    QTimer callbacks carry the returned job object and must pass it back to
    :meth:`is_current`. This prevents a callback already queued for a cancelled
    job from advancing a newer Replace All operation.
    """

    _active_job: ReplaceAllJob[TabT, SessionT] | None = None

    @property
    def active_job(self) -> ReplaceAllJob[TabT, SessionT] | None:
        """Return the current job, if one is still owned by this tracker."""
        return self._active_job

    @property
    def inflight(self) -> bool:
        """Return whether a Replace All callback chain is active."""
        return self._active_job is not None

    def begin(
        self,
        *,
        tab: TabT,
        session: SessionT,
        operation_id: int,
        expected_content_version: int,
        was_dirty: bool,
    ) -> ReplaceAllJob[TabT, SessionT] | None:
        """Register a job, rejecting a second active chain without overwriting it."""
        if self._active_job is not None:
            return None
        if type(operation_id) is not int or operation_id < 1:
            raise ValueError("operation_id must be a positive integer")
        if type(expected_content_version) is not int or expected_content_version < 0:
            raise ValueError("expected_content_version must be a non-negative integer")
        job = ReplaceAllJob(
            tab=tab,
            session=session,
            operation_id=operation_id,
            expected_content_version=expected_content_version,
            was_dirty=was_dirty,
        )
        self._active_job = job
        return job

    def is_current(self, job: ReplaceAllJob[TabT, SessionT]) -> bool:
        """Return whether a callback still belongs to the active job."""
        return self._active_job is job

    def update_expected_content_version(
        self,
        job: ReplaceAllJob[TabT, SessionT],
        content_version: int,
    ) -> bool:
        """Update the content guard only for the still-current callback chain."""
        if not self.is_current(job):
            return False
        if type(content_version) is not int or content_version < 0:
            raise ValueError("content_version must be a non-negative integer")
        job.expected_content_version = content_version
        return True

    def finish(self, job: ReplaceAllJob[TabT, SessionT]) -> bool:
        """Release the job only when its callback identity is still current."""
        if not self.is_current(job):
            return False
        self._active_job = None
        return True
