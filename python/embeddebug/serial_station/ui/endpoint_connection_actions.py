"""TCP and UDP endpoint connection actions for Serial Station."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.endpoint_validation import validate_endpoint_fields
from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


class EndpointConnectionActionHost(Protocol):
    """Minimal main-window surface needed by endpoint connection actions."""

    def tr(self, text: str) -> str: ...

    def _set_connected_controls(self, connected: bool) -> None: ...


def connect_tcp(host: EndpointConnectionActionHost) -> None:
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
        _notify(
            host, "success", host.tr("已连接"),
            host.tr("TCP {endpoint} 已就绪").format(endpoint=f"{tcp_host}:{port}"),
        )
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")
    _notify_result(host, result, failure_title=host.tr("TCP 连接失败"))


def connect_udp(host: EndpointConnectionActionHost) -> None:
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
        _notify(
            host, "success", host.tr("已连接"),
            host.tr("UDP {endpoint} 本地 {local_port}").format(
                endpoint=f"{udp_host}:{port}", local_port=local_port
            ),
        )
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")
    _notify_result(host, result, failure_title=host.tr("UDP 连接失败"))


def _validated_tcp_endpoint(host: EndpointConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._tcp_host_edit.text(), host._tcp_port_edit.text(), "TCP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    # Batch 8: 校验失败 → 抖动出错字段（host 空抖 host_edit，port 非法抖 port_edit）。
    _shake_failed_endpoint_field(host, result.message, host._tcp_host_edit, host._tcp_port_edit)
    # Batch 13: 校验失败 → warning toast。
    _notify(host, "warning", host.tr("TCP 地址非法"), result.message)
    return None


def _validated_udp_endpoint(host: EndpointConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._udp_host_edit.text(), host._udp_port_edit.text(), "UDP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    # Batch 8: 校验失败 → 抖动出错字段。
    _shake_failed_endpoint_field(host, result.message, host._udp_host_edit, host._udp_port_edit)
    # Batch 13: 校验失败 → warning toast。
    _notify(host, "warning", host.tr("UDP 地址非法"), result.message)
    return None


def _notify(host: EndpointConnectionActionHost, level: str, title: str, message: str) -> None:
    """触发 toast 通知（Batch 13：TCP/UDP 连接工作流 → 通知系统）。

    host 可能未实现 _notify（无 AppShell 的单窗口/测试场景），getattr 安全降级。
    """

    notify_fn = getattr(host, "_notify", None)
    if callable(notify_fn):
        try:
            notify_fn(level, title, message)
        except Exception:
            pass


def _notify_result(
    host: EndpointConnectionActionHost, result: object, *, failure_title: str
) -> None:
    """OperationResult 失败路径 → error toast（Batch 13）。"""

    if getattr(result, "ok", True):
        return
    message = getattr(result, "message", "") or ""
    _notify(host, "error", failure_title, message)


def _shake_failed_endpoint_field(host: EndpointConnectionActionHost, message: str,
                                 host_edit: object, port_edit: object) -> None:
    """根据校验失败消息抖动对应字段（Batch 8：激活 ShakeAnimation）。

    消息含 'host' 抖 host_edit，含 'port' 抖 port_edit，无法区分时两者都抖。
    """

    try:
        from embeddebug.serial_station.ui.animations.shake import ShakeAnimation

        msg_lower = message.lower()
        if "host" in msg_lower:
            ShakeAnimation.shake(host_edit).start()
        elif "port" in msg_lower:
            ShakeAnimation.shake(port_edit).start()
        else:
            # 无法区分，抖 host 字段（通常 host 先校验）。
            ShakeAnimation.shake(host_edit).start()
    except Exception:
        pass
