"""Serialization helpers for protocol events."""

from __future__ import annotations

from typing import Any

from embeddebug.serial_station.protocols import ProtocolEvent


def event_to_record(event: ProtocolEvent) -> dict[str, Any]:
    return {
        "type": event.type,
        "protocolName": event.protocol_name,
        "payload": event.payload,
        "rawHex": event.raw.hex(),
    }


def event_from_record(record: dict[str, Any]) -> ProtocolEvent:
    return ProtocolEvent(
        type=str(record["type"]),
        protocol_name=str(record["protocolName"]),
        payload=dict(record.get("payload", {})),
        raw=bytes.fromhex(str(record.get("rawHex", ""))),
    )
