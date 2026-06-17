"""Connection control state helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class ConnectionControlHost(Protocol):
    """Minimal surface needed to update connection command buttons."""

    _connect_button: object
    _connect_serial_button: object
    _connect_tcp_button: object
    _connect_udp_button: object
    _disconnect_button: object


def set_connection_control_state(
    host: ConnectionControlHost, *, connected: bool, has_serial_ports: bool
) -> None:
    host._connect_button.setEnabled(not connected)
    host._connect_serial_button.setEnabled(not connected and has_serial_ports)
    host._connect_tcp_button.setEnabled(not connected)
    host._connect_udp_button.setEnabled(not connected)
    host._disconnect_button.setEnabled(connected)
