"""Bounded workspace navigation use cases."""

from dataclasses import dataclass
from pathlib import Path

from ..domain.models import WorkspaceDirectory
from .errors import ApplicationStateError, ApplicationValidationError
from .ports import WorkspaceProvider


@dataclass(frozen=True, slots=True)
class WorkspaceState:
    """The currently selected root and its most recently loaded directory page."""

    root: Path
    directory: WorkspaceDirectory


class WorkspaceService:
    """Owns root containment and enumeration policy, not filesystem mechanics."""

    def __init__(self, provider: WorkspaceProvider, *, max_entries: int = 500) -> None:
        if max_entries < 1:
            raise ApplicationValidationError("Workspace entry limit must be positive")
        self._provider = provider
        self._max_entries = max_entries
        self._root: Path | None = None

    def open_workspace(self, path: Path) -> WorkspaceState:
        """Select an explicit directory and load only its first bounded page."""
        resolved = path.expanduser().resolve()
        if not self._provider.is_directory(resolved):
            raise ApplicationValidationError(f"Workspace folder does not exist: {resolved}")
        directory = self._provider.list_directory(resolved, limit=self._max_entries)
        return WorkspaceState(resolved, directory)

    def activate(self, state: WorkspaceState) -> None:
        """Commit a successfully loaded workspace on the owning UI thread."""
        self._root = state.root

    def list_directory(self, path: Path) -> WorkspaceDirectory:
        """Load one directory only when it remains inside the selected root."""
        if self._root is None:
            raise ApplicationStateError("Select a workspace folder first")
        resolved = path.expanduser().resolve()
        _ensure_within(resolved, self._root)
        if not self._provider.is_directory(resolved):
            raise ApplicationValidationError(f"Workspace directory does not exist: {resolved}")
        return self._provider.list_directory(resolved, limit=self._max_entries)

    def contains(self, path: Path) -> bool:
        """Return whether a path is inside the selected workspace root."""
        if self._root is None:
            return False
        try:
            _ensure_within(path.expanduser().resolve(), self._root)
        except ValueError:
            return False
        return True

    @property
    def root(self) -> Path | None:
        """Return the selected root for presentation navigation."""
        return self._root


def _ensure_within(path: Path, root: Path) -> None:
    try:
        path.relative_to(root)
    except ValueError as error:
        raise ApplicationValidationError(
            f"Path is outside the selected workspace: {path}"
        ) from error
