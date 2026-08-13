"""Application ports implemented by infrastructure or presentation adapters."""

from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Literal, Protocol

from ..domain.models import (
    DocumentRevision,
    LineEnding,
    LoadedDocument,
    RecoverySnapshot,
    RecoverySnapshotMetadata,
    SessionSnapshot,
    SettingsSnapshot,
    WorkspaceDirectory,
)
from .errors import ApplicationValidationError
from .release_metadata import ReleaseManifest

RecoveryChannelState = Literal["open", "finished", "aborted"]


class RecoveryCaptureCancelled(RuntimeError):
    """Typed cancellation used when a producer becomes stale or is discarded."""


class RecoveryChannelAborted(RuntimeError):
    """Raised by a worker consumer after the producer aborts a channel."""

    def __init__(self, reason: Exception | None = None) -> None:
        self.reason = reason
        message = "Recovery chunk channel aborted"
        if reason is not None:
            message = f"{message}: {reason}"
        super().__init__(message)


class RecoveryChunkChannel(Protocol):
    """Non-blocking producer/consumer handoff for recovery text chunks."""

    @property
    def state(self) -> RecoveryChannelState:
        """Return the channel lifecycle state."""

    @property
    def error(self) -> Exception | None:
        """Return a worker or producer error visible to the coordinator."""

    @property
    def queued_chunks(self) -> int:
        """Return the currently buffered chunk count."""

    @property
    def queued_bytes(self) -> int:
        """Return the currently buffered UTF-8 byte count."""

    @property
    def peak_queued_chunks(self) -> int:
        """Return the maximum buffered chunk count observed by the channel."""

    @property
    def peak_queued_bytes(self) -> int:
        """Return the maximum buffered UTF-8 bytes observed by the channel."""

    def offer(self, chunk: str) -> bool:
        """Accept one chunk without blocking, or return False for backpressure."""

    def finish(self) -> bool:
        """Close the producer side after the final chunk was accepted."""

    def abort(self, reason: Exception | None = None) -> bool:
        """Abort production and wake the worker without blocking the caller."""

    def consume(self) -> Iterable[str]:
        """Yield accepted chunks until finish or raise after abort."""


class DocumentStore(Protocol):
    """Persistence port for decoded text documents."""

    def load(self, path: Path) -> LoadedDocument:
        """Read and decode a document without making UI assumptions."""

    def save(
        self,
        path: Path,
        text: str,
        *,
        encoding: str,
        line_ending: LineEnding,
        expected_revision: DocumentRevision | None,
    ) -> DocumentRevision:
        """Atomically save a document after checking its expected revision."""

    def current_revision(self, path: Path) -> DocumentRevision | None:
        """Return the current disk revision, or None when the path is absent."""


class RecoverySnapshotStore(Protocol):
    """Persist recovery snapshots without owning recovery policy."""

    def list_snapshots(self) -> tuple[RecoverySnapshot, ...]:
        """Return valid snapshots ordered for user review."""

    def save_snapshot(self, snapshot: RecoverySnapshot) -> None:
        """Atomically create or replace one snapshot."""

    def delete_snapshot(self, snapshot_id: str) -> None:
        """Remove one snapshot after an explicit discard or successful save."""


class RecoverySnapshotChunkStore(Protocol):
    """Persist a recovery envelope while encoding text chunks incrementally."""

    def save_snapshot_chunks(
        self,
        metadata: RecoverySnapshotMetadata,
        chunks: Iterable[str],
    ) -> None:
        """Atomically persist metadata and text chunks without joining them first."""


class DirectoryCapability(Protocol):
    """Expose one filesystem directory predicate through an infrastructure port."""

    def is_directory(self, path: Path) -> bool:
        """Return whether the adapter can treat ``path`` as a directory."""


class WorkspaceProvider(DirectoryCapability, Protocol):
    """Enumerate one explicitly requested directory without UI assumptions."""

    def list_directory(self, path: Path, *, limit: int) -> WorkspaceDirectory:
        """Return one bounded directory page."""


class SettingsStore(Protocol):
    """Persist a versioned settings envelope without owning defaults or migration."""

    def load(self) -> SettingsSnapshot | None:
        """Read settings, returning None for absent or unreadable data."""

    def save(self, settings: SettingsSnapshot) -> None:
        """Atomically persist one validated settings envelope."""


SessionLoadState = Literal["absent", "valid", "invalid"]


@dataclass(frozen=True, slots=True)
class SessionLoadResult:
    """Typed distinction between no manifest, valid state, and bad state."""

    state: SessionLoadState
    snapshot: SessionSnapshot | None = None

    def __post_init__(self) -> None:
        if self.state not in {"absent", "valid", "invalid"}:
            raise ApplicationValidationError(f"Unsupported session load state: {self.state!r}")
        if self.state == "valid" and not isinstance(self.snapshot, SessionSnapshot):
            raise ApplicationValidationError("A valid session load must contain a snapshot")
        if self.state != "valid" and self.snapshot is not None:
            raise ApplicationValidationError("Only a valid session load may contain a snapshot")


class SessionStore(Protocol):
    """Persist small local UI continuity state without document content."""

    def load(self) -> SessionLoadResult:
        """Read session state while preserving absent-versus-invalid semantics."""

    def save(self, session: SessionSnapshot) -> None:
        """Atomically persist one validated session envelope."""


