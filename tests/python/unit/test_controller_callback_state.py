from __future__ import annotations

from embeddebug.serial_station.controllers import controller_callback_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry


def test_callback_state_registers_and_exposes_callback_groups():
    callbacks = controller_callback_state.create_callback_state()
    logs: list[SerialWorkbenchLogEntry] = []
    errors: list[str] = []
    batches = []

    controller_callback_state.add_log_callback(callbacks, logs.append)
    controller_callback_state.add_error_callback(callbacks, errors.append)
    controller_callback_state.add_measurement_callback(callbacks, batches.append)

    assert callbacks.log == [logs.append]
    assert callbacks.error == [errors.append]
    assert callbacks.measurement == [batches.append]


def test_callback_state_records_errors_through_receive_state():
    callbacks = controller_callback_state.create_callback_state()
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[SerialWorkbenchLogEntry] = []
    errors: list[str] = []
    controller_callback_state.add_log_callback(callbacks, logged.append)
    controller_callback_state.add_error_callback(callbacks, errors.append)

    controller_callback_state.handle_error("callback_failure", callbacks, entries)

    assert entries[-1].direction == "error"
    assert entries[-1].text == "callback_failure"
    assert logged == [entries[-1]]
    assert errors == ["callback_failure"]
