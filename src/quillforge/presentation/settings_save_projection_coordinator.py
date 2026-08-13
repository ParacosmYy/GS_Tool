"""Qt-free presentation orchestration for valid settings saves."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..domain.models import SettingsSnapshot


@dataclass(frozen=True, slots=True)
class SettingsSaveProjectionPorts:
    """Typed callbacks required to project one valid settings result."""

    apply_snapshot: Callable[[SettingsSnapshot], None]
    retranslate: Callable[[], None]
    apply_editor_settings: Callable[[], None]
    animate_transition: Callable[[], None]
    notify_saved: Callable[[], None]


class SettingsSaveProjectionCoordinator:
    """Project one validated settings result in the existing observable order."""

    def __init__(self, ports: SettingsSaveProjectionPorts) -> None:
        self._ports = ports

    def project(self, result: SettingsSnapshot) -> None:
        """Project a result already validated by ``SettingsSaveCoordinator``."""
        self._ports.apply_snapshot(result)
        self._ports.retranslate()
        self._ports.apply_editor_settings()
        self._ports.animate_transition()
        self._ports.notify_saved()


__all__ = [
    "SettingsSaveProjectionCoordinator",
    "SettingsSaveProjectionPorts",
]
