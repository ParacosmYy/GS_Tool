"""Protocol runtime helpers for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Sequence
from dataclasses import dataclass

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers import controller_receive_state as receive_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import event_from_entry
from embeddebug.serial_station.core import SerialDispatcher
from embeddebug.serial_station.protocols import ProtocolEvent, SerialProtocolRegistry, create_default_registry


@dataclass(frozen=True)
class ProtocolRuntime:
    """Controller-owned protocol registry, dispatcher and receive buffers."""

    registry: SerialProtocolRegistry
    dispatcher: SerialDispatcher
    receive_state: receive_state.ReceiveState


def create_protocol_runtime() -> ProtocolRuntime:
    registry = create_default_registry()
    return ProtocolRuntime(
        registry=registry,
        dispatcher=SerialDispatcher(registry.create("raw_data")),
        receive_state=receive_state.ReceiveState(),
    )


def available_protocols(runtime: ProtocolRuntime) -> tuple[str, ...]:
    return tuple(runtime.registry.names())


def set_protocol(
    runtime: ProtocolRuntime,
    name: str,
    *,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
) -> ProtocolRuntime:
    runtime.dispatcher.set_protocol(runtime.registry.create(name))
    log_state.append_system_entry(entries, log_callbacks, f"protocol: {name}")
    return ProtocolRuntime(
        registry=runtime.registry,
        dispatcher=runtime.dispatcher,
        receive_state=receive_state.ReceiveState(),
    )


def handle_received_bytes(
    runtime: ProtocolRuntime,
    data: bytes,
    *,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
    measurement_callbacks: Sequence[receive_state.MeasurementCallback],
) -> ProtocolRuntime:
    next_receive_state = receive_state.handle_received_bytes(
        state=runtime.receive_state,
        data=data,
        dispatcher=runtime.dispatcher,
        entries=entries,
        log_callbacks=log_callbacks,
        measurement_callbacks=measurement_callbacks,
    )
    return ProtocolRuntime(
        registry=runtime.registry,
        dispatcher=runtime.dispatcher,
        receive_state=next_receive_state,
    )


def protocol_event_from_entry(runtime: ProtocolRuntime, entry: SerialWorkbenchLogEntry) -> ProtocolEvent:
    return event_from_entry(entry, runtime.dispatcher.protocol_name)
