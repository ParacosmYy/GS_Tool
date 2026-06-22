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
