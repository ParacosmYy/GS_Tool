"""Qt-free orchestration for cancelling or failing recovery captures."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..application.ports import RecoveryChunkChannel
from .recovery_capture_tracker import RecoveryCaptureTracker


@dataclass(frozen=True, slots=True)
class RecoveryCaptureAbortPorts[JobT, OwnerT]:
    """Typed callbacks required to abort one matching recovery capture."""

    owner_for_job: Callable[[JobT], OwnerT]
    document_id: Callable[[OwnerT], str]
    snapshot_id: Callable[[JobT], str]
    channel: Callable[[JobT], RecoveryChunkChannel | None]
    cancel_session: Callable[[JobT], None]
    notify_failure: Callable[[OwnerT, Exception], None]
    is_live: Callable[[OwnerT], bool]


class RecoveryCaptureAbortCoordinator[JobT, OwnerT]:
    """Close a matching capture without owning editor or notification policy."""

    def __init__(
        self,
        tracker: RecoveryCaptureTracker[JobT, OwnerT],
        ports: RecoveryCaptureAbortPorts[JobT, OwnerT],
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def abort(
        self,
        job: JobT,
        reason: Exception,
        *,
        notify: bool,
        worker_started: bool = True,
    ) -> bool:
        """Abort a matching producer and release its document lifecycle."""
        owner = self._ports.owner_for_job(job)
        document_id = self._ports.document_id(owner)
        if self._tracker.capture_for_document(document_id) is not job:
            return False

        self._tracker.finish_capture(document_id, job)
        snapshot_id = self._ports.snapshot_id(job)
        channel = self._ports.channel(job)
        if channel is not None:
            if worker_started:
                self._tracker.mark_discarded(snapshot_id)
            else:
                self._tracker.release_snapshot(snapshot_id)
            channel.abort(reason)
        self._ports.cancel_session(job)
        self._tracker.complete_document(document_id)
        if notify and self._ports.is_live(owner):
            self._ports.notify_failure(owner, reason)
        return True


__all__ = ["RecoveryCaptureAbortCoordinator", "RecoveryCaptureAbortPorts"]
