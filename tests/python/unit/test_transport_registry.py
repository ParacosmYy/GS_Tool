from __future__ import annotations

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    TransportRegistry,
)


def test_transport_registry_exposes_default_modes_and_fake_transport():
    registry = TransportRegistry.with_defaults()

    assert registry.modes == ("fake", "serial")
    assert registry.available_ports("fake") == ("FAKE_LOOPBACK",)

    transport = registry.create("fake")
    assert isinstance(transport, FakeSerialTransport)
    assert transport.open(SerialPortConfig(port_name="FAKE_LOOPBACK"))


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
