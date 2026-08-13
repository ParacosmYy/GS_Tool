"""Qt-free presentation orchestration for the isolated plugin-host probe."""

from __future__ import annotations

from dataclasses import dataclass

from ..application.plugin_host import PluginHostClient, PluginHostProbeResult
from .notification_contract import NotificationSink
from .plugin_operation_tracker import PluginOperationTracker
from .task_contract import TaskSubmitter


@dataclass(frozen=True, slots=True)
class PluginHostProbePorts:
    """Typed dependencies required by isolated plugin-host diagnostics."""

    host: PluginHostClient | None
    operations: PluginOperationTracker
    runner: TaskSubmitter
    notify: NotificationSink


class PluginHostProbeCoordinator:
    """Own host-probe sequencing without owning host security or window policy."""

    def __init__(self, ports: PluginHostProbePorts) -> None:
        self._ports = ports

    def probe(self) -> None:
        """Probe the separate diagnostic host through the worker boundary."""
        if self._ports.host is None:
            self._ports.notify("Plugin host diagnostics are unavailable", level="error")
            return
        if self._ports.operations.in_flight("host-probe"):
            self._ports.notify("Plugin host diagnostic already in progress", level="warning")
            return
        operation_id = self._ports.operations.begin("host-probe")
        self._ports.notify("Starting isolated plugin host diagnostic...", level="info")
        self._ports.runner.submit(
            self._ports.host.probe,
            operation_id,
            self._complete_probe,
            self._fail_probe,
        )

    def _complete_probe(self, result: object, operation_id: int) -> None:
        if not self._ports.operations.complete("host-probe", operation_id):
            return
        if not isinstance(result, PluginHostProbeResult):
            self._ports.notify("Plugin host returned an invalid diagnostic result", level="error")
            return
        self._ports.notify(
            result.summary(),
            level=(
                "success"
                if result.state == "ready"
                else "warning"
                if result.state == "rejected"
                else "error"
            ),
        )

    def _fail_probe(self, error: Exception, operation_id: int) -> None:
        if not self._ports.operations.complete("host-probe", operation_id):
            return
        self._ports.notify(f"Plugin host diagnostic failed: {error}", level="error")


__all__ = ["PluginHostProbeCoordinator", "PluginHostProbePorts"]
