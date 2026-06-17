from __future__ import annotations

from embeddebug.serial_station.ui.serial_port_options import (
    combo_has_serial_ports,
    populate_serial_port_options,
)


class Combo:
    def __init__(self) -> None:
        self.enabled = True
        self.items: list[str] = []
        self.current = ""

    def addItem(self, text: str) -> None:
        self.items.append(text)
        if not self.current:
            self.current = text

    def addItems(self, items: list[str]) -> None:
        for item in items:
            self.addItem(item)

    def clear(self) -> None:
        self.items.clear()
        self.current = ""

    def count(self) -> int:
        return len(self.items)

    def currentText(self) -> str:
        return self.current

    def setCurrentText(self, text: str) -> None:
        self.current = text


class Host:
    def tr(self, text: str) -> str:
        return f"tr:{text}"


def test_populate_serial_port_options_writes_translated_empty_option():
    combo = Combo()

    populate_serial_port_options(Host(), combo, ports=[], current="")

    assert combo.items == ["tr:No serial ports"]
    assert not combo_has_serial_ports(Host(), combo)


def test_populate_serial_port_options_keeps_current_port_when_available():
    combo = Combo()

    populate_serial_port_options(Host(), combo, ports=["COM1", "COM2"], current="COM2")

    assert combo.items == ["COM1", "COM2"]
    assert combo.currentText() == "COM2"
    assert combo_has_serial_ports(Host(), combo)
