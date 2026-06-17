"""Protocol registry for the Python Serial Station core."""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.protocols.base import SerialProtocol
from embeddebug.serial_station.protocols.fire_water import FireWaterProtocol
from embeddebug.serial_station.protocols.just_float import JustFloatProtocol
from embeddebug.serial_station.protocols.raw_data import RawDataProtocol

ProtocolFactory = Callable[[], SerialProtocol]


class SerialProtocolRegistry:
    """Factory registry for built-in and future protocol engines."""

    def __init__(self) -> None:
        self._factories: dict[str, ProtocolFactory] = {}

    def register(self, name: str, factory: ProtocolFactory) -> None:
        if not name:
            raise ValueError("protocol name must not be empty")
        self._factories[name] = factory

    def names(self) -> list[str]:
        return sorted(self._factories)

    def create(self, name: str) -> SerialProtocol:
        try:
            return self._factories[name]()
        except KeyError as exc:
            raise KeyError(f"unknown protocol: {name}") from exc


def create_default_registry() -> SerialProtocolRegistry:
    registry = SerialProtocolRegistry()
    registry.register(RawDataProtocol.name, RawDataProtocol)
    registry.register(FireWaterProtocol.name, FireWaterProtocol)
    registry.register(JustFloatProtocol.name, JustFloatProtocol)
    return registry
