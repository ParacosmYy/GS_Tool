from __future__ import annotations

import json

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import FakeSerialTransport


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


def test_workbench_controller_connects_serial_transport_and_profiles(tmp_path):
    serial_transport = FakeSerialTransport()
    controller = SerialWorkbenchController(
        serial_transport_factory=lambda: serial_transport,
        port_provider=lambda: ["COM_TEST"],
    )
    profile_path = tmp_path / "serial-profile.json"

    assert controller.available_serial_ports() == ("COM_TEST",)
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
