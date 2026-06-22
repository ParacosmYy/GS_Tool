"""BLE 帧编解码单元测试 — 编码/解码 round-trip + 流式分片。

覆盖：encode/encode_frame 结构、feed 流式解码、分片重组、
reset 清空、decode 静态方法、BleFrameEvent 属性。
"""

from __future__ import annotations

from embeddebug.serial_station.ble.codec import (
    BleFrameCodec,
    BleFrameEvent,
    FRAME_NOTIFY,
    FRAME_READ_RESPONSE,
    FRAME_WRITE,
)


def test_encode_notify_structure():
    event = BleFrameEvent(FRAME_NOTIFY, handle=0x0010, value=b"\x01\x02")
    encoded = BleFrameCodec.encode(event)
    assert encoded[0] == FRAME_NOTIFY
    assert encoded[1] == 0x10  # handle low byte
    assert encoded[2] == 0x00  # handle high byte
    assert encoded[3] == 2  # value length
    assert encoded[4:] == b"\x01\x02"


def test_encode_frame_convenience():
    """encode_frame 等价于 encode(BleFrameEvent(...))。"""
    encoded = BleFrameCodec.encode_frame(FRAME_WRITE, 0x0005, b"\xAA")
    event = BleFrameEvent(FRAME_WRITE, 0x0005, b"\xAA")
    assert encoded == BleFrameCodec.encode(event)


def test_feed_complete_frame():
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 0x0010, b"\x01\x02\x03")
    events = codec.feed(frame)
    assert len(events) == 1
    assert events[0].type == FRAME_NOTIFY
    assert events[0].handle == 0x0010
    assert events[0].value == b"\x01\x02\x03"


def test_feed_multiple_frames():
    codec = BleFrameCodec()
    f1 = BleFrameCodec.encode_frame(FRAME_NOTIFY, 1, b"\x01")
    f2 = BleFrameCodec.encode_frame(FRAME_WRITE, 2, b"\x02\x03")
    events = codec.feed(f1 + f2)
    assert len(events) == 2
    assert events[0].handle == 1
    assert events[1].handle == 2


def test_feed_partial_frame_buffers():
    """分片喂入重组完整帧。"""
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 0x0042, b"hello")
    # 喂入前 3 字节（不足 header）
    assert codec.feed(frame[:3]) == []
    # 喂入剩余
    events = codec.feed(frame[3:])
    assert len(events) == 1
    assert events[0].value == b"hello"


def test_feed_split_across_value():
    """分片在 value 中间断开。"""
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_READ_RESPONSE, 0x0100, b"\xAA\xBB\xCC")
    mid = 5  # header(4) + 1 byte of value
    assert codec.feed(frame[:mid]) == []
    events = codec.feed(frame[mid:])
    assert len(events) == 1
    assert events[0].value == b"\xAA\xBB\xCC"


def test_reset_clears_buffer():
    codec = BleFrameCodec()
    codec.feed(b"\x01\x02")  # 部分数据
    codec.reset()
    assert codec.feed(b"") == []  # buffer 已清空


def test_decode_static():
    """decode 静态方法等价于 feed。"""
    frame = BleFrameCodec.encode_frame(FRAME_WRITE, 10, b"\xFF")
    events = BleFrameCodec.decode(frame)
    assert len(events) == 1
    assert events[0].is_write is True


def test_event_is_properties():
    notify = BleFrameEvent(FRAME_NOTIFY, 0, b"")
    write = BleFrameEvent(FRAME_WRITE, 0, b"")
    read_resp = BleFrameEvent(FRAME_READ_RESPONSE, 0, b"")
    assert notify.is_notify is True
    assert write.is_write is True
    assert read_resp.is_read_response is True
    assert notify.is_write is False


def test_encode_value_truncated_to_255():
    """value 超过 255 字节截断。"""
    long_value = b"\x00" * 300
    event = BleFrameEvent(FRAME_NOTIFY, 0, long_value)
    encoded = BleFrameCodec.encode(event)
    assert encoded[3] == 255  # length capped
    assert len(encoded) == 4 + 255


# ---- Batch 145: BLE codec 边界扩展 ----


def test_encode_handle_masked_to_16_bits():
    """handle > 0xFFFF 被 & 0xFFFF 截断。"""
    event = BleFrameEvent(FRAME_NOTIFY, 0x12345, b"")
    encoded = BleFrameCodec.encode(event)
    # handle 低字节 = 0x45, 高字节 = 0x23
    assert encoded[1] == 0x45
    assert encoded[2] == 0x23


def test_encode_frame_type_masked_to_8_bits():
    """frame_type > 0xFF 被 & 0xFF 截断。"""
    encoded = BleFrameCodec.encode_frame(0x101, 0, b"")
    assert encoded[0] == 0x01  # 0x101 & 0xFF = 0x01


def test_feed_empty_data_returns_empty():
    """feed(b"") 不产出事件。"""
    assert BleFrameCodec().feed(b"") == []


def test_feed_partial_header_buffers():
    """< 4 字节时缓冲，不产出。"""
    codec = BleFrameCodec()
    assert codec.feed(b"\x01\x02") == []  # 仅 2 字节
    # 补齐 header(2 字节) + 1 字节 value
    events = codec.feed(b"\x00\x01\x42")
    assert len(events) == 1
    assert events[0].value == b"\x42"  # 'B'


def test_feed_unknown_frame_type():
    """type=0xFF 是未知类型，is_notify/is_write/is_read_response 全 False。"""
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(0xFF, 0, b"x")
    events = codec.feed(frame)
    assert len(events) == 1
    assert events[0].is_notify is False
    assert events[0].is_write is False
    assert events[0].is_read_response is False


def test_feed_zero_length_value():
    """length=0 的帧 value 为空 bytes。"""
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 5, b"")
    events = codec.feed(frame)
    assert len(events) == 1
    assert events[0].value == b""


def test_reset_after_complete_frame_clears_buffer():
    """完整帧消费后 buffer 为空；reset 不影响。"""
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 1, b"x")
    codec.feed(frame)
    codec.reset()
    assert codec.feed(b"") == []


def test_ble_frame_event_is_frozen():
    """BleFrameEvent 是 frozen dataclass。"""
    import pytest
    e = BleFrameEvent(FRAME_NOTIFY, 0, b"")
    with pytest.raises((AttributeError, TypeError)):
        e.handle = 99


def test_encode_frame_default_value_empty():
    """encode_frame 不传 value 时默认 b''。"""
    encoded = BleFrameCodec.encode_frame(FRAME_WRITE, 10)
    assert encoded[3] == 0  # length=0
    assert len(encoded) == 4


def test_feed_handles_big_endian_handle():
    """handle 用 little-endian 编码（低字节在前）。"""
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 0xBEEF, b"x")
    events = codec.feed(frame)
    assert events[0].handle == 0xBEEF


def test_feed_garbage_data_still_parses():
    """任意 4+ 字节数据都会被解析（不校验 type 合法性）。"""
    codec = BleFrameCodec()
    # type=0xAA, handle=0x0000, length=1, value=0xFF
    events = codec.feed(bytes([0xAA, 0x00, 0x00, 0x01, 0xFF]))
    assert len(events) == 1
    assert events[0].type == 0xAA
    assert events[0].value == b"\xFF"
