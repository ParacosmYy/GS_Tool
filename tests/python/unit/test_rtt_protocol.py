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
