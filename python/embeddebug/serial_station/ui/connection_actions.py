"""Connection and send actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.command_entry_text import apply_command_history_selection
from embeddebug.serial_station.ui.command_history_options import populate_command_history_options
from embeddebug.serial_station.ui.connection_control_state import set_connection_control_state
from embeddebug.serial_station.ui.endpoint_validation import validate_endpoint_fields
from embeddebug.serial_station.ui.serial_connection_fields import read_serial_connection_fields
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
    fields = read_serial_connection_fields(host)
    if not fields.port_name or not host._has_serial_ports():
        set_status_text(host, "Serial port is empty")
        return
    result = host._controller.connect_serial_result(
        fields.port_name,
        fields.baud_rate,
        data_bits=fields.data_bits,
        parity=fields.parity,
        stop_bits=fields.stop_bits,
        flow_control=fields.flow_control,
    )
    if result.ok:
        set_result_status(
            host,
            result,
            success_text="Connected to {port}",
            failure_prefix="Connection failed",
            port=fields.port_name,
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
    set_connection_control_state(host, connected=connected, has_serial_ports=has_serial_ports(host))


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
    populate_command_history_options(host._command_history_combo, history)


def select_command_history(host: ConnectionActionHost, text: str) -> None:
    apply_command_history_selection(host._send_edit, text)


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
