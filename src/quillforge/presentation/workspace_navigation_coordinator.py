"""Qt-free presentation orchestration for workspace navigation completions."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

from ..application.workspace import WorkspaceState
from ..domain.models import WorkspaceDirectory
from .notification_contract import NotificationSink
from .workspace_operation_tracker import WorkspaceOperationTracker

WorkspaceNavigationOperation = Callable[[], object]
WorkspaceNavigationSuccess = Callable[[object, int], None]
WorkspaceNavigationFailure = Callable[[Exception, int], None]
WorkspaceNavigationDispatcher = Callable[
    [WorkspaceNavigationOperation, int, WorkspaceNavigationSuccess, WorkspaceNavigationFailure],
    None,
]


class WorkspaceNavigationView(Protocol):
    """Minimal workspace surface needed by navigation completion callbacks."""

    def set_loading(self, loading: bool) -> None:
        """Project worker lifecycle state."""

    def set_directory(self, directory: WorkspaceDirectory, root: Path) -> None:
        """Project a validated directory page."""

    def show_error(self, message: str | Exception) -> None:
        """Project a recoverable navigation error."""


@dataclass(frozen=True, slots=True)
class WorkspaceNavigationPorts:
    """Typed callbacks required by workspace navigation completion policy."""

    tracker: WorkspaceOperationTracker
    complete_operation: Callable[[int], bool]
    get_surface: Callable[[], WorkspaceNavigationView | None]
    apply_opened: Callable[[WorkspaceState], None]
    apply_directory: Callable[[WorkspaceDirectory], None]
    finish_session_restore: Callable[[], None]
    notify: NotificationSink


class WorkspaceNavigationCoordinator:
    """Classify navigation callbacks without owning workspace policy."""

    def __init__(
        self,
        ports: WorkspaceNavigationPorts,
    ) -> None:
        self._ports = ports

    def submit_open(
        self,
        *,
        operation: WorkspaceNavigationOperation,
        operation_id: int,
        generation: int,
        dispatch: WorkspaceNavigationDispatcher,
    ) -> None:
        """Bind workspace-open generation before invoking a worker dispatcher."""

        def on_opened(result: object, current_operation_id: int) -> None:
            self.complete_open(result, current_operation_id, generation)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(error, current_operation_id, generation)

        dispatch(operation, operation_id, on_opened, on_failed)

    def submit_directory(
        self,
        *,
        operation: WorkspaceNavigationOperation,
        operation_id: int,
        generation: int,
        dispatch: WorkspaceNavigationDispatcher,
    ) -> None:
        """Bind directory generation before invoking a worker dispatcher."""

        def on_directory(result: object, current_operation_id: int) -> None:
            self.complete_directory(result, current_operation_id, generation)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(error, current_operation_id, generation)

        dispatch(operation, operation_id, on_directory, on_failed)

    def complete_open(self, result: object, operation_id: int, generation: int) -> None:
        """Classify an open-workspace callback and delegate valid policy."""
        if not self._ports.complete_operation(operation_id):
            return
        completion = self._ports.tracker.finish(operation_id, generation)
        if completion == "stale":
            return
        surface = self._ports.get_surface()
        if surface is not None:
            surface.set_loading(False)
        if completion == "invalidated":
            self._ports.finish_session_restore()
            return
        if not isinstance(result, WorkspaceState) or surface is None:
            if surface is not None:
                surface.show_error("The workspace service returned an invalid result")
            self._ports.notify(
                "Workspace operation failed: invalid workspace result",
                level="error",
            )
            self._ports.finish_session_restore()
            return
        self._ports.apply_opened(result)

    def complete_directory(self, result: object, operation_id: int, generation: int) -> None:
        """Classify a directory callback and delegate valid page projection."""
        if not self._ports.complete_operation(operation_id):
            return
        completion = self._ports.tracker.finish(operation_id, generation)
        if completion == "stale":
            return
        surface = self._ports.get_surface()
        if surface is not None:
            surface.set_loading(False)
        if completion == "invalidated":
            return
        if surface is None or not isinstance(result, WorkspaceDirectory):
            if surface is not None:
                surface.show_error("The workspace service returned an invalid directory")
            self._ports.notify(
                "Workspace operation failed: invalid directory result",
                level="error",
            )
            return
        self._ports.apply_directory(result)

    def fail(self, error: Exception, operation_id: int, generation: int) -> None:
        """Project one current or invalidated worker failure."""
        if not self._ports.complete_operation(operation_id):
            return
        completion = self._ports.tracker.finish(operation_id, generation)
        if completion == "stale":
            return
        surface = self._ports.get_surface()
        if surface is not None:
            surface.set_loading(False)
            surface.show_error(error)
        if completion == "current":
            self._ports.notify(f"Workspace operation failed: {error}", level="error")
        self._ports.finish_session_restore()


__all__ = ["WorkspaceNavigationCoordinator", "WorkspaceNavigationPorts", "WorkspaceNavigationView"]
