"""Converters between controller log entries and protocol events."""

from __future__ import annotations

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.protocols import ProtocolEvent


def entry_from_event(event: ProtocolEvent) -> SerialWorkbenchLogEntry:
    text = str(event.payload.get("text", event.raw.decode("utf-8", errors="replace")))
    direction = "tx" if event.type == "tx" else "rx"
    return SerialWorkbenchLogEntry(direction=direction, text=text, raw=event.raw)


def event_from_entry(entry: SerialWorkbenchLogEntry, protocol_name: str) -> ProtocolEvent:
    event_type = "tx" if entry.direction == "tx" else "frame"
    return ProtocolEvent(
        type=event_type,
        protocol_name=protocol_name,
        payload={"text": entry.text, "direction": entry.direction},
        raw=entry.raw,
    )
