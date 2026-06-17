"""Serial transport profile helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.profile_combo_options import (
    ProfileCombo,
    select_profile_combo_value,
)


class SerialProfileControlsHost(Protocol):
    """Minimal combo surface needed by serial profile helpers."""

    _baud_combo: ProfileCombo
    _data_bits_combo: ProfileCombo
    _parity_combo: ProfileCombo
    _stop_bits_combo: ProfileCombo
    _flow_control_combo: ProfileCombo


def apply_serial_profile_controls(
    host: SerialProfileControlsHost,
    transport: dict[str, object],
) -> None:
    baud_rate = transport.get("baudRate")
    if baud_rate is not None:
        select_profile_combo_value(host._baud_combo, str(baud_rate))
    data_bits = transport.get("dataBits")
    if data_bits is not None:
        select_profile_combo_value(host._data_bits_combo, str(data_bits))
    parity = str(transport.get("parity", ""))
    if parity:
        select_profile_combo_value(host._parity_combo, parity.capitalize())
    stop_bits = transport.get("stopBits")
    if stop_bits is not None:
        select_profile_combo_value(host._stop_bits_combo, str(stop_bits))
    flow_control = str(transport.get("flowControl", ""))
    if flow_control:
        select_profile_combo_value(host._flow_control_combo, flow_control.capitalize())
