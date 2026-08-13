"""Application-owned runtime status and lifecycle control contracts."""

from dataclasses import dataclass
from typing import Protocol


@dataclass(frozen=True, slots=True)
class PluginRuntimeStatus:
    """Immutable diagnostic projection for one registered plugin."""

    plugin_id: str
    name: str
    version: str
    trusted: bool
    enabled: bool
    active: bool
    permissions: tuple[str, ...]
    error: str | None = None


class PluginRuntime(Protocol):
    """Lifecycle boundary exposed to presentation without manager internals."""

    def runtime_statuses(self) -> tuple[PluginRuntimeStatus, ...]:
        """Return immutable state snapshots for explicitly registered plugins."""

    def set_enabled(self, plugin_id: str, enabled: bool) -> None:
        """Enable or disable one registered in-process plugin on its owning thread."""
