"""Measurement export service for Python Serial Station."""

from __future__ import annotations

import csv
from pathlib import Path
from typing import Iterable

from embeddebug.serial_station.protocols import ProtocolEvent
from embeddebug.shared import OperationResult


class SerialMeasurementExportService:
    """Export measurement events to CSV."""

    def export_csv_result(
        self,
        path: str | Path,
        events: Iterable[ProtocolEvent],
    ) -> OperationResult[Path]:
        try:
            self.export_csv(path, events)
        except OSError as exc:
            return OperationResult.failure("measurement_export_failed", str(exc))
        return OperationResult.success(Path(path))

    def export_csv(self, path: str | Path, events: Iterable[ProtocolEvent]) -> None:
        measurements = [event for event in events if event.type == "measurement"]
        output_path = Path(path)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        channel_names = self._channel_names(measurements)
        with output_path.open("w", encoding="utf-8", newline="") as handle:
            writer = csv.writer(handle)
            writer.writerow(["frameIndex", "protocol", *channel_names])
            for event in measurements:
                values = list(event.payload.get("values", []))
                writer.writerow(
                    [
                        str(event.payload.get("frameIndex", "")),
                        event.protocol_name,
                        *[str(values[index]) if index < len(values) else "" for index in range(len(channel_names))],
                    ]
                )

    @staticmethod
    def _channel_names(events: list[ProtocolEvent]) -> list[str]:
        for event in events:
            names = event.payload.get("channelNames")
            if isinstance(names, list) and names:
                return [str(name) for name in names]
        max_count = max((len(event.payload.get("values", [])) for event in events), default=0)
        return [f"ch{index + 1}" for index in range(max_count)]
