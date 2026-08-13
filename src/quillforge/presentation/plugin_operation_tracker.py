"""Framework-neutral lifecycle state for independent plugin operations."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Literal

PluginOperationKind = Literal[
    "catalog-scan",
    "catalog-governance",
    "host-probe",
]

_OPERATION_KINDS: tuple[PluginOperationKind, ...] = (
    "catalog-scan",
    "catalog-governance",
    "host-probe",
)


@dataclass(slots=True)
class _OperationSlot:
    next_id: int = 0
    active_id: int | None = None


def _new_operation_slots() -> dict[PluginOperationKind, _OperationSlot]:
    return {kind: _OperationSlot() for kind in _OPERATION_KINDS}


@dataclass(slots=True)
class PluginOperationTracker:
    """Track independent plugin callback lifecycles without owning policy."""

    _slots: dict[PluginOperationKind, _OperationSlot] = field(default_factory=_new_operation_slots)

    def begin(self, kind: PluginOperationKind) -> int:
        """Allocate and mark the next operation for one plugin domain."""
        slot = self._slots[kind]
        slot.next_id += 1
        slot.active_id = slot.next_id
        return slot.active_id

    def in_flight(self, kind: PluginOperationKind) -> bool:
        """Return whether one callback boundary is active for ``kind``."""
        return self._slots[kind].active_id is not None

    def complete(self, kind: PluginOperationKind, operation_id: int) -> bool:
        """Clear only the current ID for ``kind``; reject stale delivery."""
        slot = self._slots[kind]
        if slot.active_id != operation_id:
            return False
        slot.active_id = None
        return True
