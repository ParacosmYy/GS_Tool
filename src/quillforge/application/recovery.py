"""Recovery snapshot use cases independent of Qt and concrete storage."""

from collections.abc import Iterable
from dataclasses import dataclass
from time import time_ns
from typing import Literal
from uuid import uuid4

from ..domain.models import DocumentState, RecoverySnapshot, RecoverySnapshotMetadata
from .documents import OpenedDocument
from .errors import ApplicationValidationError
from .ports import DocumentStore, RecoverySnapshotChunkStore, RecoverySnapshotStore

RecoverySourceStatus = Literal["untitled", "unchanged", "changed", "missing", "unavailable"]


@dataclass(frozen=True, slots=True)
class RecoveryCandidate:
    """A snapshot plus its non-UI source-file explanation."""

    snapshot: RecoverySnapshot
    source_status: RecoverySourceStatus


class RecoveryService:
    """Creates and restores opt-in snapshots while delegating persistence."""

    def __init__(
        self,
        store: RecoverySnapshotStore,
        documents: DocumentStore,
        chunk_store: RecoverySnapshotChunkStore | None = None,
    ) -> None:
        self._store = store
        self._documents = documents
        self._chunk_store = chunk_store

    def list_candidates(self) -> tuple[RecoverySnapshot, ...]:
        """Return snapshots that require an explicit user decision."""
        return self._store.list_snapshots()

    def scan_candidates(self) -> tuple[RecoveryCandidate, ...]:
        """Load snapshots and classify source files for a worker-thread UI handoff."""
        return tuple(
            RecoveryCandidate(snapshot, self.source_status(snapshot))
            for snapshot in self._store.list_snapshots()
        )

    def new_snapshot_id(self) -> str:
        """Allocate a stable ID before an asynchronous snapshot write."""
        return uuid4().hex

    def save_snapshot(
        self,
        state: DocumentState,
        text: str,
        *,
        snapshot_id: str,
    ) -> RecoverySnapshot:
        """Persist one immutable snapshot of dirty document content."""
        metadata = self._snapshot_metadata(state, snapshot_id=snapshot_id)
        snapshot = RecoverySnapshot(
            snapshot_id=metadata.snapshot_id,
            document_id=metadata.document_id,
            path=metadata.path,
            text=text,
            encoding=metadata.encoding,
            line_ending=metadata.line_ending,
            source_revision=metadata.source_revision,
            created_at_ns=metadata.created_at_ns,
        )
        self._store.save_snapshot(snapshot)
        return snapshot

    def save_snapshot_chunks(
        self,
        state: DocumentState,
        chunks: Iterable[str],
        *,
        snapshot_id: str,
    ) -> RecoverySnapshotMetadata:
        """Persist captured chunks without joining them in the worker first."""
        metadata = self._snapshot_metadata(state, snapshot_id=snapshot_id)
        if self._chunk_store is None:
            self._store.save_snapshot(
                RecoverySnapshot(
                    snapshot_id=metadata.snapshot_id,
                    document_id=metadata.document_id,
                    path=metadata.path,
                    text="".join(chunks),
                    encoding=metadata.encoding,
                    line_ending=metadata.line_ending,
                    source_revision=metadata.source_revision,
                    created_at_ns=metadata.created_at_ns,
                )
            )
        else:
            self._chunk_store.save_snapshot_chunks(metadata, chunks)
        return metadata

    def _snapshot_metadata(
        self,
        state: DocumentState,
        *,
        snapshot_id: str,
    ) -> RecoverySnapshotMetadata:
        if not state.dirty:
            raise ApplicationValidationError("Only dirty documents can create recovery snapshots")
        return RecoverySnapshotMetadata(
            snapshot_id=snapshot_id,
            document_id=state.document_id,
            path=state.path,
            encoding=state.encoding,
            line_ending=state.line_ending,
            source_revision=state.revision,
            created_at_ns=time_ns(),
        )

    def restore(self, snapshot: RecoverySnapshot) -> OpenedDocument:
        """Turn a snapshot into dirty document state guarded by its source revision."""
        state = DocumentState(
            document_id=snapshot.document_id,
            path=snapshot.path,
            encoding=snapshot.encoding,
            line_ending=snapshot.line_ending,
            dirty=True,
            revision=snapshot.source_revision,
        )
        return OpenedDocument(state, snapshot.text)

    def source_status(self, snapshot: RecoverySnapshot) -> RecoverySourceStatus:
        """Explain whether the source file still matches the snapshot baseline."""
        if snapshot.path is None or snapshot.source_revision is None:
            return "untitled"
        try:
            current = self._documents.current_revision(snapshot.path)
        except OSError:
            return "unavailable"
        if current is None:
            return "missing"
        return "unchanged" if current == snapshot.source_revision else "changed"

    def delete_snapshot(self, snapshot_id: str) -> None:
        """Delete a snapshot after a user-visible lifecycle decision."""
        self._store.delete_snapshot(snapshot_id)
