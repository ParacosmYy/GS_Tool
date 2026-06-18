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
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")


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
        return
    set_result_status(host, result, success_text="", failure_prefix="Connection failed")


def _validated_tcp_endpoint(host: EndpointConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._tcp_host_edit.text(), host._tcp_port_edit.text(), "TCP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    # Batch 8: 校验失败 → 抖动出错字段（host 空抖 host_edit，port 非法抖 port_edit）。
    _shake_failed_endpoint_field(host, result.message, host._tcp_host_edit, host._tcp_port_edit)
    return None


def _validated_udp_endpoint(host: EndpointConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._udp_host_edit.text(), host._udp_port_edit.text(), "UDP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    # Batch 8: 校验失败 → 抖动出错字段。
    _shake_failed_endpoint_field(host, result.message, host._udp_host_edit, host._udp_port_edit)
    return None


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
