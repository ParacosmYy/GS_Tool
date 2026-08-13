"""Framework-neutral lifecycle state for cooperative recovery capture."""

from __future__ import annotations

from dataclasses import dataclass, field


@dataclass(frozen=True, slots=True)
class RecoveryDeleteRequest[OwnerT]:
    """Presentation owner and success projection waiting for snapshot deletion."""

    tab: OwnerT | None
    success_message: str | None


@dataclass(slots=True)
class RecoveryCaptureTracker[JobT, OwnerT]:
    """Own recovery lifecycle indexes without owning services or Qt objects.

    MainWindow retains editor capture, channel backpressure, RecoveryService,
    TaskRunner, notifications, tab identity, and close policy. This tracker
    only keeps the cross-callback identity and delete-after-write invariants
    explicit.
    """

    _inflight_documents: set[str] = field(default_factory=set)
    _jobs_by_document: dict[str, JobT] = field(default_factory=dict)
    _document_by_snapshot: dict[str, str] = field(default_factory=dict)
    _inflight_snapshots: set[str] = field(default_factory=set)
    _discarded_snapshots: set[str] = field(default_factory=set)
    _delete_inflight: set[str] = field(default_factory=set)
    _delete_pending: dict[str, RecoveryDeleteRequest[OwnerT]] = field(default_factory=dict)

    @property
    def has_inflight_documents(self) -> bool:
        """Return whether a document capture or write is still outstanding."""
        return bool(self._inflight_documents)

    @property
    def has_inflight_snapshots(self) -> bool:
        """Return whether a worker still owns a recovery snapshot write."""
        return bool(self._inflight_snapshots)

    @property
    def has_delete_inflight(self) -> bool:
        """Return whether a recovery snapshot delete callback is outstanding."""
        return bool(self._delete_inflight)

    def document_in_flight(self, document_id: str) -> bool:
        """Return whether one document already has capture/write lifecycle state."""
        _require_id(document_id, "document ID")
        return document_id in self._inflight_documents

    def delete_in_flight_or_pending(self, snapshot_id: str) -> bool:
        """Return whether deletion already owns or awaits this snapshot."""
        _require_id(snapshot_id, "snapshot ID")
        return snapshot_id in self._delete_inflight or snapshot_id in self._delete_pending

    def begin_capture(self, document_id: str, snapshot_id: str, job: JobT) -> None:
        """Register one document capture before UI slices or worker dispatch."""
        _require_id(document_id, "document ID")
        _require_id(snapshot_id, "snapshot ID")
        if document_id in self._inflight_documents or document_id in self._jobs_by_document:
            raise RuntimeError("A recovery capture is already active for this document")
        if snapshot_id in self._document_by_snapshot:
            raise RuntimeError("A recovery capture already owns this snapshot")
        self._inflight_documents.add(document_id)
        self._jobs_by_document[document_id] = job
        self._document_by_snapshot[snapshot_id] = document_id

    def capture_for_document(self, document_id: str) -> JobT | None:
        """Return the active opaque capture job for one document."""
        _require_id(document_id, "document ID")
        return self._jobs_by_document.get(document_id)

    def capture_for_snapshot(self, snapshot_id: str) -> JobT | None:
        """Return the active opaque capture job for one snapshot, if any."""
        _require_id(snapshot_id, "snapshot ID")
        document_id = self._document_by_snapshot.get(snapshot_id)
        if document_id is None:
            return None
        return self._jobs_by_document.get(document_id)

    def finish_capture(self, document_id: str, job: JobT) -> bool:
        """Remove the UI producer while retaining document write lifecycle state."""
        _require_id(document_id, "document ID")
        if self._jobs_by_document.get(document_id) is not job:
            return False
        self._jobs_by_document.pop(document_id, None)
        self._remove_snapshot_binding(document_id)
        return True

    def abort_capture(self, document_id: str, job: JobT) -> bool:
        """Remove a matching producer and release its document lifecycle state."""
        if not self.finish_capture(document_id, job):
            return False
        self.complete_document(document_id)
        return True

    def complete_document(self, document_id: str) -> bool:
        """Release a document after its capture/write callback is classified."""
        _require_id(document_id, "document ID")
        was_inflight = document_id in self._inflight_documents
        self._inflight_documents.discard(document_id)
        return was_inflight

    def mark_snapshot_inflight(self, snapshot_id: str) -> None:
        """Mark a snapshot as owned by a worker write."""
        _require_id(snapshot_id, "snapshot ID")
        self._inflight_snapshots.add(snapshot_id)

    def release_snapshot(self, snapshot_id: str) -> None:
        """Release a snapshot whose worker never started or has been discarded."""
        _require_id(snapshot_id, "snapshot ID")
        self._inflight_snapshots.discard(snapshot_id)

    def mark_discarded(self, snapshot_id: str) -> None:
        """Remember a worker callback that must delete rather than publish."""
        _require_id(snapshot_id, "snapshot ID")
        self._discarded_snapshots.add(snapshot_id)

    def consume_discarded(self, snapshot_id: str) -> bool:
        """Consume and classify a discarded worker callback exactly once."""
        _require_id(snapshot_id, "snapshot ID")
        if snapshot_id not in self._discarded_snapshots:
            return False
        self._discarded_snapshots.remove(snapshot_id)
        return True

    def finish_write(self, snapshot_id: str) -> RecoveryDeleteRequest[OwnerT] | None:
        """Release a worker write and return any deferred delete request."""
        _require_id(snapshot_id, "snapshot ID")
        self._inflight_snapshots.discard(snapshot_id)
        return self._delete_pending.pop(snapshot_id, None)

    def request_delete(
        self,
        snapshot_id: str,
        *,
        tab: OwnerT | None,
        success_message: str | None,
    ) -> bool:
        """Queue deletion when write/delete state is busy, or claim the delete slot."""
        _require_id(snapshot_id, "snapshot ID")
        _require_message(success_message)
        request = RecoveryDeleteRequest(tab, success_message)
        if snapshot_id in self._inflight_snapshots or snapshot_id in self._delete_inflight:
            self._delete_pending[snapshot_id] = request
            return False
        self._delete_inflight.add(snapshot_id)
        return True

    def complete_delete(self, snapshot_id: str) -> RecoveryDeleteRequest[OwnerT] | None:
        """Release a successful delete and return the next deferred request."""
        _require_id(snapshot_id, "snapshot ID")
        self._delete_inflight.discard(snapshot_id)
        return self._delete_pending.pop(snapshot_id, None)

    def fail_delete(self, snapshot_id: str) -> None:
        """Release a failed delete while preserving any existing deferred request."""
        _require_id(snapshot_id, "snapshot ID")
        self._delete_inflight.discard(snapshot_id)

    def _remove_snapshot_binding(self, document_id: str) -> None:
        snapshot_id = next(
            (
                snapshot_id
                for snapshot_id, bound_document_id in self._document_by_snapshot.items()
                if bound_document_id == document_id
            ),
            None,
        )
        if snapshot_id is not None:
            self._document_by_snapshot.pop(snapshot_id, None)


def _require_id(value: str, label: str) -> None:
    if type(value) is not str or not value:
        raise ValueError(f"{label} must be a non-empty string")


def _require_message(message: str | None) -> None:
    if message is not None and not isinstance(message, str):
        raise TypeError("recovery delete success message must be a string or None")
