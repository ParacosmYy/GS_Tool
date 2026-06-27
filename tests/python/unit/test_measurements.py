"""core/measurements 纯函数单元测试 — batch_from_measurement_events + 辅助。

覆盖：batch_from_measurement_events 空/单事件/多通道、_default_channel_names。
"""

from __future__ import annotations

import numpy as np

from embeddebug.serial_station.core.measurements import (
    ChannelBatch,
    ChannelRingBuffer,
    _default_channel_names,
    batch_from_measurement_events,
)
from embeddebug.serial_station.protocols.base import ProtocolEvent


def test_default_channel_names():
    names = _default_channel_names(3)
    assert names == ("ch1", "ch2", "ch3")


def test_default_channel_names_zero():
    assert _default_channel_names(0) == ()


def test_batch_from_empty_events():
    assert batch_from_measurement_events([]) is None


def test_batch_from_non_measurement_events():
    """非 measurement 类型事件 → None。"""
    events = [ProtocolEvent(type="byte", protocol_name="ascii")]
    assert batch_from_measurement_events(events) is None


def test_batch_from_single_measurement():
    event = ProtocolEvent(
        type="measurement",
        protocol_name="custom",
        payload={"values": [1.0, 2.0]},
    )
    batch = batch_from_measurement_events([event])
    assert batch is not None
    assert batch.values.shape == (1, 2)
    assert batch.values[0, 0] == 1.0
    assert batch.values[0, 1] == 2.0


def test_batch_from_multiple_measurements():
    events = [
        ProtocolEvent(type="measurement", protocol_name="x", payload={"values": [10.0]}),
        ProtocolEvent(type="measurement", protocol_name="x", payload={"values": [20.0]}),
    ]
    batch = batch_from_measurement_events(events)
    assert batch is not None
    assert batch.values.shape == (2, 1)
    assert batch.values[0, 0] == 10.0
    assert batch.values[1, 0] == 20.0


def test_batch_with_channel_names():
    event = ProtocolEvent(
        type="measurement",
        protocol_name="x",
        payload={"values": [1.0, 2.0], "channelNames": ["temp", "humidity"]},
    )
    batch = batch_from_measurement_events([event])
    assert batch is not None
    assert batch.channel_names == ("temp", "humidity")


def test_batch_custom_dt():
    event = ProtocolEvent(type="measurement", protocol_name="x", payload={"values": [1.0]})
    batch = batch_from_measurement_events([event], t0_ns=1000, dt_ns=500)
    assert batch is not None
    assert batch.t0_ns == 1000
    assert batch.dt_ns == 500


def test_batch_filters_non_measurement():
    """measurement 与非 measurement 混合时只取 measurement。"""
    events = [
        ProtocolEvent(type="byte", protocol_name="ascii"),
        ProtocolEvent(type="measurement", protocol_name="x", payload={"values": [42.0]}),
    ]
    batch = batch_from_measurement_events(events)
    assert batch is not None
    assert batch.values.shape == (1, 1)


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
