"""Transport lifecycle helpers for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers.connection_results import open_transport_result
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.transport_connections import open_endpoint_transport, open_serial_transport
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig, SerialTransport, TransportRegistry
from embeddebug.shared import OperationResult


BytesCallback = Callable[[bytes], None]
ReplaceTransport = Callable[[SerialTransport], None]


def active_local_port(transport: SerialTransport) -> int | None:
    return int(getattr(transport, "local_port", 0) or 0) or None


def bind_transport(
    transport: SerialTransport,
    bytes_callback: BytesCallback,
    error_callback: log_state.ErrorCallback,
) -> None:
    transport.on_bytes_received(bytes_callback)
    transport.on_error(error_callback)


def replace_transport(
    current: SerialTransport,
    replacement: SerialTransport,
    bytes_callback: BytesCallback,
    error_callback: log_state.ErrorCallback,
) -> SerialTransport:
    if current.is_open:
        current.close()
    bind_transport(replacement, bytes_callback, error_callback)
    return replacement


def disconnect_transport(
    transport: SerialTransport,
    mode: str,
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
) -> None:
    was_connected = transport.is_open
    transport.close()
    if was_connected:
        log_state.append_system_entry(entries, callbacks, f"disconnected: {mode}")


def connect_fake_transport_result(
    current: SerialTransport,
    registry: TransportRegistry,
    replace: ReplaceTransport,
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
) -> OperationResult[SerialPortConfig]:
    transport = current
    if not isinstance(current, FakeSerialTransport):
        transport = registry.create("fake")
        replace(transport)
    config = SerialPortConfig(port_name="FAKE_LOOPBACK", baud_rate=115200)
    result = open_transport_result(transport, config, "fake")
    log_state.append_connected_entry(entries, callbacks, result, "fake")
    return result


def connect_serial_transport_result(
    registry: TransportRegistry,
    replace: ReplaceTransport,
    port_name: str,
    baud_rate: int,
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
    *,
    data_bits: int = 8,
    parity: str = "none",
    stop_bits: str = "1",
    flow_control: str = "none",
) -> OperationResult[SerialPortConfig]:
    result = open_serial_transport(
        registry,
        replace,
        port_name,
        baud_rate,
        data_bits=data_bits,
        parity=parity,
        stop_bits=stop_bits,
        flow_control=flow_control,
    )
    log_state.append_connected_entry(entries, callbacks, result, "serial")
    return result


def connect_endpoint_transport_result(
    registry: TransportRegistry,
    replace: ReplaceTransport,
    mode: str,
    host: str,
    port: int,
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
) -> OperationResult[SerialPortConfig]:
    result = open_endpoint_transport(registry, replace, mode, host, port)
    log_state.append_connected_entry(entries, callbacks, result, mode)
    return result
