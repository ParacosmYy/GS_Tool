"""Serial port combo option helpers for Serial Station UI actions."""

from __future__ import annotations

from typing import Protocol


EMPTY_SERIAL_PORT_TEXT = "No serial ports"


class SerialPortOptionsHost(Protocol):
    """Minimal host surface needed for translated serial port options."""

    def tr(self, text: str) -> str: ...


class SerialPortCombo(Protocol):
    """Minimal combo surface needed by serial port option helpers."""

    def addItem(self, text: str) -> None: ...

    def addItems(self, items: list[str]) -> None: ...

    def clear(self) -> None: ...

    def count(self) -> int: ...

    def currentText(self) -> str: ...

    def setCurrentText(self, text: str) -> None: ...


def empty_serial_port_text(host: SerialPortOptionsHost) -> str:
    """Return the translated empty serial port placeholder."""

    return host.tr(EMPTY_SERIAL_PORT_TEXT)


def combo_has_serial_ports(host: SerialPortOptionsHost, combo: SerialPortCombo) -> bool:
    """Return whether a combo contains a real serial port selection."""

    return combo.count() > 0 and combo.currentText() != empty_serial_port_text(host)


def populate_serial_port_options(
    host: SerialPortOptionsHost,
    combo: SerialPortCombo,
    *,
    ports: list[str],
    current: str,
) -> None:
    """Populate a serial port combo while preserving a valid current port."""

    combo.clear()
    if ports:
        combo.addItems(ports)
        if current in ports:
            combo.setCurrentText(current)
        return
    combo.addItem(empty_serial_port_text(host))
