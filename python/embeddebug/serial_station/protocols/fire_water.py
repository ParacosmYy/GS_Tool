"""VOFA+ FireWater line protocol implementation."""

from __future__ import annotations

import math
from typing import Any, Mapping

from embeddebug.serial_station.protocols.base import ProtocolEvent, SerialProtocol


class FireWaterProtocol(SerialProtocol):
    """Parse newline-delimited text frames into ordered channel samples."""

    name = "fire_water"

    def __init__(self, delimiter: str = ",", max_channels: int = 32) -> None:
        self._delimiter = delimiter
        self._max_channels = max_channels
        self._buffer = bytearray()
        self._frame_index = 0
        self._channel_names: list[str] = []

    def build_command(self, command: str, params: Mapping[str, Any] | None = None) -> bytes:
        line_ending = str(params.get("line_ending", "\n")) if params else "\n"
        return f"{command}{line_ending}".encode("utf-8")

    def feed(self, data: bytes) -> list[ProtocolEvent]:
        if data:
            self._buffer.extend(data)

        events: list[ProtocolEvent] = []
        while True:
            line = self._take_line()
            if line is None:
                return events
            event = self._parse_line(line)
            if event is not None:
                events.append(event)

    def reset(self) -> None:
        self._buffer.clear()
        self._frame_index = 0
        self._channel_names = []

    def _take_line(self) -> bytes | None:
        for index, value in enumerate(self._buffer):
            if value in (0x0A, 0x0D):
                raw_line = bytes(self._buffer[:index])
                end = index + 1
                if end < len(self._buffer) and self._buffer[end] in (0x0A, 0x0D):
                    end += 1
                del self._buffer[:end]
                return raw_line
        return None

    def _parse_line(self, raw_line: bytes) -> ProtocolEvent | None:
        text = raw_line.decode("utf-8", errors="replace").strip()
        if not text:
            return None

        body = text.split(":", 1)[1] if ":" in text else text
        fields = [item.strip() for item in body.split(self._delimiter)]
        if len(fields) > self._max_channels:
            return ProtocolEvent(
                type="error",
                protocol_name=self.name,
                payload={
                    "format": self.name,
                    "reason": "too_many_channels",
                    "channelCount": len(fields),
                    "maxChannels": self._max_channels,
                },
                raw=raw_line,
            )

        values: list[float] = []
        for field in fields:
            try:
                values.append(float(field))
            except ValueError:
                if self._looks_like_header(fields):
                    self._channel_names = fields[: self._max_channels]
                    return ProtocolEvent(
                        type="header",
                        protocol_name=self.name,
                        payload={
                            "format": self.name,
                            "channelNames": list(self._channel_names),
                            "channelCount": len(self._channel_names),
                        },
                        raw=raw_line,
                    )
                values.append(math.nan)

        self._frame_index += 1
        names = self._channel_names[: len(values)]
        if len(names) < len(values):
            names.extend(f"ch{index + 1}" for index in range(len(names), len(values)))

        return ProtocolEvent(
            type="measurement",
            protocol_name=self.name,
            payload={
                "format": self.name,
                "values": values,
                "channelCount": len(values),
                "frameIndex": self._frame_index,
                "channelNames": names,
            },
            raw=raw_line,
        )

    @staticmethod
    def _looks_like_header(fields: list[str]) -> bool:
        return all(field and not _is_float(field) for field in fields)


def _is_float(value: str) -> bool:
    try:
        float(value)
    except ValueError:
        return False
    return True
