"""Qt-free admission for choosing a workspace directory."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from .notification_contract import NotificationSink


@dataclass(frozen=True, slots=True)
class WorkspacePickerAdmissionPorts:
    """Typed shell callbacks required to admit one workspace picker flow."""

    has_workspace: Callable[[], bool]
    startup_restore_inflight: Callable[[], bool]
    is_busy: Callable[[], bool]
    choose_workspace: Callable[[], Path | None]
    start_open: Callable[[Path], None]
    notify: NotificationSink


class WorkspacePickerAdmissionCoordinator:
    """Gate one workspace directory selection before navigation begins."""

    def __init__(self, ports: WorkspacePickerAdmissionPorts) -> None:
        self._ports = ports

    def admit(self) -> bool:
        """Choose and submit one workspace when the shell permits it."""
        if not self._ports.has_workspace():
            self._ports.notify("Workspace navigation is unavailable", level="error")
            return False
        if self._ports.startup_restore_inflight():
            self._ports.notify("Restoring the previous session...", level="warning")
            return False
        if self._ports.is_busy():
            return False
        selected = self._ports.choose_workspace()
        if selected is None:
            return False
        self._ports.start_open(selected)
        return True


__all__ = ["WorkspacePickerAdmissionCoordinator", "WorkspacePickerAdmissionPorts"]
