"""Transport contracts for the Python Serial Station lane."""

from __future__ import annotations

from abc import ABC, abstractmethod
from collections.abc import Callable
from dataclasses import dataclass


BytesCallback = Callable[[bytes], None]
ErrorCallback = Callable[[str], None]


@dataclass(frozen=True)
class SerialPortConfig:
    """Basic serial port configuration."""

    port_name: str
    baud_rate: int = 115200
    data_bits: int = 8
    parity: str = "none"
    stop_bits: str = "1"
    flow_control: str = "none"


class SerialTransport(ABC):
    """Minimal byte transport interface for serial station workers/controllers."""

    @property
    @abstractmethod
    def config(self) -> SerialPortConfig | None:
        """Return the active or most recent serial configuration."""

    @property
    @abstractmethod
    def is_open(self) -> bool:
        """Return whether the transport is open."""

    @abstractmethod
    def open(self, config: SerialPortConfig) -> bool:
        """Open the transport with the supplied configuration."""

    @abstractmethod
    def close(self) -> None:
        """Close the transport."""

    @abstractmethod
    def write(self, data: bytes) -> int:
        """Write bytes and return the accepted byte count."""

    @abstractmethod
    def on_bytes_received(self, callback: BytesCallback) -> None:
        """Register a bytes-received callback."""

    @abstractmethod
    def on_error(self, callback: ErrorCallback) -> None:
        """Register an error callback."""
