"""Qt-free orchestration for opening a file selected in the workspace dock."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from .notification_contract import NotificationSink


@dataclass(frozen=True, slots=True)
class WorkspaceFileActivationPorts[TabT]:
    """Typed shell callbacks required to activate one workspace file."""

    is_startup_restore_inflight: Callable[[], bool]
    is_busy: Callable[[], bool]
    contains: Callable[[Path], bool]
    find_tab: Callable[[Path], TabT | None]
    set_current: Callable[[TabT], None]
    start_open: Callable[[Path], None]
    notify: NotificationSink


class WorkspaceFileActivationCoordinator[TabT]:
    """Apply workspace-file admission and route one accepted path to opening."""

    def __init__(self, ports: WorkspaceFileActivationPorts[TabT]) -> None:
        self._ports = ports

    def activate(self, path: object) -> None:
        """Handle one file intent without owning Qt, services, or tab state."""
        if not isinstance(path, Path):
            return
        if self._ports.is_startup_restore_inflight():
            self._ports.notify("Restoring the previous session...", level="warning")
            return
        if self._ports.is_busy():
            return
        if not self._ports.contains(path):
            self._ports.notify("That file is outside the selected workspace", level="warning")
            return
        existing = self._ports.find_tab(path)
        if existing is not None:
            self._ports.set_current(existing)
            self._ports.notify(f"Already open: {path.name}", level="info")
            return
        self._ports.start_open(path)


__all__ = ["WorkspaceFileActivationCoordinator", "WorkspaceFileActivationPorts"]
