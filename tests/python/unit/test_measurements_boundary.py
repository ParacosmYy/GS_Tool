"""core/measurements 边界扩展单元测试。

补强 test_measurements.py / test_measurement_ring_buffer.py 未直接断言的边角：
- ChannelBatch.__post_init__：1D/3D values ValueError + 列数不匹配 ValueError + frozen。
- ChannelRingBuffer.__init__：capacity=0/channel_count=0 ValueError + 默认通道名。
- ChannelRingBuffer.latest：空缓冲区 + latest(0) + latest(None) + latest(count>size) clamp。
- ChannelRingBuffer.append：通道数变更 ValueError + dt_ns 更新。
- _payload_values：非序列/str/bytes/bytearray ValueError。
"""

from __future__ import annotations

import numpy as np
import pytest

from embeddebug.serial_station.core.measurements import (
    ChannelBatch,
    ChannelRingBuffer,
    _payload_values,
)


# ── ChannelBatch.__post_init__ 验证 ──────────────────────────────────────


def test_channel_batch_1d_values_raises():
    """1D values（非 2D 矩阵）→ ValueError。"""

    with pytest.raises(ValueError, match="2D"):
        ChannelBatch(channel_names=("a",), values=np.array([1.0, 2.0, 3.0]))


def test_channel_batch_3d_values_raises():
    """3D values → ValueError。"""

    with pytest.raises(ValueError, match="2D"):
        ChannelBatch(channel_names=("a",), values=np.zeros((2, 2, 2)))


def test_channel_batch_column_mismatch_raises():
    """values 列数 != channel_names 长度 → ValueError。"""

    with pytest.raises(ValueError, match="channel count"):
        ChannelBatch(channel_names=("a", "b"), values=np.array([[1.0]]))


def test_channel_batch_frozen():
    """ChannelBatch 是 frozen（不可变）。"""

    batch = ChannelBatch(channel_names=("a",), values=np.array([[1.0]]))
    with pytest.raises((AttributeError, Exception)):
        batch.t0_ns = 999  # type: ignore[misc]


def test_channel_batch_coerces_to_float32():
    """values 自动转为 float32（即使传 int/float64）。"""

    batch = ChannelBatch(channel_names=("a",), values=np.array([[1]], dtype=np.int32))
    assert batch.values.dtype == np.float32


def test_channel_batch_defaults():
    """ChannelBatch 默认 t0_ns=0 / dt_ns=1。"""

    batch = ChannelBatch(channel_names=("a",), values=np.array([[1.0]]))
    assert batch.t0_ns == 0
    assert batch.dt_ns == 1


# ── ChannelRingBuffer.__init__ 验证 ──────────────────────────────────────


def test_ring_buffer_capacity_zero_raises():
    """capacity=0 → ValueError。"""

    with pytest.raises(ValueError, match="capacity"):
        ChannelRingBuffer(capacity=0, channel_count=2)


def test_ring_buffer_negative_capacity_raises():
    """capacity=-1 → ValueError。"""

    with pytest.raises(ValueError, match="capacity"):
        ChannelRingBuffer(capacity=-1, channel_count=2)


def test_ring_buffer_channel_count_zero_raises():
    """channel_count=0 → ValueError。"""

    with pytest.raises(ValueError, match="channel_count"):
        ChannelRingBuffer(capacity=10, channel_count=0)


def test_ring_buffer_default_channel_names():
    """无 channel_names → 默认 ch1/ch2/...。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=3)
    latest = ring.latest()
    assert latest.channel_names == ("ch1", "ch2", "ch3")


def test_ring_buffer_custom_channel_names():
    """显式 channel_names 覆盖默认。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=2, channel_names=("volt", "temp"))
    latest = ring.latest()
    assert latest.channel_names == ("volt", "temp")


def test_ring_buffer_custom_dt_ns():
    """自定义 dt_ns 默认值。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=1, dt_ns=1000)
    latest = ring.latest()
    assert latest.dt_ns == 1000


# ── ChannelRingBuffer.latest 边界 ────────────────────────────────────────


def test_ring_buffer_latest_empty_returns_empty_batch():
    """空缓冲区 latest() → 0 行空 batch（不抛）。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=2)
    latest = ring.latest()
    assert latest.values.shape == (0, 2)
    assert latest.t0_ns == 0


def test_ring_buffer_latest_zero_count_returns_empty():
    """latest(0) → 0 行（clamp min(0, size)=0）。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=1)
    ring.append(ChannelBatch(("a",), np.array([[1.0]])))
    latest = ring.latest(0)
    assert latest.values.shape == (0, 1)


def test_ring_buffer_latest_count_exceeds_size_clamps():
    """latest(100) 当 size=1 → clamp 到 1 行。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=1)
    ring.append(ChannelBatch(("a",), np.array([[42.0]])))
    latest = ring.latest(100)
    assert latest.values.shape == (1, 1)


def test_ring_buffer_latest_none_returns_all():
    """latest(None) → 返回全部已缓冲样本。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=1)
    ring.append(ChannelBatch(("a",), np.array([[1.0], [2.0]])))
    latest = ring.latest(None)
    assert latest.values.shape == (2, 1)


# ── ChannelRingBuffer.append 边界 ────────────────────────────────────────


def test_ring_buffer_append_channel_count_mismatch_raises():
    """append 的 batch 通道数与 buffer 不一致 → ValueError。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=2)
    with pytest.raises(ValueError, match="channel count"):
        ring.append(ChannelBatch(("a",), np.array([[1.0]])))


def test_ring_buffer_append_updates_channel_names():
    """append 的 batch 含 channel_names → buffer 更新通道名。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=2, channel_names=("old1", "old2"))
    ring.append(ChannelBatch(("new1", "new2"), np.array([[1.0, 2.0]])))
    latest = ring.latest()
    assert latest.channel_names == ("new1", "new2")


def test_ring_buffer_append_updates_dt_ns():
    """append 的 batch dt_ns → buffer 更新 dt_ns。"""

    ring = ChannelRingBuffer(capacity=5, channel_count=1, dt_ns=10)
    ring.append(ChannelBatch(("a",), np.array([[1.0]]), dt_ns=500))
    latest = ring.latest()
    assert latest.dt_ns == 500


def test_ring_buffer_size_caps_at_capacity():
    """size 在达到 capacity 后不再增长。"""

    ring = ChannelRingBuffer(capacity=3, channel_count=1)
    for i in range(5):
        ring.append(ChannelBatch(("a",), np.array([[float(i)]])))
    assert ring.size == 3  # 不超 capacity


# ── _payload_values 私有 helper ──────────────────────────────────────────


def test_payload_values_valid_list():
    """合法 list → float list。"""

    assert _payload_values({"values": [1, 2.5, 3]}) == [1.0, 2.5, 3.0]


def test_payload_values_empty_list():
    """空 list → 空 list。"""

    assert _payload_values({"values": []}) == []


def test_payload_values_missing_values_key():
    """缺 values key → 默认空 list。"""

    assert _payload_values({}) == []


def test_payload_values_string_raises():
    """str 不是合法 values 序列 → ValueError。"""

    with pytest.raises(ValueError, match="sequence"):
        _payload_values({"values": "abc"})


def test_payload_values_bytes_raises():
    """bytes 不是合法 values 序列 → ValueError。"""

    with pytest.raises(ValueError, match="sequence"):
        _payload_values({"values": b"\x01\x02"})
