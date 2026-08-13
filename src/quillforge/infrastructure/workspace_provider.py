"""Bounded local filesystem adapter for workspace navigation."""

from pathlib import Path

from ..application.ports import WorkspaceProvider
from ..domain.models import WorkspaceDirectory, WorkspaceEntry


class FileWorkspaceProvider(WorkspaceProvider):
    """Enumerate one directory page without recursion or symlink traversal."""

    def is_directory(self, path: Path) -> bool:
        """Keep filesystem directory classification inside the adapter."""
        return path.is_dir()

    def list_directory(self, path: Path, *, limit: int) -> WorkspaceDirectory:
        """Return at most ``limit`` entries plus a truncation signal."""
        entries: list[WorkspaceEntry] = []
        iterator = path.iterdir()
        truncated = False
        for index, entry in enumerate(iterator):
            if index >= limit:
                truncated = True
                break
            entries.append(_describe_entry(entry))
        entries.sort(key=lambda item: (item.kind != "directory", item.name.casefold()))
        return WorkspaceDirectory(path=path, entries=tuple(entries), truncated=truncated)


def _describe_entry(path: Path) -> WorkspaceEntry:
    try:
        if path.is_symlink():
            return WorkspaceEntry(
                path=path,
                name=path.name,
                kind="inaccessible",
                error="Symbolic links are not traversed",
            )
        if path.is_dir():
            return WorkspaceEntry(path=path, name=path.name, kind="directory")
        if path.is_file():
            return WorkspaceEntry(path=path, name=path.name, kind="file")
        return WorkspaceEntry(
            path=path,
            name=path.name,
            kind="inaccessible",
            error="Entry type is not supported",
        )
    except OSError as error:
        return WorkspaceEntry(
            path=path,
            name=path.name,
            kind="inaccessible",
            error=str(error),
        )
