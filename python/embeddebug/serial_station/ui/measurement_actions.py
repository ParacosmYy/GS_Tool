"""Measurement display actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.core import ChannelBatch


class MeasurementActionHost(Protocol):
    """Minimal main-window surface needed by measurement action handlers."""


def append_measurement_batch(host: MeasurementActionHost, batch: ChannelBatch) -> None:
    host._waveform_preview.update_batch(batch)
