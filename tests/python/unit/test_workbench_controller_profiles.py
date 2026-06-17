from __future__ import annotations

import json

from embeddebug.serial_station.controllers import SerialWorkbenchController


def test_workbench_controller_exports_replays_and_profiles(tmp_path):
    controller = SerialWorkbenchController()
    log_path = tmp_path / "session.jsonl"
    profile_path = tmp_path / "profile.json"
    replayed: list[str] = []

    controller.on_log_entry(lambda entry: replayed.append(f"{entry.direction}:{entry.text}"))

    assert controller.connect_fake()
    assert controller.send_text("ping")
    controller.inject_received_text("pong")

    controller.export_log(log_path)
    assert len(log_path.read_text(encoding="utf-8").splitlines()) == 3

    controller.clear_log()
    assert controller.entries == ()

    controller.replay_log(log_path)
    assert [(entry.direction, entry.text) for entry in controller.entries] == [
        ("system", "connected: fake FAKE_LOOPBACK"),
        ("tx", "ping"),
        ("rx", "pong"),
    ]
    assert replayed[-3:] == ["system:connected: fake FAKE_LOOPBACK", "tx:ping", "rx:pong"]

    controller.save_profile(profile_path, "bench-profile")
    loaded = controller.load_profile(profile_path)

    assert loaded["name"] == "bench-profile"
    assert loaded["transport"]["portName"] == "FAKE_LOOPBACK"
    assert loaded["protocol"] == "raw_data"
    assert json.loads(profile_path.read_text(encoding="utf-8"))["name"] == "bench-profile"


def test_workbench_controller_persists_command_history_in_profiles(tmp_path):
    controller = SerialWorkbenchController()
    profile_path = tmp_path / "history-profile.json"

    assert controller.connect_fake()
    assert controller.send_text("status?")
    assert controller.send_text("reset")
    controller.save_profile(profile_path, "history-profile")

    profile = json.loads(profile_path.read_text(encoding="utf-8"))
    assert profile["commandHistory"] == ["status?", "reset"]

    restored = SerialWorkbenchController()
    restored.load_profile(profile_path)

    assert restored.command_history == ("status?", "reset")


def test_workbench_controller_records_profile_load_entry(tmp_path):
    profile_path = tmp_path / "factory-profile.json"
    controller = SerialWorkbenchController()
    logged: list[str] = []
    controller.save_profile(profile_path, "factory-profile")
    controller.on_log_entry(lambda entry: logged.append(f"{entry.direction}:{entry.text}"))

    result = controller.load_profile_result(profile_path)

    assert result.ok
    assert controller.entries[-1].direction == "system"
    assert controller.entries[-1].text == "profile loaded: factory-profile"
    assert logged[-1] == "system:profile loaded: factory-profile"


def test_workbench_controller_service_result_success_paths(tmp_path):
    controller = SerialWorkbenchController()
    log_path = tmp_path / "session-result.jsonl"
    profile_path = tmp_path / "profile-result.json"

    assert controller.connect_fake()
    assert controller.send_text("status?")

    export_result = controller.export_log_result(log_path)
    assert export_result.ok
    assert export_result.value == log_path

    controller.clear_log()
    replay_result = controller.replay_log_result(log_path)
    assert replay_result.ok
    assert [(entry.direction, entry.text) for entry in controller.entries] == [
        ("system", "connected: fake FAKE_LOOPBACK"),
        ("tx", "status?"),
    ]

    save_result = controller.save_profile_result(profile_path, "result-profile")
    assert save_result.ok
    assert save_result.value == profile_path

    controller.send_text("reset")
    load_result = controller.load_profile_result(profile_path)
    assert load_result.ok
    assert load_result.value is not None
    assert load_result.value["name"] == "result-profile"
    assert controller.command_history == ("status?",)


def test_workbench_controller_replays_diagnostic_log_directions(tmp_path):
    controller = SerialWorkbenchController()
    log_path = tmp_path / "diagnostic-session.jsonl"
    records = [
        {
            "type": "frame",
            "protocolName": "raw_data",
            "payload": {"text": "profile loaded", "direction": "system"},
            "rawHex": b"profile loaded".hex(),
        },
        {
            "type": "frame",
            "protocolName": "raw_data",
            "payload": {"text": "port denied", "direction": "error"},
            "rawHex": b"port denied".hex(),
        },
    ]
    log_path.write_text("\n".join(json.dumps(record) for record in records), encoding="utf-8")

    replay_result = controller.replay_log_result(log_path)

    assert replay_result.ok
    assert [(entry.direction, entry.text) for entry in controller.entries] == [
        ("system", "profile loaded"),
        ("error", "port denied"),
    ]


def test_workbench_controller_service_result_failures_do_not_mutate_state(tmp_path):
    controller = SerialWorkbenchController()
    blocked_parent = tmp_path / "blocked"
    missing_log = tmp_path / "missing.jsonl"
    missing_profile = tmp_path / "missing-profile.json"
    blocked_parent.write_text("not a directory", encoding="utf-8")

    assert controller.connect_fake()
    assert controller.send_text("status?")
    before_entries = controller.entries
    before_history = controller.command_history

    export_result = controller.export_log_result(blocked_parent / "session.jsonl")
    assert export_result.failed
    assert export_result.error_code == "log_export_failed"

    replay_result = controller.replay_log_result(missing_log)
    assert replay_result.failed
    assert replay_result.error_code == "replay_load_failed"
    assert controller.entries == before_entries

    save_result = controller.save_profile_result(blocked_parent / "profile.json", "blocked")
    assert save_result.failed
    assert save_result.error_code == "profile_save_failed"

    load_result = controller.load_profile_result(missing_profile)
    assert load_result.failed
    assert load_result.error_code == "profile_load_failed"
    assert controller.command_history == before_history
