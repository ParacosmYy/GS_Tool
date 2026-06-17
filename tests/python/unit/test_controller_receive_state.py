from __future__ import annotations

from embeddebug.serial_station.controllers import controller_receive_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.core import SerialDispatcher
from embeddebug.serial_station.protocols import create_default_registry


def test_handle_received_bytes_appends_log_entries_and_measurement_batch():
    registry = create_default_registry()
    dispatcher = SerialDispatcher(registry.create("fire_water"))
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[SerialWorkbenchLogEntry] = []
    measured = []

    state = controller_receive_state.ReceiveState()
    state = controller_receive_state.handle_received_bytes(
        state=state,
        data=b"1.25,2.50\n",
        dispatcher=dispatcher,
        entries=entries,
        log_callbacks=[logged.append],
        measurement_callbacks=[measured.append],
    )

    assert entries[-1].direction == "rx"
    assert entries[-1].text == "1.25,2.50"
    assert logged == [entries[-1]]
    assert len(measured) == 1
    assert measured[-1].channel_names == ("ch1", "ch2")
    assert measured[-1].values.tolist() == [[1.25, 2.5]]
    assert state.measurement_ring is not None


def test_handle_error_records_log_entry_and_error_callback():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[SerialWorkbenchLogEntry] = []
    errors: list[str] = []

    controller_receive_state.handle_error(
        "transport_not_open",
        entries=entries,
        log_callbacks=[logged.append],
        error_callbacks=[errors.append],
    )

    assert entries[-1].direction == "error"
    assert entries[-1].text == "transport_not_open"
    assert logged == [entries[-1]]
    assert errors == ["transport_not_open"]
