"""Helpers for controller-owned measurement ring state."""

from __future__ import annotations

from embeddebug.serial_station.core import ChannelBatch, ChannelRingBuffer


def append_measurement_batch(
    ring: ChannelRingBuffer | None,
    batch: ChannelBatch,
) -> tuple[ChannelRingBuffer, ChannelBatch]:
    if ring is None or ring.latest().values.shape[1] != batch.values.shape[1]:
        ring = ChannelRingBuffer(
            capacity=4096,
            channel_count=batch.values.shape[1],
            channel_names=batch.channel_names,
            dt_ns=batch.dt_ns,
        )
    ring.append(batch)
    return ring, ring.latest()
