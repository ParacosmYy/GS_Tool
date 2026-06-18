"""Connection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.connection_control_state import set_connection_control_state
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
        # Batch 8: 空端口校验失败 → 抖动 port_combo 反馈（激活 ShakeAnimation）。
        _shake_widget(host._port_combo)
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


def _shake_widget(widget: object) -> None:
    """对控件触发左右抖动动画（校验失败反馈，Batch 8：激活 ShakeAnimation）。

    动画失败不阻塞（抖动是锦上添花）。
    """

    try:
        from embeddebug.serial_station.ui.animations.shake import ShakeAnimation

        ShakeAnimation.shake(widget).start()
    except Exception:
        pass


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
