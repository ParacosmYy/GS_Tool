"""Connection helpers for Serial Station controller transports."""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.controllers.connection_results import open_transport_result
from embeddebug.serial_station.drivers import SerialPortConfig, SerialTransport, TransportRegistry
from embeddebug.shared import OperationResult


ReplaceTransport = Callable[[SerialTransport], None]


def open_serial_transport(
    registry: TransportRegistry,
    replace_transport: ReplaceTransport,
    port_name: str,
    baud_rate: int,
    data_bits: int = 8,
    parity: str = "none",
    stop_bits: str = "1",
    flow_control: str = "none",
) -> OperationResult[SerialPortConfig]:
    """Create, install and open the configured UART transport."""
    transport = registry.create("serial")
    replace_transport(transport)
    config = SerialPortConfig(
        port_name=port_name,
        baud_rate=baud_rate,
        data_bits=data_bits,
        parity=parity,
        stop_bits=stop_bits,
        flow_control=flow_control,
    )
    return open_transport_result(transport, config, "serial")


def open_endpoint_transport(
    registry: TransportRegistry,
    replace_transport: ReplaceTransport,
    mode: str,
    host: str,
    port: int,
) -> OperationResult[SerialPortConfig]:
    """Create, install and open a host:port byte transport."""
    transport = registry.create(mode)
    replace_transport(transport)
    config = SerialPortConfig(port_name=f"{host}:{port}", baud_rate=0)
    return open_transport_result(transport, config, mode)
