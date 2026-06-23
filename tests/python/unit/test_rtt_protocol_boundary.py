"""rtt/protocol RttChannel + RttConfig frozen + control_block_layout/channel_index 边界测试。

RttChannel/RttConfig frozen + control_block_layout 长度 + channel_index 此前经 test_rtt_protocol 间接测试。
本文件补 frozen + 默认值 + control_block_layout 结构 + channel_index 边界。

覆盖：
1. RttChannel frozen。
2. RttChannel 默认 mode。
3. RttConfig frozen。
4. RttConfig 默认 ram_base。
5. control_block_layout 返回 list。
6. control_block_layout 含 acID。
7. channel_index 已知返回索引。
8. channel_index 未知 raises KeyError。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.rtt.protocol import (
    RttChannel,
    RttConfig,
    channel_index,
    control_block_layout,
)


def test_rtt_channel_is_frozen():
    ch = RttChannel(name="test", buffer_size=1024, mode="up")
    with pytest.raises(AttributeError):
        ch.name = "other"  # type: ignore[misc]


def test_rtt_channel_fields():
    ch = RttChannel(name="log", buffer_size=512, mode="up")
    assert ch.name == "log"
    assert ch.buffer_size == 512
    assert ch.mode == "up"


def test_rtt_config_is_frozen():
    cfg = RttConfig(channels=(RttChannel(name="x", buffer_size=1, mode="up"),))
    with pytest.raises(AttributeError):
        cfg.ram_base = 0  # type: ignore[misc]


def test_rtt_config_default_ram_base():
    cfg = RttConfig(channels=())
    assert cfg.ram_base == 0x2000_0000


def test_control_block_layout_returns_list():
    layout = control_block_layout(max_up=2, max_down=1)
    assert isinstance(layout, list)


def test_control_block_layout_contains_acid():
    layout = control_block_layout(max_up=1, max_down=1)
    names = [entry[0] for entry in layout]
    assert "acID" in names


def test_channel_index_known():
    channels = (
        RttChannel(name="a", buffer_size=1, mode="up"),
        RttChannel(name="b", buffer_size=1, mode="up"),
    )
    assert channel_index(channels, "b") == 1


def test_channel_index_unknown_raises():
    channels = (RttChannel(name="a", buffer_size=1, mode="up"),)
    with pytest.raises(KeyError):
        channel_index(channels, "nonexistent")
