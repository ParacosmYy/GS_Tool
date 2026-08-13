"""Qt-free admission for the non-modal workspace search surface."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

from .notification_contract import NotificationSink


class WorkspaceSearchSurfacePort(Protocol):
    """Minimal surface contract needed to scope and show workspace search."""

    @property
    def root(self) -> Path:
        """Return the workspace root currently projected by the surface."""

    def set_root(self, root: Path) -> None:
        """Switch the explicit search scope."""

    def show(self) -> None:
        """Present the surface."""


@dataclass(frozen=True, slots=True)
class WorkspaceSearchSurfaceAdmissionPorts:
    """Typed callbacks required to admit one workspace search surface flow."""

    startup_restore_inflight: Callable[[], bool]
    search_available: Callable[[], bool]
    workspace_root: Callable[[], Path | None]
    get_surface: Callable[[], WorkspaceSearchSurfacePort | None]
    set_surface: Callable[[WorkspaceSearchSurfacePort], None]
    create_surface: Callable[[Path], WorkspaceSearchSurfacePort]
    invalidate_search: Callable[[], None]
    notify: NotificationSink


class WorkspaceSearchSurfaceAdmissionCoordinator:
    """Admit and scope the search surface without owning search operations."""

    def __init__(self, ports: WorkspaceSearchSurfaceAdmissionPorts) -> None:
        self._ports = ports

    def admit(self) -> bool:
        """Create or rescope the search surface and present it when available."""
        if self._ports.startup_restore_inflight():
            self._ports.notify("Restoring the previous session...", level="warning")
            return False
        if not self._ports.search_available():
            self._ports.notify("Workspace search is unavailable", level="error")
            return False
        root = self._ports.workspace_root()
        if root is None:
            self._ports.notify("Open a workspace before searching files", level="warning")
            return False
        surface = self._ports.get_surface()
        if surface is None:
            self._ports.set_surface(self._ports.create_surface(root))
        elif surface.root != root:
            self._ports.invalidate_search()
            surface.set_root(root)
        surface = self._ports.get_surface()
        if surface is None:
            return False
        surface.show()
        return True


__all__ = [
    "WorkspaceSearchSurfaceAdmissionCoordinator",
    "WorkspaceSearchSurfaceAdmissionPorts",
    "WorkspaceSearchSurfacePort",
]
