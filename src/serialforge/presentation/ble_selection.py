"""Pure BLE write-mode selection helpers for presentation owners."""

from __future__ import annotations

from ..domain.models import BleGattCharacteristic, BleGattWriteMode
from .connection_bindings import ble_bindings_for
from .qt import Qt


def selected_ble_write_mode(window) -> BleGattWriteMode | None:
    """Read the bounded write-mode combo without exposing a window facade."""

    ble = ble_bindings_for(window)
    if ble is None:
        return None
    value = ble.write_mode.currentData(Qt.ItemDataRole.UserRole)
    if isinstance(value, BleGattWriteMode):
        return value
    try:
        return BleGattWriteMode(value)
    except (TypeError, ValueError):
        return None


def supported_ble_write_modes(
    characteristic: BleGattCharacteristic | None,
) -> tuple[BleGattWriteMode, ...]:
    """Return the write modes explicitly advertised by one characteristic."""

    if characteristic is None:
        return ()
    modes: list[BleGattWriteMode] = []
    if characteristic.supports("write"):
        modes.append(BleGattWriteMode.WITH_RESPONSE)
    if characteristic.supports("write-without-response"):
        modes.append(BleGattWriteMode.WITHOUT_RESPONSE)
    return tuple(modes)
