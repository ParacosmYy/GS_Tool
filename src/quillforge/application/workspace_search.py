"""Application contracts for bounded, cancellable workspace search."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Literal, Protocol

from .errors import ApplicationTypeError, ApplicationValidationError
from .ports import DirectoryCapability

WorkspaceSearchLimitReason = Literal[
    "none",
    "max-files",
    "max-total-bytes",
    "max-file-bytes",
    "max-directory-entries",
    "max-line-bytes",
    "max-results",
    "max-depth",
    "max-issue-records",
]


@dataclass(frozen=True, slots=True)
class WorkspaceSearchPolicy:
    """Product-owned limits applied to every workspace search request."""

    max_files: int = 2_000
    max_total_bytes: int = 64 * 1024 * 1024
    max_file_bytes: int = 4 * 1024 * 1024
    max_directory_entries: int = 4_096
    max_results: int = 10_000
    max_depth: int = 32
    max_line_bytes: int = 256 * 1024
    max_preview_chars: int = 180
    max_issue_records: int = 32
    excluded_directory_names: tuple[str, ...] = (
        ".git",
        ".venv",
        "__pycache__",
        ".pytest_cache",
        "build",
        "dist",
        "node_modules",
    )

    def __post_init__(self) -> None:
        for name in (
            "max_files",
            "max_total_bytes",
            "max_file_bytes",
            "max_directory_entries",
            "max_results",
            "max_line_bytes",
            "max_preview_chars",
            "max_issue_records",
        ):
            value = getattr(self, name)
            if type(value) is not int or value < 1:
                raise ApplicationValidationError(f"{name} must be a positive integer")
        if type(self.max_depth) is not int or self.max_depth < 0:
            raise ApplicationValidationError("max_depth must be a non-negative integer")
        if self.max_file_bytes > self.max_total_bytes:
            raise ApplicationValidationError("max_file_bytes cannot exceed max_total_bytes")
        if not isinstance(self.excluded_directory_names, tuple):
            raise ApplicationTypeError("excluded_directory_names must be a tuple")
        normalized: set[str] = set()
        for directory_name in self.excluded_directory_names:
            if not isinstance(directory_name, str) or not directory_name:
                raise ApplicationValidationError(
                    "excluded directory names must be non-empty strings"
                )
            if Path(directory_name).name != directory_name or directory_name in {".", ".."}:
                raise ApplicationValidationError(
                    "excluded directory names must be single path components"
                )
            normalized.add(directory_name.casefold())
        if len(normalized) != len(self.excluded_directory_names):
            raise ApplicationValidationError(
                "excluded directory names must be unique case-insensitively"
            )


DEFAULT_WORKSPACE_SEARCH_POLICY = WorkspaceSearchPolicy()


@dataclass(frozen=True, slots=True)
class WorkspaceSearchQuery:
    """One explicit user search request, independent of Qt and filesystem APIs."""

    root: Path
    needle: str
    case_sensitive: bool = False

    def __post_init__(self) -> None:
        if not isinstance(self.root, Path):
            raise ApplicationTypeError("workspace search root must be a Path")
        if not isinstance(self.needle, str) or not self.needle:
            raise ApplicationValidationError("workspace search text must not be empty")
        if len(self.needle) > 1_024:
            raise ApplicationValidationError("workspace search text is too long")
        if type(self.case_sensitive) is not bool:
            raise ApplicationTypeError("case_sensitive must be a boolean")


@dataclass(frozen=True, slots=True)
class WorkspaceSearchRequest:
    """Validated query plus the immutable policy snapshot used by an adapter."""

    query: WorkspaceSearchQuery
    policy: WorkspaceSearchPolicy


@dataclass(frozen=True, slots=True)
class WorkspaceSearchMatch:
    """One literal match projected without exposing editor or widget state."""

    path: Path
    line: int
    column: int
    preview: str

    def __post_init__(self) -> None:
        if not isinstance(self.path, Path):
            raise ApplicationTypeError("workspace search match path must be a Path")
        if type(self.line) is not int or self.line < 1:
            raise ApplicationValidationError("workspace search match line must be positive")
        if type(self.column) is not int or self.column < 1:
            raise ApplicationValidationError("workspace search match column must be positive")
        if not isinstance(self.preview, str):
            raise ApplicationTypeError("workspace search match preview must be text")


@dataclass(frozen=True, slots=True)
class WorkspaceSearchIssue:
    """A bounded, recoverable file-level search diagnostic."""

    path: Path
    reason: str

    def __post_init__(self) -> None:
        if not isinstance(self.path, Path):
            raise ApplicationTypeError("workspace search issue path must be a Path")
        if not isinstance(self.reason, str) or not self.reason:
            raise ApplicationValidationError("workspace search issue reason must be non-empty text")
        if len(self.reason) > 240:
            raise ApplicationValidationError("workspace search issue reason is too long")


@dataclass(frozen=True, slots=True)
class WorkspaceSearchResult:
    """Immutable search outcome with explicit completeness and cancellation state."""

    query: WorkspaceSearchQuery
    matches: tuple[WorkspaceSearchMatch, ...] = ()
    files_scanned: int = 0
    files_skipped: int = 0
    bytes_scanned: int = 0
    truncated: bool = False
    cancelled: bool = False
    limit_reason: WorkspaceSearchLimitReason = "none"
    issues: tuple[WorkspaceSearchIssue, ...] = ()
    issues_truncated: bool = False

    def __post_init__(self) -> None:
        if not isinstance(self.query, WorkspaceSearchQuery):
            raise ApplicationTypeError("workspace search result query is invalid")
        if not isinstance(self.matches, tuple) or not isinstance(self.issues, tuple):
            raise ApplicationTypeError("workspace search result collections must be tuples")
        for name in ("files_scanned", "files_skipped", "bytes_scanned"):
            value = getattr(self, name)
            if type(value) is not int or value < 0:
                raise ApplicationValidationError(
                    f"workspace search result {name} must be non-negative"
                )
        if type(self.truncated) is not bool or type(self.cancelled) is not bool:
            raise ApplicationTypeError("workspace search result state must be boolean")
        if type(self.issues_truncated) is not bool:
            raise ApplicationTypeError("workspace search diagnostic state must be boolean")
        if self.issues_truncated and not self.truncated:
            raise ApplicationValidationError(
                "truncated diagnostics require a limited search result"
            )
        valid_reasons = {
            "none",
            "max-files",
            "max-total-bytes",
            "max-file-bytes",
            "max-directory-entries",
            "max-line-bytes",
            "max-results",
            "max-depth",
            "max-issue-records",
        }
        if self.limit_reason not in valid_reasons:
            raise ApplicationValidationError("workspace search result limit reason is invalid")
        if not all(isinstance(match, WorkspaceSearchMatch) for match in self.matches):
            raise ApplicationTypeError("workspace search result contains an invalid match")
        if not all(isinstance(issue, WorkspaceSearchIssue) for issue in self.issues):
            raise ApplicationTypeError("workspace search result contains an invalid issue")

    def summary(self) -> str:
        """Return a short bounded status suitable for the search dialog."""
        if self.cancelled:
            state = "Search cancelled"
        elif self.truncated:
            state = f"Search limited ({self.limit_reason})"
        else:
            state = "Search complete"
        diagnostic_suffix = " (diagnostics truncated)" if self.issues_truncated else ""
        return (
            f"{state}: {len(self.matches):,} matches in {self.files_scanned:,} files; "
            f"{self.bytes_scanned:,} bytes scanned{diagnostic_suffix}"
        )


class WorkspaceSearchProvider(DirectoryCapability, Protocol):
    """Infrastructure port for deterministic local workspace content search."""

    def search(
        self,
        request: WorkspaceSearchRequest,
        cancel_requested: Callable[[], bool],
    ) -> WorkspaceSearchResult:
        """Search one explicit root without owning UI state or policy mutation."""


class WorkspaceSearchService:
    """Validate one request and delegate bounded search to an infrastructure port."""

    def __init__(
        self,
        provider: WorkspaceSearchProvider,
        *,
        policy: WorkspaceSearchPolicy = DEFAULT_WORKSPACE_SEARCH_POLICY,
    ) -> None:
        self._provider = provider
        self._policy = policy

    @property
    def policy(self) -> WorkspaceSearchPolicy:
        """Expose the immutable product policy for presentation diagnostics."""
        return self._policy

    def search(
        self,
        query: WorkspaceSearchQuery,
        *,
        cancel_requested: Callable[[], bool] | None = None,
    ) -> WorkspaceSearchResult:
        """Run one validated search; callers may execute this method off the UI thread."""
        if not isinstance(query, WorkspaceSearchQuery):
            raise ApplicationTypeError("workspace search requires a WorkspaceSearchQuery")
        root = query.root.expanduser().resolve()
        if not self._provider.is_directory(root):
            raise ApplicationValidationError(f"Workspace search root does not exist: {root}")
        normalized_query = replace(query, root=root)
        if cancel_requested is None:
            cancellation = _never_cancelled
        elif not callable(cancel_requested):
            raise ApplicationTypeError("cancel_requested must be callable")
        else:
            cancellation = cancel_requested
        result = self._provider.search(
            WorkspaceSearchRequest(normalized_query, self._policy),
            cancellation,
        )
        if not isinstance(result, WorkspaceSearchResult):
            raise ApplicationTypeError("workspace search provider returned an invalid result")
        if result.query != normalized_query:
            raise ApplicationValidationError(
                "workspace search provider returned a mismatched query"
            )
        if len(result.matches) > self._policy.max_results:
            raise ApplicationValidationError("workspace search provider exceeded the result limit")
        if result.files_scanned > self._policy.max_files:
            raise ApplicationValidationError("workspace search provider exceeded the file limit")
        if result.bytes_scanned > self._policy.max_total_bytes:
            raise ApplicationValidationError("workspace search provider exceeded the byte limit")
        if result.files_scanned < 0 or result.files_skipped < 0 or result.bytes_scanned < 0:
            raise ApplicationValidationError("workspace search provider returned negative counters")
        if len(result.issues) > self._policy.max_issue_records:
            raise ApplicationValidationError("workspace search provider exceeded the issue limit")
        if result.issues_truncated and not result.truncated:
            raise ApplicationValidationError(
                "workspace search provider truncated diagnostics without a limit"
            )
        if result.limit_reason == "none" and result.truncated:
            raise ApplicationValidationError(
                "workspace search provider returned an unexplained truncation"
            )
        if result.limit_reason != "none" and not result.truncated:
            raise ApplicationValidationError(
                "workspace search provider returned a limit without truncation"
            )
        for match in result.matches:
            if len(match.preview) > self._policy.max_preview_chars:
                raise ApplicationValidationError(
                    "workspace search provider exceeded the preview limit"
                )
            if not match.path.is_absolute():
                raise ApplicationValidationError(
                    "workspace search provider returned a relative result path"
                )
            try:
                match.path.resolve().relative_to(normalized_query.root)
            except ValueError as error:
                raise ApplicationValidationError(
                    "workspace search provider returned a path outside the root"
                ) from error
        for issue in result.issues:
            if not issue.path.is_absolute():
                raise ApplicationValidationError(
                    "workspace search provider returned a relative issue path"
                )
            try:
                issue.path.resolve().relative_to(normalized_query.root)
            except ValueError as error:
                raise ApplicationValidationError(
                    "workspace search provider returned an issue outside the root"
                ) from error
        return result


def _never_cancelled() -> bool:
    """Provide the default cancellation callback without a truthiness shortcut."""
    return False
