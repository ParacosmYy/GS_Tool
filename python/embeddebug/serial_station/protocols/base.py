"""Protocol contracts for the Python Serial Station core."""

from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from typing import Any, Mapping


@dataclass(frozen=True)
class ProtocolEvent:
    """A protocol fact emitted by a streaming parser."""

    type: str
    protocol_name: str
    payload: dict[str, Any] = field(default_factory=dict)
    raw: bytes = b""


class SerialProtocol(ABC):
    """Streaming protocol contract shared by all Python protocols."""

    name: str

    @abstractmethod
    def build_command(self, command: str, params: Mapping[str, Any] | None = None) -> bytes:
        """Build bytes for a named command."""

    @abstractmethod
    def feed(self, data: bytes) -> list[ProtocolEvent]:
        """Feed bytes into the parser and return zero or more events."""

    @abstractmethod
    def reset(self) -> None:
        """Reset parser state."""
