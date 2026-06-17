from __future__ import annotations

import json

from embeddebug.serial_station.controllers.controller_profile_state import (
    load_profile_state_result,
    save_profile_state_result,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.drivers import SerialPortConfig


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
