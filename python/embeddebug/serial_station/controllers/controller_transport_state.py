"""Transport runtime helpers for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, replace

from embeddebug.serial_station.controllers import controller_connection_state as connection_state
from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig, SerialTransport, TransportRegistry
from embeddebug.shared import OperationResult


TransportFactory = Callable[[], SerialTransport]
PortProvider = Callable[[], list[str]]


@dataclass(frozen=True)
class TransportRuntime:
    """Controller-owned transport, registry and active mode."""

    transport: SerialTransport
    registry: TransportRegistry
    bytes_callback: connection_state.BytesCallback
    error_callback: log_state.ErrorCallback
    mode: str = "fake"


def create_transport_runtime(
    *,
    transport: SerialTransport | None = None,
    transport_registry: TransportRegistry | None = None,
    serial_transport_factory: TransportFactory | None = None,
    port_provider: PortProvider | None = None,
    bytes_callback: connection_state.BytesCallback,
    error_callback: log_state.ErrorCallback,
) -> TransportRuntime:
    selected_transport = transport or FakeSerialTransport()
    registry = transport_registry or TransportRegistry.with_defaults(
        serial_factory=serial_transport_factory,
        serial_port_provider=port_provider,
    )
    connection_state.bind_transport(selected_transport, bytes_callback, error_callback)
    return TransportRuntime(
        transport=selected_transport,
        registry=registry,
        bytes_callback=bytes_callback,
        error_callback=error_callback,
    )


def is_connected(runtime: TransportRuntime) -> bool:
    return runtime.transport.is_open


def active_local_port(runtime: TransportRuntime) -> int | None:
    return connection_state.active_local_port(runtime.transport)


def available_serial_ports(runtime: TransportRuntime) -> tuple[str, ...]:
    return runtime.registry.available_ports("serial")


def available_transport_modes(runtime: TransportRuntime) -> tuple[str, ...]:
    return runtime.registry.modes


def disconnect(
    runtime: TransportRuntime,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
) -> None:
    connection_state.disconnect_transport(runtime.transport, runtime.mode, entries, log_callbacks)


def connect_fake_result(
    runtime: TransportRuntime,
    *,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
) -> tuple[TransportRuntime, OperationResult[SerialPortConfig]]:
    next_transport = runtime.transport

    def replace_transport(transport: SerialTransport) -> None:
        nonlocal next_transport
        next_transport = connection_state.replace_transport(
            runtime.transport,
            transport,
            runtime.bytes_callback,
            runtime.error_callback,
        )

    result = connection_state.connect_fake_transport_result(
        runtime.transport,
        runtime.registry,
        replace_transport,
        entries,
        log_callbacks,
    )
    return replace(runtime, transport=next_transport, mode="fake"), result


def connect_serial_result(
    runtime: TransportRuntime,
    port_name: str,
    baud_rate: int,
    *,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
    data_bits: int = 8,
    parity: str = "none",
    stop_bits: str = "1",
    flow_control: str = "none",
) -> tuple[TransportRuntime, OperationResult[SerialPortConfig]]:
    next_transport = runtime.transport

    def replace_transport(transport: SerialTransport) -> None:
        nonlocal next_transport
        next_transport = connection_state.replace_transport(
            runtime.transport,
            transport,
            runtime.bytes_callback,
            runtime.error_callback,
        )

    result = connection_state.connect_serial_transport_result(
        runtime.registry,
        replace_transport,
        port_name,
        baud_rate,
        entries,
        log_callbacks,
        data_bits=data_bits,
        parity=parity,
        stop_bits=stop_bits,
        flow_control=flow_control,
    )
    return replace(runtime, transport=next_transport, mode="serial"), result


def connect_endpoint_result(
    runtime: TransportRuntime,
    mode: str,
    host: str,
    port: int,
    *,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
) -> tuple[TransportRuntime, OperationResult[SerialPortConfig]]:
    next_transport = runtime.transport

    def replace_transport(transport: SerialTransport) -> None:
        nonlocal next_transport
        next_transport = connection_state.replace_transport(
            runtime.transport,
            transport,
            runtime.bytes_callback,
            runtime.error_callback,
        )

    result = connection_state.connect_endpoint_transport_result(
        runtime.registry,
        replace_transport,
        mode,
        host,
        port,
        entries,
        log_callbacks,
    )
    return replace(runtime, transport=next_transport, mode=mode), result
