"""Connection and send actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.endpoint_validation import validate_endpoint_fields
from embeddebug.serial_station.ui.serial_port_options import (
    combo_has_serial_ports,
    populate_serial_port_options,
)
from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


class ConnectionActionHost(Protocol):
    """Minimal main-window surface needed by connection action handlers."""

    def tr(self, text: str) -> str: ...

    def _set_connected_controls(self, connected: bool) -> None: ...

    def _has_serial_ports(self) -> bool: ...

    def _refresh_command_history(self) -> None: ...


def connect_fake(host: ConnectionActionHost) -> None:
    result = host._controller.connect_fake_result()
    if result.ok:
        set_result_status(
            host,
            result,
            success_text="Connected to fake loopback",
            failure_prefix="Connection failed",
        )
        host._set_connected_controls(True)
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")


def connect_serial(host: ConnectionActionHost) -> None:
    port_name = host._port_combo.currentText()
    if not port_name or not host._has_serial_ports():
        set_status_text(host, "Serial port is empty")
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
        set_result_status(
            host,
            result,
            success_text="Connected to {port}",
            failure_prefix="Connection failed",
            port=port_name,
        )
        host._set_connected_controls(True)
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")


def connect_tcp(host: ConnectionActionHost) -> None:
    endpoint = _validated_tcp_endpoint(host)
    if endpoint is None:
        return
    tcp_host, port = endpoint
    result = host._controller.connect_tcp_result(tcp_host, port)
    if result.ok:
        set_result_status(
            host,
            result,
            success_text="Connected to TCP {endpoint}",
            failure_prefix="Connection failed",
            endpoint=f"{tcp_host}:{port}",
        )
        host._set_connected_controls(True)
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")


def connect_udp(host: ConnectionActionHost) -> None:
    endpoint = _validated_udp_endpoint(host)
    if endpoint is None:
        return
    udp_host, port = endpoint
    result = host._controller.connect_udp_result(udp_host, port)
    if result.ok:
        local_port = host._controller.active_local_port or 0
        set_result_status(
            host,
            result,
            success_text="Connected to UDP {endpoint} local {local_port}",
            failure_prefix="Connection failed",
            endpoint=f"{udp_host}:{port}",
            local_port=local_port,
        )
        host._set_connected_controls(True)
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")


def disconnect(host: ConnectionActionHost) -> None:
    host._controller.disconnect()
    set_status_text(host, "Disconnected")
    host._set_connected_controls(False)


def set_connected_controls(host: ConnectionActionHost, connected: bool) -> None:
    host._connect_button.setEnabled(not connected)
    host._connect_serial_button.setEnabled(not connected and has_serial_ports(host))
    host._connect_tcp_button.setEnabled(not connected)
    host._connect_udp_button.setEnabled(not connected)
    host._disconnect_button.setEnabled(connected)


def has_serial_ports(host: ConnectionActionHost) -> bool:
    return combo_has_serial_ports(host, host._port_combo)


def populate_serial_port_combo(host: ConnectionActionHost) -> None:
    current = host._port_combo.currentText()
    ports = host._controller.available_serial_ports()
    populate_serial_port_options(host, host._port_combo, ports=ports, current=current)


def refresh_serial_ports(host: ConnectionActionHost) -> None:
    populate_serial_port_combo(host)
    host._set_connected_controls(host._controller.is_connected)
    set_status_text(host, "Serial ports refreshed")


def send_text(host: ConnectionActionHost) -> None:
    text = host._send_edit.text()
    if not text:
        set_status_text(host, "Command is empty")
        return
    result = host._controller.send_text_result(text)
    if result.ok:
        refresh_command_history(host)
        set_result_status(host, result, success_text="Command sent", failure_prefix="Send failed")
        return
    set_result_status(host, result, success_text="", failure_prefix="Send failed")


def refresh_command_history(host: ConnectionActionHost) -> None:
    history = host._controller.command_history
    host._command_history_combo.blockSignals(True)
    host._command_history_combo.clear()
    host._command_history_combo.addItems(history)
    if history:
        host._command_history_combo.setCurrentText(history[-1])
    host._command_history_combo.setEnabled(bool(history))
    host._command_history_combo.blockSignals(False)


def select_command_history(host: ConnectionActionHost, text: str) -> None:
    if text:
        host._send_edit.setText(text)


def _validated_tcp_endpoint(host: ConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._tcp_host_edit.text(), host._tcp_port_edit.text(), "TCP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    return None


def _validated_udp_endpoint(host: ConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._udp_host_edit.text(), host._udp_port_edit.text(), "UDP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    return None
