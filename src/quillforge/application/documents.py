"""Document use cases independent of Qt and concrete filesystem APIs."""

from dataclasses import dataclass, replace
from pathlib import Path
from uuid import uuid4

from ..domain.models import DocumentState
from .errors import ApplicationValidationError
from .ports import DocumentStore


@dataclass(frozen=True, slots=True)
class OpenedDocument:
    """Document state plus decoded text ready for a presentation adapter."""

    state: DocumentState
    text: str


class DocumentService:
    """Coordinates document lifecycle while delegating persistence to a port."""

    def __init__(self, store: DocumentStore) -> None:
        self._store = store

    def new_document(self) -> OpenedDocument:
        """Create an untitled document without touching the filesystem."""
        return OpenedDocument(DocumentState(document_id=uuid4().hex), "")

    def open_document(self, path: Path) -> OpenedDocument:
        """Load a document; callers may run this use case off the UI thread."""
        resolved_path = path.expanduser().resolve()
        loaded = self._store.load(resolved_path)
        state = DocumentState(
            document_id=uuid4().hex,
            path=resolved_path,
            encoding=loaded.encoding,
            line_ending=loaded.line_ending,
            revision=loaded.revision,
        )
        return OpenedDocument(state, loaded.text)

    def save_document(
        self,
        state: DocumentState,
        text: str,
        target: Path | None = None,
    ) -> DocumentState:
        """Persist a snapshot and return the next immutable document state."""
        path = target or state.path
        if path is None:
            raise ApplicationValidationError(
                "A target path is required to save an untitled document"
            )

        resolved_path = path.expanduser().resolve()
        expected_revision = state.revision if resolved_path == state.path else None
        revision = self._store.save(
            resolved_path,
            text,
            encoding=state.encoding,
            line_ending=state.line_ending,
            expected_revision=expected_revision,
        )
        return replace(
            state,
            path=resolved_path,
            dirty=False,
            revision=revision,
        )

    def mark_dirty(self, state: DocumentState, dirty: bool) -> DocumentState:
        """Return a state transition without mutating the existing snapshot."""
        return replace(state, dirty=dirty)
