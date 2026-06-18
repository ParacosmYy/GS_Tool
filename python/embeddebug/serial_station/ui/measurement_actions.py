"""Measurement display actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.core import ChannelBatch


class MeasurementActionHost(Protocol):
    """Minimal main-window surface needed by measurement action handlers."""


def append_measurement_batch(host: MeasurementActionHost, batch: ChannelBatch) -> None:
    """热路径入口：通过 submit_batch 走 BatchAccumulator + RefreshThrottle 节流。

    Batch 7-2：controller 高频 measurement_batch 不再直接 update_batch（逐批次
    setData 卡 UI），而是 submit_batch 累积合并 + 60Hz 节流刷新。
    对齐 serial_station_architecture §5.4 VOFA+ parity：批量信号 + 定时刷新。
    """

    host._waveform_preview.submit_batch(batch)
