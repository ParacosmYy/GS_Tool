"""Converters between controller log entries and protocol events."""

from __future__ import annotations

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.protocols.base import ProtocolEvent


_KNOWN_DIRECTIONS = {"tx", "rx", "system", "error"}


def entry_from_event(event: ProtocolEvent) -> SerialWorkbenchLogEntry:
    text = str(event.payload.get("text", event.raw.decode("utf-8", errors="replace")))
    payload_direction = event.payload.get("direction")
    direction = payload_direction if payload_direction in _KNOWN_DIRECTIONS else _direction_from_type(event.type)
    return SerialWorkbenchLogEntry(direction=direction, text=text, raw=event.raw)


def event_from_entry(entry: SerialWorkbenchLogEntry, protocol_name: str) -> ProtocolEvent:
    event_type = "tx" if entry.direction == "tx" else "frame"
    return ProtocolEvent(
        type=event_type,
        protocol_name=protocol_name,
        payload={"text": entry.text, "direction": entry.direction},
        raw=entry.raw,
    )


def _direction_from_type(event_type: str) -> str:
    return "tx" if event_type == "tx" else "rx"
