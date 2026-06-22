"""CanFrameCodec SLCAN 编解码边界测试。

test_can.py 已覆盖基础往返/流式/多帧/unknown_prefix；本文件补错误路径全分支 +
PROTOCOL_NAME + reset + encode 格式细节 + CAN-FD。

覆盖：
1. PROTOCOL_NAME 常量 + name 属性。
2. encode 标准帧格式（t 前缀 + 3 位 ID + DLC + data + \r）。
3. encode 扩展帧格式（T 前缀 + 8 位 ID）。
4. _parse 错误全分支：empty_frame / unknown_prefix / frame_too_short /
   invalid_hex / dlc_exceeds_limit / data_length_mismatch / invalid_data_hex。
5. feed 空数据不崩溃（返回空列表）。
6. reset 清空缓冲区 + 帧计数。
7. CAN-FD 扩展帧 dlc>8 编码/解码（is_fd True）。
"""

from __future__ import annotations

from embeddebug.serial_station.can.codec import CanFrameCodec, PROTOCOL_NAME
from embeddebug.serial_station.can.frame import CanFrame, CanId


# ── 常量/属性 ─────────────────────────────────────────────────────
def test_protocol_name_constant():
    assert PROTOCOL_NAME == "can_slcan"


def test_codec_name_attribute_matches():
    codec = CanFrameCodec()
    assert codec.name == PROTOCOL_NAME


# ── encode 格式 ───────────────────────────────────────────────────
def test_encode_standard_frame_format():
    """标准帧：t + 3位ID(大写hex) + DLC + data(大写hex) + \\r。"""

    codec = CanFrameCodec()
    encoded = codec.encode(CanFrame(can_id=CanId(0x123), data=b"\xAB\xCD"))
    # t1232ABCD\r
    assert encoded == b"t1232ABCD\r"


def test_encode_extended_frame_format():
    """扩展帧：T + 8位ID + DLC + data + \\r。"""

    codec = CanFrameCodec()
    encoded = codec.encode(
        CanFrame(can_id=CanId(0x12345678, is_extended=True), data=b"\xFF")
    )
    assert encoded == b"T123456781FF\r"


def test_encode_zero_id_standard():
    codec = CanFrameCodec()
    encoded = codec.encode(CanFrame(can_id=CanId(0x000), data=b""))
    assert encoded == b"t0000\r"


def test_encode_returns_ascii_bytes():
    """encode 输出应为纯 ASCII（含 \\r 结束符）。"""

    codec = CanFrameCodec()
    encoded = codec.encode(CanFrame(can_id=CanId(0x1), data=b"\x00\x01"))
    encoded.decode("ascii")  # 不抛 UnicodeDecodeError
    assert encoded.endswith(b"\r")


# ── _parse 错误全分支 ────────────────────────────────────────────
def test_parse_empty_frame_error():
    events = CanFrameCodec().feed(b"\r")
    assert len(events) == 1
    assert events[0]["type"] == "error"
    assert events[0]["payload"]["reason"] == "empty_frame"


def test_parse_unknown_prefix_error():
    events = CanFrameCodec().feed(b"X1234\r")
    assert events[0]["payload"]["reason"] == "unknown_prefix:X"


def test_parse_frame_too_short_standard():
    """标准帧 body < 4（3 ID + 1 DLC）。"""

    events = CanFrameCodec().feed(b"t12\r")  # body="12" len=2 < 4
    assert events[0]["payload"]["reason"] == "frame_too_short"


def test_parse_frame_too_short_extended():
    """扩展帧 body < 9（8 ID + 1 DLC）。"""

    events = CanFrameCodec().feed(b"T1234\r")  # body="1234" len=4 < 9
    assert events[0]["payload"]["reason"] == "frame_too_short"


def test_parse_invalid_hex_id():
    events = CanFrameCodec().feed(b"tGGG1\r")  # G 非法 hex
    assert events[0]["payload"]["reason"] == "invalid_hex"


def test_parse_invalid_hex_dlc():
    """DLC 位非法 hex。"""

    events = CanFrameCodec().feed(b"t123G\r")  # ID ok, DLC='G' 非法
    assert events[0]["payload"]["reason"] == "invalid_hex"


