from __future__ import annotations

from embeddebug.serial_station.ui.serial_config_options import (
    apply_serial_config_options,
    serial_config_options,
)


def test_serial_config_options_describe_baud_choices_and_default():
    options = serial_config_options("baud")

    assert options.values == ("9600", "19200", "38400", "57600", "115200", "921600")
    assert options.default == "115200"


def test_serial_config_options_describe_frame_choices_and_defaults():
    assert serial_config_options("data_bits").values == ("5", "6", "7", "8")
    assert serial_config_options("data_bits").default == "8"
    assert serial_config_options("parity").values == ("None", "Even", "Odd", "Space", "Mark")
    assert serial_config_options("parity").default == "None"
    assert serial_config_options("stop_bits").values == ("1", "1.5", "2")
    assert serial_config_options("stop_bits").default == "1"
    assert serial_config_options("flow_control").values == ("None", "Hardware", "Software")
    assert serial_config_options("flow_control").default == "None"


class ComboBox:
    def __init__(self) -> None:
        self.items: list[str] = []
        self.current = ""

    def addItems(self, values: list[str]) -> None:
        self.items.extend(values)

    def setCurrentText(self, text: str) -> None:
        self.current = text


def test_apply_serial_config_options_writes_values_and_default_to_combo():
    combo = ComboBox()

    apply_serial_config_options(combo, "baud")

    assert combo.items == ["9600", "19200", "38400", "57600", "115200", "921600"]
    assert combo.current == "115200"
