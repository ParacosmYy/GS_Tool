"""JSON Lines log service for Python Serial Station events."""

from __future__ import annotations

import json
from pathlib import Path

from embeddebug.serial_station.protocols import ProtocolEvent
from embeddebug.serial_station.services.event_codec import event_to_record


class SerialLogService:
    """Append protocol events to a UTF-8 JSON Lines log."""

    def __init__(self, path: str | Path) -> None:
        self._path = Path(path)

    def append(self, event: ProtocolEvent) -> None:
        self._path.parent.mkdir(parents=True, exist_ok=True)
        with self._path.open("a", encoding="utf-8", newline="\n") as handle:
            handle.write(json.dumps(event_to_record(event), ensure_ascii=False))
            handle.write("\n")
