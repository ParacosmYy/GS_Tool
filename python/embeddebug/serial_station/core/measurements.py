"""Measurement batches and fixed-size channel buffers."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

import numpy as np

from embeddebug.serial_station.protocols.base import ProtocolEvent


@dataclass(frozen=True)
class ChannelBatch:
    """A dense sample batch with one column per channel."""

    channel_names: tuple[str, ...]
    values: np.ndarray
    t0_ns: int = 0
    dt_ns: int = 1

    def __post_init__(self) -> None:
        values = np.asarray(self.values, dtype=np.float32)
        if values.ndim != 2:
            raise ValueError("channel values must be a 2D matrix")
        if values.shape[1] != len(self.channel_names):
            raise ValueError("channel count must match matrix columns")
        object.__setattr__(self, "values", values)


class ChannelRingBuffer:
    """Fixed-capacity ring buffer for equally spaced channel samples."""

    def __init__(
        self,
        capacity: int,
        channel_count: int,
        channel_names: Sequence[str] | None = None,
        dt_ns: int = 1,
    ) -> None:
        if capacity <= 0:
            raise ValueError("capacity must be positive")
        if channel_count <= 0:
            raise ValueError("channel_count must be positive")
        self._values = np.empty((capacity, channel_count), dtype=np.float32)
        self._write_index = 0
        self._size = 0
        self._total_samples = 0
        self._channel_names = tuple(channel_names or _default_channel_names(channel_count))
        self._dt_ns = dt_ns

    @property
    def size(self) -> int:
        return self._size

    def append(self, batch: ChannelBatch) -> None:
        if batch.values.shape[1] != self._values.shape[1]:
            raise ValueError("batch channel count changed")
        if batch.channel_names:
            self._channel_names = batch.channel_names
        self._dt_ns = batch.dt_ns
        for row in batch.values:
            self._values[self._write_index] = row
            self._write_index = (self._write_index + 1) % self._values.shape[0]
            self._size = min(self._size + 1, self._values.shape[0])
            self._total_samples += 1

    def latest(self, count: int | None = None) -> ChannelBatch:
        if self._size == 0:
            return ChannelBatch(
                channel_names=self._channel_names,
                values=np.empty((0, self._values.shape[1]), dtype=np.float32),
                t0_ns=0,
                dt_ns=self._dt_ns,
            )
        sample_count = self._size if count is None else min(max(count, 0), self._size)
        start = (self._write_index - sample_count) % self._values.shape[0]
        if start + sample_count <= self._values.shape[0]:
            values = self._values[start : start + sample_count].copy()
        else:
            first = self._values[start:]
            second = self._values[: (start + sample_count) % self._values.shape[0]]
            values = np.vstack((first, second)).astype(np.float32, copy=False)
        first_sample_index = self._total_samples - sample_count
        return ChannelBatch(
            channel_names=self._channel_names,
            values=values,
            t0_ns=first_sample_index * self._dt_ns,
            dt_ns=self._dt_ns,
        )


def batch_from_measurement_events(
    events: Sequence[ProtocolEvent],
    t0_ns: int = 0,
    dt_ns: int = 1,
) -> ChannelBatch | None:
    """Convert measurement protocol events into a dense float32 batch."""

    measurement_events = [event for event in events if event.type == "measurement"]
    if not measurement_events:
        return None

    first_payload = measurement_events[0].payload
    first_values = _payload_values(first_payload)
    channel_names = tuple(
        str(name)
        for name in first_payload.get(
            "channelNames",
            _default_channel_names(len(first_values)),
        )
    )
    rows: list[list[float]] = []
    for event in measurement_events:
        values = _payload_values(event.payload)
        row = [float("nan")] * len(channel_names)
        for index, value in enumerate(values[: len(channel_names)]):
            row[index] = value
        rows.append(row)

    return ChannelBatch(
        channel_names=channel_names,
        values=np.asarray(rows, dtype=np.float32),
        t0_ns=t0_ns,
        dt_ns=dt_ns,
    )


def _payload_values(payload: dict[str, object]) -> list[float]:
    raw_values = payload.get("values", [])
    if not isinstance(raw_values, Sequence) or isinstance(raw_values, (str, bytes, bytearray)):
        raise ValueError("measurement payload values must be a sequence")
    return [float(value) for value in raw_values]


def _default_channel_names(count: int) -> tuple[str, ...]:
    return tuple(f"ch{index + 1}" for index in range(count))
