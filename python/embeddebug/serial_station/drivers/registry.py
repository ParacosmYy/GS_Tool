"""Transport registry for pluggable Serial Station connection drivers."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from embeddebug.serial_station.drivers.base import SerialTransport
from embeddebug.serial_station.drivers.fake import FakeSerialTransport
from embeddebug.serial_station.drivers.qt_serial import QtSerialPortTransport


TransportFactory = Callable[[], SerialTransport]
PortProvider = Callable[[], list[str]]


@dataclass(frozen=True)
class TransportDriver:
    """Factory and discovery callbacks for one transport mode."""

    factory: TransportFactory
    port_provider: PortProvider


class TransportRegistry:
    """Register and create byte transports by stable mode name."""

    def __init__(self) -> None:
        self._drivers: dict[str, TransportDriver] = {}

    @classmethod
    def with_defaults(
        cls,
        serial_factory: TransportFactory | None = None,
        serial_port_provider: PortProvider | None = None,
    ) -> TransportRegistry:
        registry = cls()
        registry.register(
            "fake",
            factory=FakeSerialTransport,
            port_provider=lambda: ["FAKE_LOOPBACK"],
        )
        registry.register(
            "serial",
            factory=serial_factory or QtSerialPortTransport,
            port_provider=serial_port_provider or QtSerialPortTransport.available_ports,
        )
        return registry

    @property
    def modes(self) -> tuple[str, ...]:
        return tuple(self._drivers)

    def register(
        self,
        mode: str,
        factory: TransportFactory,
        port_provider: PortProvider | None = None,
    ) -> None:
        if not mode:
            raise ValueError("transport mode is required")
        self._drivers[mode] = TransportDriver(
            factory=factory,
            port_provider=port_provider or (lambda: []),
        )

    def create(self, mode: str) -> SerialTransport:
        return self._driver(mode).factory()

    def available_ports(self, mode: str) -> tuple[str, ...]:
        return tuple(self._driver(mode).port_provider())

    def _driver(self, mode: str) -> TransportDriver:
        try:
            return self._drivers[mode]
        except KeyError as exc:
            raise KeyError(f"unknown transport mode: {mode}") from exc
