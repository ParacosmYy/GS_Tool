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
        _notify(host, "success", host.tr("已连接"), host.tr("Fake loopback 通道已就绪"))
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")
    _notify_result(host, result, failure_title=host.tr("连接失败"))


def connect_serial(host: ConnectionActionHost) -> None:
    fields = read_serial_connection_fields(host)
    if not fields.port_name or not host._has_serial_ports():
        set_status_text(host, "Serial port is empty")
        # Batch 8: 空端口校验失败 → 抖动 port_combo 反馈（激活 ShakeAnimation）。
        _shake_widget(host._port_combo)
        _notify(host, "warning", host.tr("请选择端口"), host.tr("串口端口为空，无法连接"))
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
        _notify(
            host, "success", host.tr("已连接"),
            host.tr("串口 {port} 已就绪").format(port=fields.port_name),
        )
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")
    _notify_result(host, result, failure_title=host.tr("连接失败"))


def _shake_widget(widget: object) -> None:
    """对控件触发左右抖动动画（校验失败反馈，Batch 8：激活 ShakeAnimation）。

    动画失败不阻塞（抖动是锦上添花）。
    """

    try:
        from embeddebug.serial_station.ui.animations.shake import ShakeAnimation

        ShakeAnimation.shake(widget).start()
    except Exception:
        pass


def _notify(host: ConnectionActionHost, level: str, title: str, message: str) -> None:
    """触发 toast 通知（Batch 13：连接工作流 → 通知系统）。

    host 可能未实现 _notify（无 AppShell 的单窗口/测试场景），getattr 安全降级。
    """

    notify_fn = getattr(host, "_notify", None)
    if callable(notify_fn):
        try:
            notify_fn(level, title, message)
        except Exception:
            pass


def _notify_result(
    host: ConnectionActionHost, result: object, *, failure_title: str
) -> None:
    """OperationResult 失败路径 → error toast（Batch 13）。"""

    if getattr(result, "ok", True):
        return
    message = getattr(result, "message", "") or ""
    _notify(host, "error", failure_title, message)


def disconnect(host: ConnectionActionHost) -> None:
    host._controller.disconnect()
    set_status_text(host, "Disconnected")
    host._set_connected_controls(False)
    _notify(host, "info", host.tr("已断开"), host.tr("连接已关闭"))


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
    # Batch 13: 端口刷新完成 → info toast（反馈扫描结果）。
    port_count = host._port_combo.count() if hasattr(host._port_combo, "count") else 0
    _notify(
        host, "info", host.tr("端口已刷新"),
        host.tr("发现 {n} 个串口端口").format(n=port_count),
    )
