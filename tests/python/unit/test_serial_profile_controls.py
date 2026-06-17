from __future__ import annotations

from embeddebug.serial_station.ui.serial_profile_controls import (
    apply_serial_profile_controls,
)


class Combo:
    def __init__(self) -> None:
        self.items: list[str] = []
        self.current = ""

    def findText(self, text: str) -> int:
        try:
            return self.items.index(text)
        except ValueError:
            return -1

    def addItem(self, text: str) -> None:
        self.items.append(text)

    def setCurrentText(self, text: str) -> None:
        self.current = text


class Host:
    def __init__(self) -> None:
        self._baud_combo = Combo()
        self._data_bits_combo = Combo()
        self._parity_combo = Combo()
        self._stop_bits_combo = Combo()
        self._flow_control_combo = Combo()


def test_apply_serial_profile_controls_writes_serial_transport_values():
    host = Host()

    apply_serial_profile_controls(
        host,
        {
            "baudRate": 115200,
            "dataBits": 8,
            "parity": "none",
            "stopBits": 1,
            "flowControl": "none",
        },
    )

    assert host._baud_combo.current == "115200"
    assert host._data_bits_combo.current == "8"
    assert host._parity_combo.current == "None"
    assert host._stop_bits_combo.current == "1"
    assert host._flow_control_combo.current == "None"


def test_apply_serial_profile_controls_ignores_missing_values():
    host = Host()

    apply_serial_profile_controls(host, {})

    assert host._baud_combo.current == ""
    assert host._data_bits_combo.current == ""
    assert host._parity_combo.current == ""
    assert host._stop_bits_combo.current == ""
    assert host._flow_control_combo.current == ""
