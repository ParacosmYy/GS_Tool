from __future__ import annotations

from embeddebug.serial_station.ui.serial_connection_fields import (
    read_serial_connection_fields,
)


class Combo:
    def __init__(self, value: str) -> None:
        self.value = value

    def currentText(self) -> str:
        return self.value


class Host:
    def __init__(self) -> None:
        self._port_combo = Combo("COM7")
        self._baud_combo = Combo("115200")
        self._data_bits_combo = Combo("8")
        self._parity_combo = Combo("None")
        self._stop_bits_combo = Combo("1")
        self._flow_control_combo = Combo("Hardware")


def test_read_serial_connection_fields_converts_combo_text_values():
    fields = read_serial_connection_fields(Host())

    assert fields.port_name == "COM7"
    assert fields.baud_rate == 115200
    assert fields.data_bits == 8
    assert fields.parity == "none"
    assert fields.stop_bits == "1"
    assert fields.flow_control == "hardware"
