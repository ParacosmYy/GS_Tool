"""VOFA+ RawData protocol implementation."""

from __future__ import annotations

from typing import Any, Mapping

from embeddebug.serial_station.protocols.base import ProtocolEvent, SerialProtocol


class RawDataProtocol(SerialProtocol):
    """Pass through bytes without parsing."""

    name = "raw_data"

    def build_command(self, command: str, params: Mapping[str, Any] | None = None) -> bytes:
        if params and params.get("hex"):
            return bytes.fromhex(command)
        encoding = str(params.get("encoding", "utf-8")) if params else "utf-8"
        return command.encode(encoding)

    def feed(self, data: bytes) -> list[ProtocolEvent]:
        if not data:
            return []
        return [
            ProtocolEvent(
                type="frame",
                protocol_name=self.name,
                payload={
                    "format": self.name,
                    "text": data.decode("utf-8", errors="replace"),
                    "size": len(data),
                },
                raw=bytes(data),
            )
        ]

    def reset(self) -> None:
        return None
