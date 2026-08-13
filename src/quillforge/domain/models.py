"""Domain models that do not depend on Qt or the filesystem."""

from dataclasses import dataclass
from pathlib import Path
from typing import Literal

LineEnding = Literal["LF", "CRLF", "CR"]
WorkspaceEntryKind = Literal["file", "directory", "inaccessible"]
SESSION_SCHEMA_VERSION = 1
Locale = Literal["en-US", "zh-CN"]
ThemeId = Literal["ink-violet", "paper-sand", "sakura-pop"]
AccentId = Literal["violet", "cyan", "rose", "amber"]
FontStyle = Literal["regular", "semibold", "bold", "italic"]


@dataclass(frozen=True, slots=True)
class DocumentRevision:
    """Small disk identity used to detect an external file change."""

    modified_ns: int
    size: int


@dataclass(frozen=True, slots=True)
class DocumentState:
    """Stable metadata for an open document."""

    document_id: str
    path: Path | None = None
    encoding: str = "utf-8"
    line_ending: LineEnding = "LF"
    dirty: bool = False
    revision: DocumentRevision | None = None


@dataclass(frozen=True, slots=True)
class LoadedDocument:
    """Decoded document content returned by a document store."""

    text: str
    encoding: str
    line_ending: LineEnding
    revision: DocumentRevision


@dataclass(frozen=True, slots=True)
class RecoverySnapshot:
    """A user-created document snapshot kept outside the original file."""

    snapshot_id: str
    document_id: str
    path: Path | None
    text: str
    encoding: str
    line_ending: LineEnding
    source_revision: DocumentRevision | None
    created_at_ns: int


@dataclass(frozen=True, slots=True)
class RecoverySnapshotMetadata:
    """Immutable recovery envelope fields independent of the text payload."""

    snapshot_id: str
    document_id: str
    path: Path | None
    encoding: str
    line_ending: LineEnding
    source_revision: DocumentRevision | None
    created_at_ns: int


@dataclass(frozen=True, slots=True)
class WorkspaceEntry:
    """One bounded directory entry projected without filesystem behavior."""

    path: Path
    name: str
    kind: WorkspaceEntryKind
    error: str | None = None


@dataclass(frozen=True, slots=True)
class WorkspaceDirectory:
    """A finite directory page returned by the workspace provider."""

    path: Path
    entries: tuple[WorkspaceEntry, ...]
    truncated: bool = False


@dataclass(frozen=True, slots=True)
class EditorSettings:
    """Small, validated editor preference set owned by the application."""

    font_family: str = "Cascadia Code"
    font_size: int = 11
    font_style: FontStyle = "regular"
    wrap_lines: bool = False
    show_line_numbers: bool = True


@dataclass(frozen=True, slots=True)
class AppearanceSettings:
    """Persisted language, theme, and application-font preferences."""

    locale: Locale = "zh-CN"
    theme: ThemeId = "sakura-pop"
    accent: AccentId = "rose"
    ui_font_family: str = "Microsoft YaHei UI"
    ui_font_size: int = 10
    ui_font_style: FontStyle = "regular"
    motion_enabled: bool = True


@dataclass(frozen=True, slots=True)
class SettingsSnapshot:
    """Versioned persisted settings envelope."""

    schema_version: int
    editor: EditorSettings
    appearance: AppearanceSettings = AppearanceSettings()


@dataclass(frozen=True, slots=True)
class SessionDocument:
    """One path-backed document remembered by the local session."""

    path: Path
    line: int = 0
    column: int = 0

    def __post_init__(self) -> None:
        if not isinstance(self.path, Path) or not self.path.is_absolute():
            raise ValueError("Session document paths must be absolute")
        if type(self.line) is not int or self.line < 0:
            raise ValueError("Session document line must be a non-negative integer")
        if type(self.column) is not int or self.column < 0:
            raise ValueError("Session document column must be a non-negative integer")


@dataclass(frozen=True, slots=True)
class SessionSnapshot:
    """Versioned local UI continuity state without document content."""

    schema_version: int = SESSION_SCHEMA_VERSION
    workspace_root: Path | None = None
    documents: tuple[SessionDocument, ...] = ()
    active_index: int = 0

    def __post_init__(self) -> None:
        if type(self.schema_version) is not int or self.schema_version < 1:
            raise ValueError("Session schema version must be a positive integer")
        if self.workspace_root is not None and (
            not isinstance(self.workspace_root, Path) or not self.workspace_root.is_absolute()
        ):
            raise ValueError("Session workspace roots must be absolute")
        if not isinstance(self.documents, tuple) or not all(
            isinstance(document, SessionDocument) for document in self.documents
        ):
            raise ValueError("Session documents must be an immutable tuple")
        if type(self.active_index) is not int or self.active_index < 0:
            raise ValueError("Session active index must be a non-negative integer")
        if self.documents and self.active_index >= len(self.documents):
            raise ValueError("Session active index must identify a remembered document")


class DocumentConflictError(RuntimeError):
    """Raised when a file changed after it was loaded."""

    def __init__(self, path: Path) -> None:
        super().__init__(f"The document changed outside QuillForge: {path}")
        self.path = path
