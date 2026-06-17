from __future__ import annotations

import json

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    SerialTransport,
    TransportRegistry,
)


class _NonFakeTransport(SerialTransport):
    def __init__(self) -> None:
        self._errors = []

    @property
    def config(self) -> SerialPortConfig | None:
        return None

    @property
    def is_open(self) -> bool:
        return True

    def open(self, config: SerialPortConfig) -> bool:
        return True

    def close(self) -> None:
        return None

    def write(self, data: bytes) -> int:
        return len(data)

    def on_bytes_received(self, callback):
        return None

    def on_error(self, callback):
        self._errors.append(callback)


def test_workbench_controller_tracks_successful_command_history():
    controller = SerialWorkbenchController()
    logged: list[str] = []
    controller.on_log_entry(lambda entry: logged.append(f"{entry.direction}:{entry.text}"))

    assert controller.command_history == ()
    result = controller.send_text_result("before-open")

    assert result.failed
    assert result.error_code == "transport_not_open"
    assert result.message == "Open a transport before sending"
    assert controller.entries[-1].direction == "error"
    assert controller.entries[-1].text == "transport_not_open"
    assert logged[-1] == "error:transport_not_open"
    assert not controller.send_text("before-open")
    assert controller.command_history == ()

    assert controller.connect_fake()
    assert controller.send_text("ping")
    assert controller.send_text("pong")
    assert controller.send_text("ping")

    assert controller.command_history == ("pong", "ping")


def test_workbench_controller_records_connection_lifecycle_entries():
    controller = SerialWorkbenchController()
    logged: list[str] = []
    controller.on_log_entry(lambda entry: logged.append(f"{entry.direction}:{entry.text}"))

    result = controller.connect_fake_result()

    assert result.ok
    assert controller.entries[-1].direction == "system"
    assert controller.entries[-1].text == "connected: fake FAKE_LOOPBACK"
    assert logged[-1] == "system:connected: fake FAKE_LOOPBACK"

    controller.disconnect()

    assert [(entry.direction, entry.text) for entry in controller.entries[-2:]] == [
        ("system", "connected: fake FAKE_LOOPBACK"),
        ("system", "disconnected: fake"),
    ]
    assert logged[-2:] == [
        "system:connected: fake FAKE_LOOPBACK",
        "system:disconnected: fake",
    ]


def test_workbench_controller_records_protocol_selection_entry():
    controller = SerialWorkbenchController()
    logged: list[str] = []
    controller.on_log_entry(lambda entry: logged.append(f"{entry.direction}:{entry.text}"))

    controller.set_protocol("fire_water")

    assert controller.entries[-1].direction == "system"
    assert controller.entries[-1].text == "protocol: fire_water"
    assert logged[-1] == "system:protocol: fire_water"


def test_workbench_controller_inject_received_text_reports_non_fake_failure():
    controller = SerialWorkbenchController(transport=_NonFakeTransport())
    errors: list[str] = []
    controller.on_error(errors.append)

    result = controller.inject_received_text("pong")

    assert result.failed
    assert result.error_code == "fake_injection_requires_fake_transport"
    assert result.message == "Fake RX injection requires fake transport"
    assert errors == ["fake_injection_requires_fake_transport"]


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
