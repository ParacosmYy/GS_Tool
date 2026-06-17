"""Receive-side state helpers for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import entry_from_event
from embeddebug.serial_station.controllers.measurement_buffer import append_measurement_batch
from embeddebug.serial_station.core import (
    ChannelBatch,
    ChannelRingBuffer,
    SerialDispatcher,
    batch_from_measurement_events,
)


MeasurementCallback = Callable[[ChannelBatch], None]


@dataclass(frozen=True)
class ReceiveState:
    """Controller-owned receive buffers kept outside the workbench facade."""

    measurement_ring: ChannelRingBuffer | None = None


def handle_received_bytes(
    *,
    state: ReceiveState,
    data: bytes,
    dispatcher: SerialDispatcher,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
    measurement_callbacks: list[MeasurementCallback],
) -> ReceiveState:
    events = dispatcher.feed(data)
    for event in events:
        log_state.append_log_entry(entries, log_callbacks, entry_from_event(event))

    batch = batch_from_measurement_events(events)
    if batch is None:
        return state

    measurement_ring, latest = append_measurement_batch(state.measurement_ring, batch)
    for callback in list(measurement_callbacks):
        callback(latest)
    return ReceiveState(measurement_ring=measurement_ring)


def handle_error(
    message: str,
    *,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
    error_callbacks: list[log_state.ErrorCallback],
) -> None:
    log_state.append_error_entry(entries, log_callbacks, error_callbacks, message)
