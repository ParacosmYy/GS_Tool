from __future__ import annotations

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import entry_from_event, event_from_entry
from embeddebug.serial_station.protocols import ProtocolEvent


def test_entry_from_event_restores_known_payload_direction():
    event = ProtocolEvent(
        type="frame",
        protocol_name="raw_data",
        payload={"text": "port denied", "direction": "error"},
        raw=b"port denied",
    )

    entry = entry_from_event(event)

    assert entry.direction == "error"
    assert entry.text == "port denied"


def test_event_from_entry_preserves_diagnostic_direction_in_payload():
    event = event_from_entry(
        SerialWorkbenchLogEntry(direction="system", text="profile loaded", raw=b"profile loaded"),
        "raw_data",
    )

    assert event.type == "frame"
    assert event.payload["direction"] == "system"
