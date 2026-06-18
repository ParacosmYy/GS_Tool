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
    return None


def _validated_udp_endpoint(host: EndpointConnectionActionHost) -> tuple[str, int] | None:
    result = validate_endpoint_fields(host._udp_host_edit.text(), host._udp_port_edit.text(), "UDP")
    if result.ok:
        return result.host, result.port
    set_status_text(host, result.message)
    return None
