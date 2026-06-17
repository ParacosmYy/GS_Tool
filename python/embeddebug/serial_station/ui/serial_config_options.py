"""Serial configuration option helpers for Serial Station widgets."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal, Protocol


SerialConfigField = Literal["baud", "data_bits", "parity", "stop_bits", "flow_control"]


@dataclass(frozen=True)
class SerialConfigOptions:
    """Selectable values and default text for one serial configuration field."""

    values: tuple[str, ...]
    default: str


class SerialConfigCombo(Protocol):
    def addItems(self, values: list[str]) -> None: ...
    def setCurrentText(self, text: str) -> None: ...


_OPTIONS_BY_FIELD: dict[SerialConfigField, SerialConfigOptions] = {
    "baud": SerialConfigOptions(
        values=("9600", "19200", "38400", "57600", "115200", "921600"),
        default="115200",
    ),
    "data_bits": SerialConfigOptions(values=("5", "6", "7", "8"), default="8"),
    "parity": SerialConfigOptions(
        values=("None", "Even", "Odd", "Space", "Mark"),
        default="None",
    ),
    "stop_bits": SerialConfigOptions(values=("1", "1.5", "2"), default="1"),
    "flow_control": SerialConfigOptions(
        values=("None", "Hardware", "Software"),
        default="None",
    ),
}


def serial_config_options(field: SerialConfigField) -> SerialConfigOptions:
    return _OPTIONS_BY_FIELD[field]


def apply_serial_config_options(combo: SerialConfigCombo, field: SerialConfigField) -> None:
    options = serial_config_options(field)
    combo.addItems(list(options.values))
    combo.setCurrentText(options.default)
