from __future__ import annotations

import json

from embeddebug.serial_station.controllers.controller_session_state import (
    clear_entries,
    export_entries_result,
    replay_entries_into_state_result,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import entry_from_event, event_from_entry


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
