"""Connection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.connection_control_state import set_connection_control_state
from embeddebug.serial_station.ui.serial_connection_fields import read_serial_connection_fields
from embeddebug.serial_station.ui.serial_port_options import (
    combo_has_serial_ports,
    populate_serial_port_options,
)
from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


def _set_loading(button: QPushButton | None, loading: bool) -> None:
    """Batch 47/49-5: 按钮加载态切换（连接中反馈）。None 时安全跳过。

    Batch 49-5 升级：原实现只 swap 文字「…」，现委托到
    ``connection_loading.set_button_loading``，在按钮内嵌 indeterminate
    ProgressRing（小尺寸居中），对齐 spec B49-5 加载态要求。
    """

    from embeddebug.serial_station.ui.connection_loading import set_button_loading

    set_button_loading(button, loading)


class ConnectionActionHost(Protocol):
    """Minimal main-window surface needed by connection action handlers."""

    def tr(self, text: str) -> str: ...

    def _set_connected_controls(self, connected: bool) -> None: ...

    def _has_serial_ports(self) -> bool: ...


def connect_fake(host: ConnectionActionHost) -> None:
    _set_loading(getattr(host, '_connect_button', None), True)
    # Batch 49-2: 连接开始显示日志加载态（SkeletonBlock + 文案）。
    _show_log_loading(host)
    # Batch 49-3: 同步切换波形加载态（连接中显示 ProgressRing 覆盖层）。
    _set_waveform_connecting(host, True)
    result = host._controller.connect_fake_result()
    _set_loading(getattr(host, '_connect_button', None), False)
    _set_waveform_connecting(host, False)
    _hide_log_loading(host)
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
    _set_loading(getattr(host, '_connect_serial_button', None), True)
    # Batch 49-2/49-3: 同步日志 + 波形加载态。
    _show_log_loading(host)
    _set_waveform_connecting(host, True)
    result = host._controller.connect_serial_result(
        fields.port_name,
        fields.baud_rate,
        data_bits=fields.data_bits,
        parity=fields.parity,
        stop_bits=fields.stop_bits,
        flow_control=fields.flow_control,
    )
    _set_loading(getattr(host, '_connect_serial_button', None), False)
    _set_waveform_connecting(host, False)
    _hide_log_loading(host)
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
    # Batch 49-2/49-3: 断开时确保加载态已隐藏（防御性，正常路径 connect 结束已 hide）。
    _hide_log_loading(host)
    _set_waveform_connecting(host, False)
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


def _show_log_loading(host: ConnectionActionHost) -> None:
    """Batch 49-2: 显示日志连接加载态（委托 log_loading_state helper）。"""

    from embeddebug.serial_station.ui.log_loading_state import show_log_loading_state

    show_log_loading_state(host)


def _hide_log_loading(host: ConnectionActionHost) -> None:
    """Batch 49-2: 隐藏日志连接加载态。"""

    from embeddebug.serial_station.ui.log_loading_state import hide_log_loading_state

    hide_log_loading_state(host)


def _set_waveform_connecting(host: ConnectionActionHost, connecting: bool) -> None:
    """Batch 49-3: 切换波形连接加载态（ProgressRing 覆盖层）。

    host 可能没有 _waveform_preview（测试场景或非主窗口），getattr 安全降级。
    """

    preview = getattr(host, "_waveform_preview", None)
    if preview is None:
        return
    set_connecting = getattr(preview, "set_connecting", None)
    if callable(set_connecting):
        set_connecting(connecting)
