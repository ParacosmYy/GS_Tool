"""Endpoint control copy helpers for Serial Station widgets."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal


EndpointMode = Literal["tcp", "udp"]


@dataclass(frozen=True)
class EndpointControlText:
    """User-visible copy used by one endpoint control group."""

    host_placeholder: str
    host_tooltip: str
    port_placeholder: str
    port_tooltip: str
    connect_label: str
    connect_tooltip: str


_TEXT_BY_MODE: dict[EndpointMode, EndpointControlText] = {
    "tcp": EndpointControlText(
        host_placeholder="TCP host",
        host_tooltip="TCP host name or address",
        port_placeholder="TCP port",
        port_tooltip="TCP port number",
        connect_label="Connect TCP",
        connect_tooltip="Open a TCP client connection",
    ),
    "udp": EndpointControlText(
        host_placeholder="UDP host",
        host_tooltip="UDP remote host name or address",
        port_placeholder="UDP port",
        port_tooltip="UDP remote port number",
        connect_label="Connect UDP",
        connect_tooltip="Open a UDP datagram connection",
    ),
}


def endpoint_control_text(mode: EndpointMode) -> EndpointControlText:
    return _TEXT_BY_MODE[mode]
