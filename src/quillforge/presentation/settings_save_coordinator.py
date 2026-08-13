"""Qt-free presentation orchestration for settings-save completions."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..domain.models import SettingsSnapshot
from .settings_save_tracker import SettingsSaveTracker

SettingsSaveOperation = Callable[[], object]
SettingsSaveSuccess = Callable[[object, int], None]
SettingsSaveFailure = Callable[[Exception, int], None]
SettingsSaveDispatcher = Callable[
    [SettingsSaveOperation, int, SettingsSaveSuccess, SettingsSaveFailure], None
]


@dataclass(frozen=True, slots=True)
class SettingsSavePorts:
    """Typed callbacks required to project one settings-save result."""

    apply_settings: Callable[[SettingsSnapshot], None]
    show_invalid_result: Callable[[], None]
    show_failure: Callable[[Exception], None]


class SettingsSaveCoordinator:
    """Classify one settings callback without owning settings application policy."""

    def __init__(
        self,
        tracker: SettingsSaveTracker,
        ports: SettingsSavePorts,
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def submit(
        self,
        *,
        operation: SettingsSaveOperation,
        operation_id: int,
        dispatch: SettingsSaveDispatcher,
    ) -> None:
        """Bind settings callbacks before invoking a generic worker dispatcher."""

        def on_saved(result: object, current_operation_id: int) -> None:
            self.complete(result, current_operation_id)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(error, current_operation_id)

        dispatch(operation, operation_id, on_saved, on_failed)

    def complete(self, result: object, operation_id: int) -> None:
        """Classify a save callback and apply only the matching valid result."""
        completion = self._tracker.complete(operation_id, result)
        if completion == "stale":
            return
        if completion == "invalid" or not isinstance(result, SettingsSnapshot):
            self._ports.show_invalid_result()
            return
        self._ports.apply_settings(result)

    def fail(self, error: Exception, operation_id: int) -> None:
        """Project a matching worker failure while ignoring stale failures."""
        if not self._tracker.fail(operation_id):
            return
        self._ports.show_failure(error)


__all__ = ["SettingsSaveCoordinator", "SettingsSavePorts"]
