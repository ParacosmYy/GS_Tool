from __future__ import annotations

from embeddebug.serial_station.controllers import controller_protocol_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry


def test_protocol_runtime_lists_protocols_and_records_selection_entry():
    runtime = controller_protocol_state.create_protocol_runtime()
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[SerialWorkbenchLogEntry] = []

    assert "raw_data" in controller_protocol_state.available_protocols(runtime)

    runtime = controller_protocol_state.set_protocol(
        runtime,
        "fire_water",
        entries=entries,
        log_callbacks=[logged.append],
    )

    assert runtime.dispatcher.protocol_name == "fire_water"
    assert runtime.receive_state.measurement_ring is None
    assert entries[-1].direction == "system"
    assert entries[-1].text == "protocol: fire_water"
    assert logged == [entries[-1]]


def test_protocol_runtime_handles_received_bytes_and_entry_conversion():
    runtime = controller_protocol_state.create_protocol_runtime()
    runtime = controller_protocol_state.set_protocol(runtime, "fire_water", entries=[], log_callbacks=[])
    entries: list[SerialWorkbenchLogEntry] = []
    measured = []

    runtime = controller_protocol_state.handle_received_bytes(
        runtime,
        b"3.0,4.5\n",
        entries=entries,
        log_callbacks=[],
        measurement_callbacks=[measured.append],
    )
    event = controller_protocol_state.protocol_event_from_entry(runtime, entries[-1])

    assert entries[-1].text == "3.0,4.5"
    assert measured[-1].values.tolist() == [[3.0, 4.5]]
    assert runtime.receive_state.measurement_ring is not None
    assert event.protocol_name == "fire_water"
    assert event.payload["direction"] == "rx"
