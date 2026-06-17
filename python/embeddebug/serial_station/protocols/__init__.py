"""Serial Station protocols for the Python/PyQt lane."""

from embeddebug.serial_station.protocols.base import ProtocolEvent, SerialProtocol
from embeddebug.serial_station.protocols.fire_water import FireWaterProtocol
from embeddebug.serial_station.protocols.just_float import JustFloatProtocol
from embeddebug.serial_station.protocols.raw_data import RawDataProtocol
from embeddebug.serial_station.protocols.registry import (
    SerialProtocolRegistry,
    create_default_registry,
)

__all__ = [
    "FireWaterProtocol",
    "JustFloatProtocol",
    "ProtocolEvent",
    "RawDataProtocol",
    "SerialProtocol",
    "SerialProtocolRegistry",
    "create_default_registry",
]
