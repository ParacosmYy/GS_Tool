"""Qt-free admission and dispatch orchestration for settings saves."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Protocol

from ..domain.models import SettingsSnapshot
from .notification_contract import NotificationSink
from .settings_save_coordinator import (
    SettingsSaveDispatcher,
    SettingsSaveOperation,
)


class SettingsPersistenceService(Protocol):
    """Minimal settings persistence contract required by save admission."""

    def save(self, settings: SettingsSnapshot) -> object:
        """Persist one validated settings candidate."""


SettingsEditor = Callable[[SettingsSnapshot], SettingsSnapshot | None]
SettingsSaveSubmission = Callable[
    [SettingsSaveOperation, int, SettingsSaveDispatcher],
    None,
]


@dataclass(frozen=True, slots=True)
class SettingsSaveAdmissionPorts:
    """Typed callbacks required to admit and dispatch one settings save."""

    get_service: Callable[[], SettingsPersistenceService | None]
    is_inflight: Callable[[], bool]
    edit: SettingsEditor
    reserve_operation: Callable[[], int]
    begin_save: Callable[[int], bool]
    submit_save: SettingsSaveSubmission
    dispatch: SettingsSaveDispatcher
    notify: NotificationSink


class SettingsSaveAdmissionCoordinator:
    """Admit one settings candidate while preserving the existing result boundary."""

    def __init__(self, ports: SettingsSaveAdmissionPorts) -> None:
        self._ports = ports

    def admit(self, current: SettingsSnapshot) -> bool:
        """Edit and start one accepted settings save request."""
        service = self._ports.get_service()
        if service is None:
            self._ports.notify("Settings persistence is unavailable", level="error")
            return False
        if self._ports.is_inflight():
            self._ports.notify("Settings save already in progress", level="warning")
            return False

        candidate = self._ports.edit(current)
        if candidate is None:
            return False
        operation_id = self._ports.reserve_operation()
        if not self._ports.begin_save(operation_id):
            return False
        self._ports.submit_save(
            lambda: service.save(candidate),
            operation_id,
            self._ports.dispatch,
        )
        return True


__all__ = [
    "SettingsPersistenceService",
    "SettingsSaveAdmissionCoordinator",
    "SettingsSaveAdmissionPorts",
]
