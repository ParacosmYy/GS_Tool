"""控制器辅助函数单元测试 — text_decode + measurement_buffer。

覆盖：decode_injected_text 转义序列展开、append_measurement_batch 环形缓冲区
自动初始化 + 通道数变化时重建 + latest 返回。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.controllers.measurement_buffer import append_measurement_batch
from embeddebug.serial_station.controllers.text_decode import decode_injected_text
from embeddebug.serial_station.core import ChannelBatch


def test_decode_injected_text_newline():
    assert decode_injected_text("hello\\nworld") == "hello\nworld"


def test_decode_injected_text_carriage_return():
    assert decode_injected_text("a\\rb") == "a\rb"


def test_decode_injected_text_tab():
    assert decode_injected_text("a\\tb") == "a\tb"


def test_decode_injected_text_combined():
    assert decode_injected_text("line1\\r\\nline2\\t") == "line1\r\nline2\t"


def test_decode_injected_text_no_escapes():
    assert decode_injected_text("plain text") == "plain text"


def _batch(channels: int = 1, names: tuple[str, ...] | None = None) -> ChannelBatch:
    if names is None:
        names = tuple(f"ch{i}" for i in range(channels))
    return ChannelBatch(
        channel_names=names,
        values=np.random.rand(10, channels).astype(np.float32),
    )


def test_append_measurement_batch_initializes_ring():
    """首次 append 自动创建 ring。"""
    ring, latest = append_measurement_batch(None, _batch(2))
    assert ring is not None
    assert latest.values.shape[1] == 2


def test_append_measurement_batch_returns_latest():
    """append 后 latest 返回累积数据（ring 合并多次 append）。"""
    ring, _ = append_measurement_batch(None, _batch(1))
    batch2 = _batch(1)
    ring, latest = append_measurement_batch(ring, batch2)
    # ring 累积两个 batch（各 10 行），latest 返回合并的 20 行。
    assert latest.values.shape == (20, 1)
    assert latest.channel_names == batch2.channel_names


def test_append_measurement_batch_channel_count_change_rebuilds():
    """通道数变化时重建 ring。"""
    ring, _ = append_measurement_batch(None, _batch(2, ("a", "b")))
    batch3 = _batch(3, ("x", "y", "z"))
    ring, latest = append_measurement_batch(ring, batch3)
    assert latest.channel_names == ("x", "y", "z")
    assert latest.values.shape[1] == 3