def test_parse_dlc_exceeds_limit_standard():
    """标准帧 DLC > 8 拒绝。"""

    events = CanFrameCodec().feed(b"t123F\r")  # DLC=F(15) > 8
    assert events[0]["payload"]["reason"] == "dlc_exceeds_limit"


def test_parse_data_length_mismatch():
    """DLC=2 需 4 hex 字符，给 2 个 → mismatch。"""

    events = CanFrameCodec().feed(b"t1232AA\r")  # DLC=2, data="AA" len=2 ≠ 4
    assert events[0]["payload"]["reason"] == "data_length_mismatch"


def test_parse_invalid_data_hex():
    """data_hex 长度匹配但含非法字符。

    DLC=1 需 2 hex 字符；给 'GG'（长度对但非法 hex）→ invalid_data_hex。
    """

    events = CanFrameCodec().feed(b"t1231GG\r")  # DLC=1, data="GG" len=2 对，但非法 hex
    assert events[0]["payload"]["reason"] == "invalid_data_hex"


def test_error_event_includes_raw_and_protocol():
    """error 事件应含 raw 原始字节 + protocol_name。"""

    events = CanFrameCodec().feed(b"Zbad\r")
    err = events[0]
    assert err["raw"] == b"Zbad\r"
    assert err["protocol_name"] == PROTOCOL_NAME


# ── feed 空数据 ───────────────────────────────────────────────────
def test_feed_empty_bytes_returns_empty_list():
    assert CanFrameCodec().feed(b"") == []


def test_feed_none_ish_empty_no_crash():
    """feed 空字节不崩溃（buffer 不扩展）。"""

    codec = CanFrameCodec()
    codec.feed(b"")
    codec.feed(b"")
    assert codec.feed(b"") == []


# ── reset ─────────────────────────────────────────────────────────
def test_reset_clears_partial_buffer():
    """reset 清空未完成帧的缓冲区（残留字节丢弃）。

    reset 后 buffer 空；feed(b'\\r') 只剩结束符 → empty_frame error（非完整帧）。
    说明：reset 丢弃了之前 feed 的 't1232AA' 残留。
    """

    codec = CanFrameCodec()
    codec.feed(b"t1232AA")  # 无 \r，留在 buffer
    codec.reset()
    events = codec.feed(b"\r")
    # buffer 已清，单独 \r 无法组成帧 → empty_frame error（证明残留被丢弃）。
    assert len(events) == 1
    assert events[0]["payload"]["reason"] == "empty_frame"


def test_reset_clears_frame_index():
    """reset 重置帧计数（后续帧 index 从 1 重新开始）。"""

    codec = CanFrameCodec()
    encoded = codec.encode(CanFrame(can_id=CanId(0x1), data=b"\x01"))
    events = codec.feed(encoded)
    assert events[0]["payload"]["frameIndex"] == 1
    codec.reset()
    events2 = codec.feed(encoded)
    assert events2[0]["payload"]["frameIndex"] == 1  # 重置后重新从 1


# ── CAN-FD ────────────────────────────────────────────────────────
def test_can_fd_extended_frame_roundtrip():
    """扩展帧 DLC>8 解码为 is_fd=True。"""

    codec = CanFrameCodec()
    frame = CanFrame(
        can_id=CanId(0x1A2B3C4D, is_extended=True),
        data=b"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C",  # 12 字节
        is_fd=True,
    )
    encoded = codec.encode(frame)
    events = codec.feed(encoded)
    assert len(events) == 1
    assert events[0]["type"] == "frame"
    payload = events[0]["payload"]
    assert payload["isFd"] is True
    assert payload["isExtended"] is True
    assert payload["dlc"] == 12


def test_frame_index_increments_across_frames():
    """连续帧的 frameIndex 递增。"""

    codec = CanFrameCodec()
    f1 = codec.encode(CanFrame(can_id=CanId(0x1), data=b"\x01"))
    f2 = codec.encode(CanFrame(can_id=CanId(0x2), data=b"\x02"))
    events = codec.feed(f1 + f2)
    assert events[0]["payload"]["frameIndex"] == 1
    assert events[1]["payload"]["frameIndex"] == 2
