"""Qt-free presentation orchestration for valid workspace navigation results."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from ..application.workspace import WorkspaceState
from ..domain.models import WorkspaceDirectory


@dataclass(frozen=True, slots=True)
class WorkspaceNavigationProjectionPorts:
    """Typed callbacks required to project one workspace navigation result."""

    invalidate_search: Callable[[], None]
    activate_workspace: Callable[[WorkspaceState], None]
    set_search_root: Callable[[Path], None]
    has_workspace_surface: Callable[[], bool]
    set_directory: Callable[[WorkspaceDirectory, Path], None]
    notify_opened: Callable[[Path], None]
    request_session_save: Callable[[], None]
    finish_session_restore: Callable[[], None]
    get_workspace_root: Callable[[], Path | None]


class WorkspaceNavigationProjectionCoordinator:
    """Project validated workspace results while preserving shell ordering."""

    def __init__(self, ports: WorkspaceNavigationProjectionPorts) -> None:
        self._ports = ports

    def project_opened(self, result: WorkspaceState) -> None:
        """Project a validated workspace-open result in the former order."""
        self._ports.invalidate_search()
        self._ports.activate_workspace(result)
        self._ports.set_search_root(result.root)
        if not self._ports.has_workspace_surface():
            return
        self._ports.set_directory(result.directory, result.root)
        self._ports.notify_opened(result.root)
        self._ports.request_session_save()
        self._ports.finish_session_restore()

    def project_directory(self, result: WorkspaceDirectory) -> None:
        """Project a validated directory page against the current workspace."""
        root = self._ports.get_workspace_root()
        if root is None or not self._ports.has_workspace_surface():
            return
        self._ports.set_directory(result, root)


__all__ = [
    "WorkspaceNavigationProjectionCoordinator",
    "WorkspaceNavigationProjectionPorts",
]
