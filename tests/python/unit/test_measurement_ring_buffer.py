from __future__ import annotations

import numpy as np

from embeddebug.serial_station.core.measurements import (
    ChannelBatch,
    ChannelRingBuffer,
    batch_from_measurement_events,
)
from embeddebug.serial_station.protocols import ProtocolEvent


def test_channel_batch_from_measurement_events_uses_float32_matrix():
    events = [
        ProtocolEvent(
            type="measurement",
            protocol_name="fire_water",
            payload={
                "frameIndex": 1,
                "channelNames": ["temp", "volt"],
                "values": [24.5, 3.3],
            },
            raw=b"24.5,3.3",
        ),
        ProtocolEvent(
            type="frame",
            protocol_name="raw_data",
            payload={"text": "ignored"},
            raw=b"ignored",
        ),
    ]

    batch = batch_from_measurement_events(events, t0_ns=100, dt_ns=1_000)

    assert isinstance(batch, ChannelBatch)
    assert batch.channel_names == ("temp", "volt")
    assert batch.values.dtype == np.float32
    assert batch.values.shape == (1, 2)
    assert batch.values.tolist() == [[24.5, 3.299999952316284]]
    assert batch.t0_ns == 100
    assert batch.dt_ns == 1_000


def test_ring_buffer_wraps_and_returns_latest_samples_in_order():
    ring = ChannelRingBuffer(capacity=3, channel_count=2)
    first = ChannelBatch(
        channel_names=("a", "b"),
        values=np.array([[1, 10], [2, 20]], dtype=np.float32),
        t0_ns=0,
        dt_ns=1,
    )
    second = ChannelBatch(
        channel_names=("a", "b"),
        values=np.array([[3, 30], [4, 40]], dtype=np.float32),
        t0_ns=2,
        dt_ns=1,
    )

    ring.append(first)
    ring.append(second)
    latest = ring.latest(3)

    assert latest.channel_names == ("a", "b")
    assert latest.values.tolist() == [[2.0, 20.0], [3.0, 30.0], [4.0, 40.0]]
    assert latest.t0_ns == 1
    assert latest.dt_ns == 1
    assert ring.size == 3
