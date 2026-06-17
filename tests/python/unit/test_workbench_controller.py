from __future__ import annotations

import json

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import FakeSerialTransport, TransportRegistry


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
    assert len(log_path.read_text(encoding="utf-8").splitlines()) == 2

    controller.clear_log()
    assert controller.entries == ()

    controller.replay_log(log_path)
    assert [entry.text for entry in controller.entries] == ["ping", "pong"]
    assert replayed[-2:] == ["tx:ping", "rx:pong"]

    controller.save_profile(profile_path, "bench-profile")
    loaded = controller.load_profile(profile_path)

    assert loaded["name"] == "bench-profile"
    assert loaded["transport"]["portName"] == "FAKE_LOOPBACK"
    assert loaded["protocol"] == "raw_data"
    assert json.loads(profile_path.read_text(encoding="utf-8"))["name"] == "bench-profile"


def test_workbench_controller_tracks_successful_command_history():
    controller = SerialWorkbenchController()

    assert controller.command_history == ()
    result = controller.send_text_result("before-open")

    assert result.failed
    assert result.error_code == "transport_not_open"
    assert result.message == "Open a transport before sending"
    assert not controller.send_text("before-open")
    assert controller.command_history == ()

    assert controller.connect_fake()
    assert controller.send_text("ping")
    assert controller.send_text("pong")
    assert controller.send_text("ping")

    assert controller.command_history == ("pong", "ping")


def test_workbench_controller_connect_fake_result_reports_open_failure():
    controller = SerialWorkbenchController(transport=FakeSerialTransport(open_error="denied"))
    errors: list[str] = []
    controller.on_error(errors.append)

    result = controller.connect_fake_result()

    assert result.failed
    assert result.error_code == "transport_open_failed"
    assert result.message == "Failed to open fake transport"
    assert errors == ["denied"]
    assert not controller.connect_fake()


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
    assert [entry.text for entry in controller.entries] == ["status?"]

    save_result = controller.save_profile_result(profile_path, "result-profile")
    assert save_result.ok
    assert save_result.value == profile_path

    controller.send_text("reset")
    load_result = controller.load_profile_result(profile_path)
    assert load_result.ok
    assert load_result.value is not None
    assert load_result.value["name"] == "result-profile"
    assert controller.command_history == ("status?",)


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


def test_workbench_controller_connects_serial_transport_and_profiles(tmp_path):
    serial_transport = FakeSerialTransport()
    controller = SerialWorkbenchController(
        serial_transport_factory=lambda: serial_transport,
        port_provider=lambda: ["COM_TEST"],
    )
    profile_path = tmp_path / "serial-profile.json"

    assert controller.available_serial_ports() == ("COM_TEST",)
    result = controller.connect_serial_result(
        "COM_TEST",
        57600,
        data_bits=7,
        parity="even",
        stop_bits="2",
        flow_control="software",
    )
    assert result.ok
    assert result.value is not None
    assert result.value.port_name == "COM_TEST"
    assert controller.connect_serial(
        "COM_TEST",
        57600,
        data_bits=7,
        parity="even",
        stop_bits="2",
        flow_control="software",
    )
    assert controller.is_connected
    assert serial_transport.config is not None
    assert serial_transport.config.port_name == "COM_TEST"
    assert serial_transport.config.baud_rate == 57600
    assert serial_transport.config.data_bits == 7
    assert serial_transport.config.parity == "even"
    assert serial_transport.config.stop_bits == "2"
    assert serial_transport.config.flow_control == "software"

    assert controller.send_text("ping")
    assert serial_transport.written == [b"ping"]

    controller.save_profile(profile_path, "serial-profile")
    profile = json.loads(profile_path.read_text(encoding="utf-8"))

    assert profile["transport"]["portName"] == "COM_TEST"
    assert profile["transport"]["baudRate"] == 57600
    assert profile["transport"]["dataBits"] == 7
    assert profile["transport"]["parity"] == "even"
    assert profile["transport"]["stopBits"] == "2"
    assert profile["transport"]["flowControl"] == "software"
    assert profile["transport"]["mode"] == "serial"


def test_workbench_controller_connect_tcp_result_profiles_endpoint(tmp_path):
    tcp_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register("fake", factory=FakeSerialTransport)
    registry.register("serial", factory=FakeSerialTransport)
    registry.register("tcp", factory=lambda: tcp_transport)
    controller = SerialWorkbenchController(transport_registry=registry)
    profile_path = tmp_path / "tcp-profile.json"

    result = controller.connect_tcp_result("127.0.0.1", 19002)

    assert result.ok
    assert result.value is not None
    assert result.value.port_name == "127.0.0.1:19002"
    assert controller.connect_tcp("127.0.0.1", 19002)

    controller.save_profile(profile_path, "tcp-profile")
    profile = json.loads(profile_path.read_text(encoding="utf-8"))

    assert profile["transport"]["mode"] == "tcp"
    assert profile["transport"]["portName"] == "127.0.0.1:19002"
