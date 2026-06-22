"""RTT 协议契约单元测试 — 控制块布局 + 通道/配置。

覆盖：SEGGER 常量、control_block_layout 偏移计算、RttChannel/RttConfig 结构。
"""

from __future__ import annotations


from embeddebug.serial_station.rtt.protocol import (
    BUFFER_HEADER_SIZE,
    CB_ACID_OFFSET,
    CB_ACID_SIZE,
    CB_BUFFERS_OFFSET,
    CB_MAX_DOWN_BUFFERS_OFFSET,
    CB_MAX_UP_BUFFERS_OFFSET,
    SEGGER_RTT_CB_ID,
    SEGGER_RTT_CB_ID_BYTES,
    SEGGER_RTT_MAGIC,
    SEGGER_RTT_MAGIC_BYTES,
    RttChannel,
    RttConfig,
    control_block_layout,
)


def test_segger_magic():
    assert SEGGER_RTT_MAGIC == "RTT\0"
    assert SEGGER_RTT_MAGIC_BYTES == b"RTT\0"


def test_segger_cb_id():
    assert SEGGER_RTT_CB_ID == "SEGGER RTT"
    assert SEGGER_RTT_CB_ID_BYTES == b"SEGGER RTT"


def test_control_block_constants():
    assert CB_ACID_OFFSET == 0
    assert CB_ACID_SIZE == 16
    assert CB_MAX_UP_BUFFERS_OFFSET == 16
    assert CB_MAX_DOWN_BUFFERS_OFFSET == 20
    assert CB_BUFFERS_OFFSET == 24


def test_buffer_header_size():
    assert BUFFER_HEADER_SIZE == 16


def test_control_block_layout_basic():
    layout = control_block_layout(max_up=1, max_down=1)
    # acID + MaxUp + MaxDown + 1 up header + 1 down header
    assert len(layout) == 5
    assert layout[0] == ("acID", 0, 16)
    assert layout[1] == ("MaxNumUpBuffers", 16, 4)


def test_control_block_layout_offsets_increment():
    """buffer header offset 按 BUFFER_HEADER_SIZE 递增。"""
    layout = control_block_layout(max_up=3, max_down=0)
    up_headers = [item for item in layout if "aUp" in item[0]]
    assert len(up_headers) == 3
    # 每个 header 16 字节，从 offset 24 开始。
    assert up_headers[0][1] == CB_BUFFERS_OFFSET
    assert up_headers[1][1] == CB_BUFFERS_OFFSET + BUFFER_HEADER_SIZE


def test_control_block_layout_down_after_up():
    """down buffer headers 排在 up 之后。"""
    layout = control_block_layout(max_up=2, max_down=2)
    up_items = [item for item in layout if "aUp" in item[0]]
    down_items = [item for item in layout if "aDown" in item[0]]
    assert len(up_items) == 2
    assert len(down_items) == 2
    # down 的首个 offset 应大于 up 最后一个。
    assert down_items[0][1] > up_items[-1][1]


def test_control_block_layout_zero_buffers():
    layout = control_block_layout(max_up=0, max_down=0)
    assert len(layout) == 3  # 只有 acID + MaxUp + MaxDown


def test_rtt_channel():
    ch = RttChannel(name="terminal", buffer_size=1024, mode="up")
    assert ch.name == "terminal"
    assert ch.buffer_size == 1024
    assert ch.mode == "up"


def test_rtt_config_defaults():
    cfg = RttConfig(channels=(RttChannel("ch0", 256, "up"),))
    assert cfg.ram_base == 0x2000_0000
    assert len(cfg.channels) == 1


def test_rtt_config_custom_ram_base():
    cfg = RttConfig(channels=(), ram_base=0x1000_0000)
    assert cfg.ram_base == 0x1000_0000


# ---- Batch 153: RTT protocol 边界扩展 ----


def test_rtt_channel_is_frozen():
    """RttChannel 是 frozen dataclass，不可变。"""
    import pytest
    ch = RttChannel(name="ch", buffer_size=128, mode="up")
    with pytest.raises((AttributeError, TypeError)):
        ch.name = "other"


def test_rtt_config_is_frozen():
    """RttConfig 是 frozen dataclass，不可变。"""
    import pytest
    cfg = RttConfig(channels=())
    with pytest.raises((AttributeError, TypeError)):
        cfg.ram_base = 0x3000_0000


def test_rtt_config_empty_channels_tuple():
    """RttConfig 空 channels 元组合法。"""
    cfg = RttConfig(channels=())
    assert cfg.channels == ()
    assert len(cfg.channels) == 0


def test_rtt_config_multiple_channels_preserve_order():
    """多通道按定义顺序保留（索引即 SEGGER 通道号）。"""
    ch0 = RttChannel("up0", 256, "up")
    ch1 = RttChannel("down0", 128, "down")
    ch2 = RttChannel("up1", 512, "up")
    cfg = RttConfig(channels=(ch0, ch1, ch2))
    assert cfg.channels[0] is ch0
    assert cfg.channels[2] is ch2
    assert [c.name for c in cfg.channels] == ["up0", "down0", "up1"]


