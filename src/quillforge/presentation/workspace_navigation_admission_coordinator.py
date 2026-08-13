"""Qt-free admission and dispatch orchestration for workspace navigation."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

from .notification_contract import NotificationSink
from .workspace_navigation_coordinator import (
    WorkspaceNavigationDispatcher,
    WorkspaceNavigationOperation,
)


class WorkspaceNavigationService(Protocol):
    """Minimal workspace service contract required to create navigation work."""

    def open_workspace(self, path: Path) -> object:
        """Create one workspace-open result."""

    def list_directory(self, path: Path) -> object:
        """Create one directory-list result."""


WorkspaceNavigationCall = Callable[[WorkspaceNavigationService, Path], object]
WorkspaceNavigationSubmission = Callable[
    [WorkspaceNavigationOperation, int, int, WorkspaceNavigationDispatcher],
    None,
]


@dataclass(frozen=True, slots=True)
class WorkspaceNavigationAdmissionPorts:
    """Typed callbacks required to admit one workspace navigation request."""

    get_workspace: Callable[[], WorkspaceNavigationService | None]
    has_surface: Callable[[], bool]
    is_busy: Callable[[], bool]
    startup_restore_inflight: Callable[[], bool]
    set_loading: Callable[[bool], None]
    begin_operation: Callable[[str], int]
    begin_generation: Callable[[int], int]
    submit_open: WorkspaceNavigationSubmission
    submit_directory: WorkspaceNavigationSubmission
    dispatch: WorkspaceNavigationDispatcher
    notify: NotificationSink


class WorkspaceNavigationAdmissionCoordinator:
    """Admit workspace navigation while preserving the existing result boundary."""

    def __init__(self, ports: WorkspaceNavigationAdmissionPorts) -> None:
        self._ports = ports

    def admit_open(self, path: Path, *, session_restore: bool = False) -> bool:
        """Admit one workspace-open request and bind its generation."""
        return self._admit(
            path,
            session_restore=session_restore,
            operation=self._open_workspace,
            submit=self._ports.submit_open,
            message=f"Loading workspace {path.name}...",
        )

    def admit_directory(self, path: object) -> bool:
        """Admit one directory-list request after preserving surface input guards."""
        if not isinstance(path, Path):
            return False
        return self._admit(
            path,
            session_restore=False,
            operation=self._list_directory,
            submit=self._ports.submit_directory,
            message=f"Loading {path.name}...",
        )

    def _admit(
        self,
        path: Path,
        *,
        session_restore: bool,
        operation: WorkspaceNavigationCall,
        submit: WorkspaceNavigationSubmission,
        message: str,
    ) -> bool:
        workspace = self._ports.get_workspace()
        restoring = self._ports.startup_restore_inflight()
        if (
            workspace is None
            or not self._ports.has_surface()
            or self._ports.is_busy()
            or (restoring and not session_restore)
        ):
            if restoring and not session_restore:
                self._ports.notify("Restoring the previous session...", level="warning")
            return False

        self._ports.set_loading(True)
        operation_id = self._ports.begin_operation(message)
        generation = self._ports.begin_generation(operation_id)
        submit(
            lambda: operation(workspace, path),
            operation_id,
            generation,
            self._ports.dispatch,
        )
        return True

    @staticmethod
    def _open_workspace(workspace: WorkspaceNavigationService, path: Path) -> object:
        return workspace.open_workspace(path)

    @staticmethod
    def _list_directory(workspace: WorkspaceNavigationService, path: Path) -> object:
        return workspace.list_directory(path)


__all__ = [
    "WorkspaceNavigationAdmissionCoordinator",
    "WorkspaceNavigationAdmissionPorts",
    "WorkspaceNavigationService",
]
