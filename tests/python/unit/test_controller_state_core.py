"""Consolidated controller state tests: connection, callback, workbench, command history."""
from __future__ import annotations

from embeddebug.serial_station.controllers import controller_callback_state
from embeddebug.serial_station.controllers import controller_workbench_state
from embeddebug.serial_station.controllers.command_history_state import (
    remember_command,
    restore_command_history,
)
from embeddebug.serial_station.controllers.controller_connection_state import (
    active_local_port,
    connect_endpoint_transport_result,
    connect_fake_transport_result,
    disconnect_transport,
    replace_transport,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    SerialTransport,
    TransportRegistry,
)


class _NonFakeTransport(SerialTransport):
    @property
    def config(self) -> SerialPortConfig | None:
        return None

    @property
    def is_open(self) -> bool:
        return False

    def open(self, config: SerialPortConfig) -> bool:
        return True

    def close(self) -> None:
        return None

    def write(self, data: bytes) -> int:
        return len(data)

    def on_bytes_received(self, callback):
        return None

    def on_error(self, callback):
        return None


def test_active_local_port_ignores_missing_or_zero_ports():
    assert active_local_port(FakeSerialTransport()) is None

    transport = FakeSerialTransport()
    transport.local_port = 19001

    assert active_local_port(transport) == 19001


def test_replace_transport_closes_open_current_and_binds_callbacks():
    current = _NonFakeTransport()
    replacement = FakeSerialTransport()
    received: list[bytes] = []
    errors: list[str] = []

    assert current.open(SerialPortConfig("OLD", 115200))

    selected = replace_transport(current, replacement, received.append, errors.append)
    replacement.inject_rx(b"pong")
    replacement._emit_error("denied")

    assert selected is replacement
    assert not current.is_open
    assert received == [b"pong"]
    assert errors == ["denied"]


def test_disconnect_transport_logs_only_when_connection_was_open():
    transport = FakeSerialTransport()
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []

    disconnect_transport(transport, "fake", entries, [lambda entry: logged.append(entry.text)])
    assert entries == []

    assert transport.open(SerialPortConfig("FAKE_LOOPBACK", 115200))
    disconnect_transport(transport, "fake", entries, [lambda entry: logged.append(entry.text)])

    assert not transport.is_open
    assert [(entry.direction, entry.text) for entry in entries] == [
        ("system", "disconnected: fake")
    ]
    assert logged == ["disconnected: fake"]


def test_connect_fake_transport_result_replaces_non_fake_transport_and_logs_success():
    current = _NonFakeTransport()
    registry = TransportRegistry()
    registry.register("fake", factory=FakeSerialTransport)
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    selected: list[FakeSerialTransport] = []

    result = connect_fake_transport_result(
        current,
        registry,
        lambda transport: selected.append(transport) or transport,
        entries,
        [lambda entry: logged.append(entry.text)],
    )

    assert result.ok
    assert selected[-1].is_open
    assert selected[-1].config is not None
    assert selected[-1].config.port_name == "FAKE_LOOPBACK"
    assert [(entry.direction, entry.text) for entry in entries] == [
        ("system", "connected: fake FAKE_LOOPBACK")
    ]
    assert logged == ["connected: fake FAKE_LOOPBACK"]


def test_connect_endpoint_transport_result_logs_mode_and_endpoint():
    tcp_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register("tcp", factory=lambda: tcp_transport)
    entries: list[SerialWorkbenchLogEntry] = []

    result = connect_endpoint_transport_result(
        registry,
        lambda transport: transport,
        "tcp",
        "127.0.0.1",
        19002,
        entries,
        [],
    )

    assert result.ok
    assert result.value is not None
    assert result.value.port_name == "127.0.0.1:19002"
    assert entries[-1].text == "connected: tcp 127.0.0.1:19002"


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


def test_workbench_state_exposes_read_only_entry_and_history_snapshots():
    state = controller_workbench_state.create_workbench_state()
    entry = SerialWorkbenchLogEntry(direction="tx", text="AT", raw=b"AT")

    state.entries.append(entry)
    state.command_history.append("AT")

    assert controller_workbench_state.entries_snapshot(state) == (entry,)
    assert controller_workbench_state.command_history_snapshot(state) == ("AT",)


def test_workbench_state_clear_entries_keeps_command_history():
    state = controller_workbench_state.create_workbench_state()
    state.entries.append(SerialWorkbenchLogEntry(direction="rx", text="OK", raw=b"OK"))
    state.command_history.append("AT")

    controller_workbench_state.clear_entries(state)

    assert controller_workbench_state.entries_snapshot(state) == ()
    assert controller_workbench_state.command_history_snapshot(state) == ("AT",)


def test_remember_command_moves_existing_command_to_latest_position():
    history = ["ping", "pong"]

    remember_command(history, "ping")

    assert history == ["pong", "ping"]


def test_restore_command_history_keeps_only_non_empty_strings_with_latest_order():
    history = ["stale"]

    restore_command_history(history, ["status?", "", 123, "reset", "status?"])

    assert history == ["reset", "status?"]
