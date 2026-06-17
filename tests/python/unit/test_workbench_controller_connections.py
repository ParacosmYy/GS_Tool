from __future__ import annotations

import json

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import FakeSerialTransport, TransportRegistry


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
