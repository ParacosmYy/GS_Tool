"""Protocol dispatcher for the Python Serial Station core."""

from __future__ import annotations

from typing import Any, Mapping

from embeddebug.serial_station.protocols.base import ProtocolEvent, SerialProtocol


class SerialDispatcher:
    """Own the active protocol parser and feed incoming bytes into it."""

    def __init__(self, protocol: SerialProtocol) -> None:
        self._protocol = protocol

    @property
    def protocol_name(self) -> str:
        return self._protocol.name

    def set_protocol(self, protocol: SerialProtocol) -> None:
        self._protocol = protocol
        self._protocol.reset()

    def build_command(self, command: str, params: Mapping[str, Any] | None = None) -> bytes:
        return self._protocol.build_command(command, params)

    def feed(self, data: bytes) -> list[ProtocolEvent]:
        return self._protocol.feed(data)
