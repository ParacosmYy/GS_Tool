"""Replay service for Python Serial Station JSON Lines logs."""

from __future__ import annotations

import json
from pathlib import Path

from embeddebug.serial_station.protocols.base import ProtocolEvent
from embeddebug.serial_station.services.event_codec import event_from_record
from embeddebug.shared import OperationResult


class SerialReplayService:
    """Load protocol events from a JSON Lines log."""

    def load_events_result(self, path: str | Path) -> OperationResult[list[ProtocolEvent]]:
        try:
            return OperationResult.success(self.load_events(path))
        except (OSError, json.JSONDecodeError, TypeError, ValueError) as exc:
            return OperationResult.failure("replay_load_failed", str(exc))

    def load_events(self, path: str | Path) -> list[ProtocolEvent]:
        events: list[ProtocolEvent] = []
        for line in Path(path).read_text(encoding="utf-8").splitlines():
            if not line.strip():
                continue
            events.append(event_from_record(json.loads(line)))
        return events
