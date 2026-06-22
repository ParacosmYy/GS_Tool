"""VOFA+ JustFloat protocol implementation."""

from __future__ import annotations

import struct
from typing import Any
from collections.abc import Mapping

from embeddebug.serial_station.protocols.base import ProtocolEvent, SerialProtocol


class JustFloatProtocol(SerialProtocol):
    """Parse little-endian float32 frames terminated by 00 00 80 7F."""

    name = "just_float"
    tail = b"\x00\x00\x80\x7f"

    def __init__(self) -> None:
        self._buffer = bytearray()
        self._frame_index = 0

    def build_command(self, command: str, params: Mapping[str, Any] | None = None) -> bytes:
        if params and params.get("hex"):
            return bytes.fromhex(command)
        encoding = str(params.get("encoding", "utf-8")) if params else "utf-8"
        return command.encode(encoding)

    def feed(self, data: bytes) -> list[ProtocolEvent]:
        if data:
            self._buffer.extend(data)

        events: list[ProtocolEvent] = []
        while True:
            tail_index = self._buffer.find(self.tail)
            if tail_index < 0:
                return events

            payload = bytes(self._buffer[:tail_index])
            raw = bytes(self._buffer[: tail_index + len(self.tail)])
            del self._buffer[: tail_index + len(self.tail)]
            events.append(self._parse_payload(payload, raw))

    def reset(self) -> None:
        self._buffer.clear()
        self._frame_index = 0

    def _parse_payload(self, payload: bytes, raw: bytes) -> ProtocolEvent:
        if len(payload) == 0 or len(payload) % 4 != 0:
            return ProtocolEvent(
                type="error",
                protocol_name=self.name,
                payload={
                    "format": self.name,
                    "reason": "invalid_payload_length",
                    "payloadSize": len(payload),
                },
                raw=raw,
            )

        values = list(struct.unpack(f"<{len(payload) // 4}f", payload))
        self._frame_index += 1
        return ProtocolEvent(
            type="measurement",
            protocol_name=self.name,
            payload={
                "format": self.name,
                "values": values,
                "channelCount": len(values),
                "frameIndex": self._frame_index,
                "channelNames": [f"ch{index + 1}" for index in range(len(values))],
            },
            raw=raw,
        )
