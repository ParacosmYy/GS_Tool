"""Framework-neutral lifecycle state for one asynchronous settings save."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal

from ..domain.models import SettingsSnapshot

SettingsSaveCompletion = Literal["stale", "invalid", "valid"]


@dataclass(slots=True)
class SettingsSaveTracker:
    """Own one settings-save callback identity without owning settings policy."""

    _operation_id: int | None = None

    @property
    def inflight(self) -> bool:
        """Return whether a settings save callback is still outstanding."""
        return self._operation_id is not None

    @property
    def operation_id(self) -> int | None:
        """Return the operation ID bound to the current settings save."""
        return self._operation_id

    def begin(self, operation_id: int) -> bool:
        """Bind one positive operation ID when no save is currently active."""
        _require_operation_id(operation_id)
        if self._operation_id is not None:
            return False
        self._operation_id = operation_id
        return True

    def complete(
        self,
        operation_id: int,
        result: object,
    ) -> SettingsSaveCompletion:
        """Classify one callback without clearing a newer save identity."""
        _require_operation_id(operation_id)
        if self._operation_id != operation_id:
            return "stale"
        self._operation_id = None
        if not isinstance(result, SettingsSnapshot):
            return "invalid"
        return "valid"

    def fail(self, operation_id: int) -> bool:
        """Consume a matching failed callback and preserve stale-callback safety."""
        _require_operation_id(operation_id)
        if self._operation_id != operation_id:
            return False
        self._operation_id = None
        return True


def _require_operation_id(operation_id: int) -> None:
    if type(operation_id) is not int or operation_id < 1:
        raise ValueError("settings save operation ID must be a positive integer")
