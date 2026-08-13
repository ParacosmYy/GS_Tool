"""Qt-free presentation orchestration for registered plugin runtime controls."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Protocol

from ..application.events import PluginFailed
from ..application.plugin_runtime import PluginRuntime, PluginRuntimeStatus
from .notification_contract import NotificationSink


class PluginRuntimeView(Protocol):
    """Presentation projection used by plugin runtime status orchestration."""

    def show_status(self, statuses: tuple[PluginRuntimeStatus, ...]) -> None:
        """Project immutable registered-plugin runtime statuses."""


@dataclass(frozen=True, slots=True)
class PluginRuntimePorts:
    """Typed presentation callbacks required by plugin runtime controls."""

    view: PluginRuntimeView
    is_busy: Callable[[], bool]
    refresh_commands: Callable[[], None]
    notify: NotificationSink


class PluginRuntimeCoordinator:
    """Own runtime status/control sequencing without owning runtime policy."""

    def __init__(
        self,
        runtime: PluginRuntime | None,
        ports: PluginRuntimePorts,
    ) -> None:
        self._runtime = runtime
        self._ports = ports

    def on_plugin_failed(self, event: PluginFailed) -> None:
        """Project an isolated plugin failure and refresh command projections."""
        self._ports.notify(
            f"Plugin {event.plugin_id} failed during {event.phase}: {event.error}",
            level="error",
        )
        self._ports.refresh_commands()

    def show_status(self) -> None:
        """Project registered plugin lifecycle state without manager internals."""
        runtime = self._runtime
        if runtime is None:
            self._ports.notify("Plugin runtime control is unavailable", level="error")
            return
        self._ports.view.show_status(runtime.runtime_statuses())

    def enable_plugin(self, plugin_id: str) -> None:
        """Request enablement for one registered plugin."""
        self._set_enabled(plugin_id, True)

    def disable_plugin(self, plugin_id: str) -> None:
        """Request disablement for one registered plugin."""
        self._set_enabled(plugin_id, False)

    def _set_enabled(self, plugin_id: str, enabled: bool) -> None:
        """Apply one UI-thread lifecycle toggle and refresh projections."""
        runtime = self._runtime
        if runtime is None:
            self._ports.notify("Plugin runtime control is unavailable", level="error")
            return
        if self._ports.is_busy():
            self._ports.notify(
                "Wait for the current document operation to finish",
                level="warning",
            )
            return
        try:
            runtime.set_enabled(plugin_id, enabled)
        except (KeyError, PermissionError, ValueError, RuntimeError) as error:
            self._ports.notify(f"Plugin lifecycle change failed: {error}", level="error")
            return
        self._ports.refresh_commands()
        state = "enabled" if enabled else "disabled"
        self._ports.notify(f"Plugin {plugin_id} {state}", level="success")
        self.show_status()


__all__ = ["PluginRuntimeCoordinator", "PluginRuntimePorts", "PluginRuntimeView"]
