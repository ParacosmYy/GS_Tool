"""Connection and send actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol


class ConnectionActionHost(Protocol):
    """Minimal main-window surface needed by connection action handlers."""

    def tr(self, text: str) -> str: ...

    def _set_connected_controls(self, connected: bool) -> None: ...

    def _has_serial_ports(self) -> bool: ...

    def _refresh_command_history(self) -> None: ...


def connect_fake(host: ConnectionActionHost) -> None:
    result = host._controller.connect_fake_result()
    if result.ok:
        host._status_label.setText(host.tr("Connected to fake loopback"))
        host._set_connected_controls(True)
        return
    host._status_label.setText(host.tr("Connection failed: {message}").format(message=result.message))


def connect_serial(host: ConnectionActionHost) -> None:
    port_name = host._port_combo.currentText()
    if not port_name or not host._has_serial_ports():
        host._status_label.setText(host.tr("Serial port is empty"))
        return
    baud_rate = int(host._baud_combo.currentText())
    result = host._controller.connect_serial_result(
        port_name,
        baud_rate,
        data_bits=int(host._data_bits_combo.currentText()),
        parity=host._parity_combo.currentText().lower(),
        stop_bits=host._stop_bits_combo.currentText(),
        flow_control=host._flow_control_combo.currentText().lower(),
    )
    if result.ok:
        host._status_label.setText(host.tr("Connected to {port}").format(port=port_name))
        host._set_connected_controls(True)
        return
    host._status_label.setText(host.tr("Connection failed: {message}").format(message=result.message))


def send_text(host: ConnectionActionHost) -> None:
    text = host._send_edit.text()
    if not text:
        host._status_label.setText(host.tr("Command is empty"))
        return
    result = host._controller.send_text_result(text)
    if result.ok:
        host._refresh_command_history()
        host._status_label.setText(host.tr("Command sent"))
        return
    host._status_label.setText(host.tr("Send failed: {message}").format(message=result.message))
