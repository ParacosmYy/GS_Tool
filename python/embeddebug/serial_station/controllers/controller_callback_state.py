"""Callback registration state for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers import controller_receive_state as receive_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.core import ChannelBatch


MeasurementCallback = Callable[[ChannelBatch], None]
LogEntryCallback = log_state.LogEntryCallback
ErrorCallback = log_state.ErrorCallback


@dataclass
class CallbackState:
    """Own controller callback lists without leaking list fields into the facade."""

    log: list[LogEntryCallback] = field(default_factory=list)
    error: list[ErrorCallback] = field(default_factory=list)
    measurement: list[MeasurementCallback] = field(default_factory=list)


def create_callback_state() -> CallbackState:
    return CallbackState()


def add_log_callback(state: CallbackState, callback: LogEntryCallback) -> None:
    state.log.append(callback)


def add_error_callback(state: CallbackState, callback: ErrorCallback) -> None:
    state.error.append(callback)


def add_measurement_callback(state: CallbackState, callback: MeasurementCallback) -> None:
    state.measurement.append(callback)


def handle_error(message: str, state: CallbackState, entries: list[SerialWorkbenchLogEntry]) -> None:
    receive_state.handle_error(
        message,
        entries=entries,
        log_callbacks=state.log,
        error_callbacks=state.error,
    )
