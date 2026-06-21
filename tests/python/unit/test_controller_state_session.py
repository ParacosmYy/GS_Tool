"""Consolidated controller session state tests: session, profile, log, protocol."""
from __future__ import annotations
import json
from embeddebug.serial_station.controllers import controller_protocol_state
from embeddebug.serial_station.controllers.controller_log_state import (
    append_connected_entry,
    append_error_entry,
    append_log_entry,
    append_system_entry,
)
from embeddebug.serial_station.controllers.controller_profile_state import (
    load_profile_state_result,
    save_profile_state_result,
)
from embeddebug.serial_station.controllers.controller_session_state import (
    clear_entries,
    export_entries_result,
    replay_entries_into_state_result,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import (
    entry_from_event,
    event_from_entry,
)
from embeddebug.serial_station.drivers import SerialPortConfig
from embeddebug.shared import OperationResult
def test_clear_entries_removes_all_log_entries():
    entries = [SerialWorkbenchLogEntry("tx", "ping", b"ping")]
    clear_entries(entries)
    assert entries == []
def test_export_entries_result_persists_protocol_events(tmp_path):
    log_path = tmp_path / "session.jsonl"
    entries = [
        SerialWorkbenchLogEntry("system", "connected", b"connected"),
        SerialWorkbenchLogEntry("tx", "ping", b"ping"),
    ]
    result = export_entries_result(log_path, entries, lambda entry: event_from_entry(entry, "raw_data"))
    records = [json.loads(line) for line in log_path.read_text(encoding="utf-8").splitlines()]
    assert result.ok
    assert result.value == log_path
    assert [record["payload"]["direction"] for record in records] == ["system", "tx"]
def test_replay_entries_into_state_result_replaces_entries_and_notifies_callbacks(tmp_path):
    log_path = tmp_path / "session.jsonl"
    records = [
        {
            "type": "frame",
            "protocolName": "raw_data",
            "payload": {"text": "connected", "direction": "system"},
            "rawHex": b"connected".hex(),
        },
        {
            "type": "frame",
            "protocolName": "raw_data",
            "payload": {"text": "ping", "direction": "tx"},
            "rawHex": b"ping".hex(),
        },
    ]
    log_path.write_text("\n".join(json.dumps(record) for record in records), encoding="utf-8")
    entries = [SerialWorkbenchLogEntry("error", "old", b"old")]
    logged: list[str] = []
    result = replay_entries_into_state_result(
        log_path,
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
        entry_from_event,
    )
    assert result.ok
    assert [(entry.direction, entry.text) for entry in entries] == [
        ("system", "connected"),
        ("tx", "ping"),
    ]
    assert logged == ["system:connected", "tx:ping"]
def test_replay_entries_into_state_result_failure_keeps_existing_entries(tmp_path):
    entries = [SerialWorkbenchLogEntry("tx", "status?", b"status?")]
    result = replay_entries_into_state_result(
        tmp_path / "missing.jsonl",
        entries,
        [],
        entry_from_event,
    )
    assert result.failed
    assert result.error_code == "replay_load_failed"
    assert [(entry.direction, entry.text) for entry in entries] == [("tx", "status?")]
def test_save_profile_state_result_persists_transport_protocol_and_history(tmp_path):
    profile_path = tmp_path / "bench-profile.json"
    result = save_profile_state_result(
        profile_path,
        "bench-profile",
        mode="tcp",
        config=SerialPortConfig("127.0.0.1:19002", baud_rate=0),
        is_connected=True,
        protocol="raw_data",
        command_history=("status?", "reset"),
    )
    profile = json.loads(profile_path.read_text(encoding="utf-8"))
    assert result.ok
    assert result.value == profile_path
    assert profile["name"] == "bench-profile"
    assert profile["transport"]["mode"] == "tcp"
    assert profile["transport"]["portName"] == "127.0.0.1:19002"
    assert profile["protocol"] == "raw_data"
    assert profile["commandHistory"] == ["status?", "reset"]
def test_load_profile_state_result_restores_history_and_logs_profile_name(tmp_path):
    profile_path = tmp_path / "factory-profile.json"
    profile_path.write_text(
        json.dumps({"name": "factory-profile", "commandHistory": ["status?", "", "reset"]}),
        encoding="utf-8",
    )
    history: list[str] = ["old"]
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    result = load_profile_state_result(
        profile_path,
        history,
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
    )
    assert result.ok
    assert history == ["status?", "reset"]
    assert entries[-1].direction == "system"
    assert entries[-1].text == "profile loaded: factory-profile"
    assert logged == ["system:profile loaded: factory-profile"]
def test_load_profile_state_result_failure_does_not_mutate_history_or_entries(tmp_path):
    history: list[str] = ["status?"]
    entries: list[SerialWorkbenchLogEntry] = []
    result = load_profile_state_result(tmp_path / "missing.json", history, entries, [])
    assert result.failed
    assert result.error_code == "profile_load_failed"
    assert history == ["status?"]
    assert entries == []
def test_append_log_entry_stores_entry_and_notifies_log_callbacks():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    append_log_entry(
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
        SerialWorkbenchLogEntry("tx", "ping", b"ping"),
    )
    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("tx", "ping", b"ping")
    ]
    assert logged == ["tx:ping"]
def test_append_system_entry_builds_utf8_raw_and_notifies_log_callbacks():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    append_system_entry(entries, [lambda entry: logged.append(entry.text)], "profile loaded")
    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("system", "profile loaded", b"profile loaded")
    ]
    assert logged == ["profile loaded"]
def test_append_error_entry_logs_before_notifying_error_callbacks():
    entries: list[SerialWorkbenchLogEntry] = []
    events: list[str] = []
    append_error_entry(
        entries,
        [lambda entry: events.append(f"log:{entry.direction}:{entry.text}")],
        [lambda message: events.append(f"error:{message}:{len(entries)}")],
        "transport_not_open",
    )
    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("error", "transport_not_open", b"transport_not_open")
    ]
    assert events == ["log:error:transport_not_open", "error:transport_not_open:1"]
def test_append_connected_entry_logs_only_successful_configs():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    append_connected_entry(
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
        OperationResult.success(SerialPortConfig("FAKE_LOOPBACK", 115200)),
        "fake",
    )
    append_connected_entry(
        entries,
        [lambda entry: logged.append(entry.text)],
        OperationResult.failure("transport_open_failed", "denied"),
        "fake",
    )
    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("system", "connected: fake FAKE_LOOPBACK", b"connected: fake FAKE_LOOPBACK")
    ]
    assert logged == ["system:connected: fake FAKE_LOOPBACK"]
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
