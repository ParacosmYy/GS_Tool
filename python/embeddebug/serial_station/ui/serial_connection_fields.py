"""Serial connection field readers for Serial Station widgets."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Protocol


class SerialConnectionCombo(Protocol):
    """Minimal combo surface needed by serial connection field readers."""

    def currentText(self) -> str: ...


class SerialConnectionFieldsHost(Protocol):
    """Minimal host surface needed to read serial connection fields."""

    _port_combo: SerialConnectionCombo
    _baud_combo: SerialConnectionCombo
    _data_bits_combo: SerialConnectionCombo
    _parity_combo: SerialConnectionCombo
    _stop_bits_combo: SerialConnectionCombo
    _flow_control_combo: SerialConnectionCombo


@dataclass(frozen=True)
class SerialConnectionFields:
    """Normalized serial connection values read from UI controls."""

    port_name: str
    baud_rate: int
    data_bits: int
    parity: str
    stop_bits: str
    flow_control: str


def read_serial_connection_fields(host: SerialConnectionFieldsHost) -> SerialConnectionFields:
    return SerialConnectionFields(
        port_name=host._port_combo.currentText(),
        baud_rate=int(host._baud_combo.currentText()),
        data_bits=int(host._data_bits_combo.currentText()),
        parity=host._parity_combo.currentText().lower(),
        stop_bits=host._stop_bits_combo.currentText(),
        flow_control=host._flow_control_combo.currentText().lower(),
    )