@dataclass(frozen=True, slots=True)
class PluginCatalogCandidate:
    """One raw extension descriptor returned by an infrastructure adapter."""

    source_path: Path
    payload: object | None = None
    error: str | None = None


@dataclass(frozen=True, slots=True)
class PluginCatalogReadResult:
    """Bounded raw catalog input before application-level validation."""

    root: Path
    candidates: tuple[PluginCatalogCandidate, ...]
    truncated: bool = False
    error: str | None = None


class PluginCatalogStore(Protocol):
    """Read extension metadata without importing or executing extension code."""

    def read_candidates(self) -> PluginCatalogReadResult:
        """Read one bounded, explicitly configured catalog directory."""


PluginApprovalState = Literal["approved", "stale", "not-approved"]


@dataclass(frozen=True, slots=True)
class PluginApprovalRecord:
    """One explicit approval bound to an exact descriptor digest."""

    plugin_id: str
    descriptor_sha256: str
    approved_at_ns: int


@dataclass(frozen=True, slots=True)
class PluginApprovalReadResult:
    """Bounded approval ledger input with fail-closed diagnostics."""

    records: tuple[PluginApprovalRecord, ...]
    error: str | None = None


class PluginApprovalStore(Protocol):
    """Persist explicit descriptor governance without enabling execution."""

    def load(self) -> PluginApprovalReadResult:
        """Read approved descriptor records or return a safe error result."""

    def save(self, records: tuple[PluginApprovalRecord, ...]) -> None:
        """Atomically replace the bounded approval ledger."""


@dataclass(frozen=True, slots=True)
class PluginEnablementRecord:
    """One local enablement preference for an explicitly registered plugin."""

    plugin_id: str
    enabled: bool


@dataclass(frozen=True, slots=True)
class PluginEnablementReadResult:
    """Bounded enablement input with fail-closed diagnostics."""

    records: tuple[PluginEnablementRecord, ...]
    error: str | None = None


class PluginEnablementStore(Protocol):
    """Persist local lifecycle preferences without owning runtime callbacks."""

    def load(self) -> PluginEnablementReadResult:
        """Read preferences or return a safe error result."""

    def save(self, records: tuple[PluginEnablementRecord, ...]) -> None:
        """Atomically replace the bounded preference ledger."""


class ReleaseManifestStore(Protocol):
    """Persist one validated release-candidate manifest."""

    def load(self) -> ReleaseManifest:
        """Load a complete manifest or raise a typed metadata error."""

    def save(self, manifest: ReleaseManifest) -> None:
        """Atomically persist one complete manifest."""


class EditorEngine(Protocol):
    """Stable editor operations used by application-facing presentation code."""

    def get_text(self) -> str:
        """Return the current editor text."""

    def set_text(self, text: str) -> None:
        """Replace the current editor text."""

    def is_modified(self) -> bool:
        """Return whether the editor has unsaved changes."""

    def set_modified(self, modified: bool) -> None:
        """Update the editor's modified marker."""

    def set_read_only(self, read_only: bool) -> None:
        """Enable or disable editing while a document operation is active."""

    def set_operation_locked(self, locked: bool) -> None:
        """Block user input while allowing an adapter-owned operation to mutate safely."""

    def set_language(self, language: str | None) -> None:
        """Apply a language hint without exposing a concrete editor control."""

    def set_font_size(self, size: int) -> None:
        """Apply the validated editor font size."""

    def set_font_family(self, family: str) -> None:
        """Apply the validated editor font family."""

    def set_wrap_lines(self, enabled: bool) -> None:
        """Apply the validated line-wrap preference."""

    def set_line_numbers(self, enabled: bool) -> None:
        """Show or hide the editor line-number margin."""

    def set_theme(self, theme: str, accent: str = "violet") -> None:
        """Apply a presentation-owned editor theme identifier."""

    def cursor_position(self) -> tuple[int, int]:
        """Return the adapter-neutral zero-based caret line and column."""

    def set_cursor_position(self, line: int, column: int) -> None:
        """Restore a bounded zero-based caret position through the adapter."""

    def undo(self) -> None:
        """Undo the most recent editor-local change."""

    def redo(self) -> None:
        """Redo the most recently undone editor-local change."""

    def cut(self) -> None:
        """Cut the current selection through the editor adapter."""

    def copy(self) -> None:
        """Copy the current selection through the editor adapter."""

    def paste(self) -> None:
        """Paste clipboard text through the editor adapter."""

    def select_all(self) -> None:
        """Select all text through the editor adapter."""

    def has_selection(self) -> bool:
        """Return whether the editor has a current selection."""

    def selected_text(self) -> str:
        """Return the current selection without exposing control types."""

    def selection_bounds(self) -> tuple[int, int, int, int] | None:
        """Return a stable selection identity for one editor session."""

    def find_literal(self, query: str, *, case_sensitive: bool, forward: bool) -> bool:
        """Find a literal query in the active editor and select its match."""

    def go_to_line(self, line_number: int) -> None:
        """Move the editor view to a one-based line from an external search result."""

    def replace_selected_text(self, replacement: str) -> None:
        """Replace the current editor selection as one local edit."""

    def replace_all_literal(
        self,
        query: str,
        replacement: str,
        *,
        case_sensitive: bool,
    ) -> int:
        """Replace all literal matches and return the number of replacements."""