def test_rtt_channel_down_mode():
    """mode='down' 合法。"""
    ch = RttChannel(name="host", buffer_size=64, mode="down")
    assert ch.mode == "down"


def test_channel_index_returns_position():
    """channel_index 返回通道在元组中的位置。"""
    from embeddebug.serial_station.rtt.protocol import channel_index
    channels = (
        RttChannel("up0", 256, "up"),
        RttChannel("down0", 128, "down"),
        RttChannel("up1", 512, "up"),
    )
    assert channel_index(channels, "up0") == 0
    assert channel_index(channels, "down0") == 1
    assert channel_index(channels, "up1") == 2


def test_channel_index_unknown_name_raises_keyerror():
    """channel_index 对未知名抛 KeyError。"""
    import pytest
    from embeddebug.serial_station.rtt.protocol import channel_index
    channels = (RttChannel("up0", 256, "up"),)
    with pytest.raises(KeyError):
        channel_index(channels, "ghost")


def test_channel_index_empty_channels_raises_keyerror():
    """空 channels 元组时任何名都抛 KeyError。"""
    import pytest
    from embeddebug.serial_station.rtt.protocol import channel_index
    with pytest.raises(KeyError):
        channel_index((), "any")


def test_control_block_layout_down_only():
    """max_up=0, max_down=2 时只有 down headers。"""
    layout = control_block_layout(max_up=0, max_down=2)
    down_items = [item for item in layout if "aDown" in item[0]]
    assert len(down_items) == 2
    # down headers 从 CB_BUFFERS_OFFSET 开始
    assert down_items[0][1] == CB_BUFFERS_OFFSET
    assert down_items[1][1] == CB_BUFFERS_OFFSET + BUFFER_HEADER_SIZE


def test_control_block_layout_all_sizes_are_16():
    """所有 buffer header 的 size 字段都是 BUFFER_HEADER_SIZE=16。"""
    layout = control_block_layout(max_up=3, max_down=2)
    header_items = [item for item in layout if "header" in item[0]]
    for _name, _offset, size in header_items:
        assert size == BUFFER_HEADER_SIZE


def test_control_block_layout_acid_size_is_16():
    """acID 字段 size 固定 16（SEGGER 规范）。"""
    layout = control_block_layout(max_up=0, max_down=0)
    acid = layout[0]
    assert acid == ("acID", 0, CB_ACID_SIZE)


def test_control_block_layout_max_up_max_down_size_4():
    """MaxNumUpBuffers / MaxNumDownBuffers 各 4 字节（int32）。"""
    layout = control_block_layout(max_up=0, max_down=0)
    assert layout[1][2] == 4  # MaxNumUpBuffers size
    assert layout[2][2] == 4  # MaxNumDownBuffers size


def test_buffer_header_offset_constants():
    """buffer header 内部字段偏移常量。"""
    from embeddebug.serial_station.rtt.protocol import (
        BUFFER_FLAGS_OFFSET,
        BUFFER_RD_OFF_OFFSET,
        BUFFER_SIZE_OF_BUFFER_OFFSET,
        BUFFER_WR_OFF_OFFSET,
    )
    assert BUFFER_SIZE_OF_BUFFER_OFFSET == 0
    assert BUFFER_WR_OFF_OFFSET == 4
    assert BUFFER_RD_OFF_OFFSET == 8
    assert BUFFER_FLAGS_OFFSET == 12


def test_segger_magic_bytes_ascii_encoding():
    """SEGGER_RTT_MAGIC_BYTES 是 ASCII 编码的 magic 字符串。"""
    assert SEGGER_RTT_MAGIC.encode("ascii") == SEGGER_RTT_MAGIC_BYTES
    assert len(SEGGER_RTT_MAGIC_BYTES) == 4


def test_segger_cb_id_bytes_ascii_encoding():
    """SEGGER_RTT_CB_ID_BYTES 是 ASCII 编码的 CB ID 字符串。"""
    assert SEGGER_RTT_CB_ID.encode("ascii") == SEGGER_RTT_CB_ID_BYTES
    assert len(SEGGER_RTT_CB_ID_BYTES) == 10


def test_control_block_layout_large_buffers():
    """大量 buffer（32 up + 32 down）布局正确展开。"""
    layout = control_block_layout(max_up=32, max_down=32)
    up_items = [item for item in layout if "aUp" in item[0]]
    down_items = [item for item in layout if "aDown" in item[0]]
    assert len(up_items) == 32
    assert len(down_items) == 32
    # 最后一个 down header 的 offset
    expected_last_offset = CB_BUFFERS_OFFSET + (32 + 32 - 1) * BUFFER_HEADER_SIZE
    assert down_items[-1][1] == expected_last_offset
