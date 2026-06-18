from __future__ import annotations

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    TcpClientTransport,
    TransportRegistry,
    UdpDatagramTransport,
)


def test_transport_registry_exposes_default_modes_and_fake_transport():
    registry = TransportRegistry.with_defaults()

    assert registry.modes == ("fake", "serial", "tcp", "tcp_server", "udp")
    assert registry.available_ports("fake") == ("FAKE_LOOPBACK",)

    transport = registry.create("fake")
    assert isinstance(transport, FakeSerialTransport)
    assert transport.open(SerialPortConfig(port_name="FAKE_LOOPBACK"))

    assert isinstance(registry.create("tcp"), TcpClientTransport)
    assert isinstance(registry.create("udp"), UdpDatagramTransport)


def test_transport_registry_injects_serial_driver_into_controller(tmp_path):
    serial_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register(
        "serial",
        factory=lambda: serial_transport,
        port_provider=lambda: ["COM_REGISTRY"],
    )
    registry.register(
        "fake",
        factory=FakeSerialTransport,
        port_provider=lambda: ["FAKE_LOOPBACK"],
    )
    controller = SerialWorkbenchController(transport_registry=registry)
    profile_path = tmp_path / "profile.json"

    assert controller.available_serial_ports() == ("COM_REGISTRY",)
    assert controller.connect_serial("COM_REGISTRY", 115200)
    assert controller.send_text("registry")

    controller.save_profile(profile_path, "registry-profile")

    assert serial_transport.written == [b"registry"]
    assert '"mode": "serial"' in profile_path.read_text(encoding="utf-8")


def test_transport_registry_injects_tcp_driver_into_controller(tmp_path):
    tcp_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register(
        "tcp",
        factory=lambda: tcp_transport,
        port_provider=lambda: [],
    )
    registry.register(
        "serial",
        factory=FakeSerialTransport,
        port_provider=lambda: ["COM_REGISTRY"],
    )
    registry.register(
        "fake",
        factory=FakeSerialTransport,
        port_provider=lambda: ["FAKE_LOOPBACK"],
    )
    controller = SerialWorkbenchController(transport_registry=registry)
    profile_path = tmp_path / "tcp-profile.json"

    assert controller.available_transport_modes() == ("tcp", "serial", "fake")
    assert controller.connect_tcp("127.0.0.1", 19001)
    assert controller.send_text("tcp-registry")

    controller.save_profile(profile_path, "tcp-profile")

    profile_text = profile_path.read_text(encoding="utf-8")
    assert tcp_transport.written == [b"tcp-registry"]
    assert '"mode": "tcp"' in profile_text
    assert '"portName": "127.0.0.1:19001"' in profile_text


def test_transport_registry_injects_udp_driver_into_controller(tmp_path):
    udp_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register(
        "udp",
        factory=lambda: udp_transport,
        port_provider=lambda: [],
    )
    registry.register(
        "serial",
        factory=FakeSerialTransport,
        port_provider=lambda: ["COM_REGISTRY"],
    )
    registry.register(
        "fake",
        factory=FakeSerialTransport,
        port_provider=lambda: ["FAKE_LOOPBACK"],
    )
    controller = SerialWorkbenchController(transport_registry=registry)
    profile_path = tmp_path / "udp-profile.json"

    assert controller.available_transport_modes() == ("udp", "serial", "fake")
    result = controller.connect_udp_result("127.0.0.1", 19003)

    assert result.ok
    assert result.value is not None
    assert result.value.port_name == "127.0.0.1:19003"
    assert controller.connect_udp("127.0.0.1", 19003)
    assert controller.send_text("udp-registry")

    controller.save_profile(profile_path, "udp-profile")

    profile_text = profile_path.read_text(encoding="utf-8")
    assert udp_transport.written == [b"udp-registry"]
    assert '"mode": "udp"' in profile_text
    assert '"portName": "127.0.0.1:19003"' in profile_text
