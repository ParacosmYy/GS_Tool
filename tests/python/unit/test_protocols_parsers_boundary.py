"""protocols fire_water/just_float 纯 helper + 解析边界单元测试。

补强 test_protocols.py：_is_float / _looks_like_header / reset / CRLF / 自定义分隔符 /
frame_index 递增 / 分片 feed / invalid_payload_length / tail 常量。
"""

from __future__ import annotations

import struct

import pytest

from embeddebug.serial_station.protocols.fire_water import (
    FireWaterProtocol,
    _is_float,
)
from embeddebug.serial_station.protocols.just_float import JustFloatProtocol


# ── _is_float 纯函数 ─────────────────────────────────────────────────────


def test_is_float_integer():
    assert _is_float("42") is True


def test_is_float_decimal():
    assert _is_float("3.14") is True


def test_is_float_negative():
    assert _is_float("-5.5") is True


def test_is_float_scientific_notation():
    assert _is_float("1e10") is True
    assert _is_float("1.5E-3") is True


def test_is_float_nan():
    assert _is_float("nan") is True


def test_is_float_inf():
    assert _is_float("inf") is True


def test_is_float_empty_string():
    assert _is_float("") is False


def test_is_float_non_numeric():
    assert _is_float("abc") is False
    assert _is_float("temp") is False


def test_is_float_hex_string():
    """十六进制字符串不是合法 float。"""

    assert _is_float("0xFF") is False


# ── _looks_like_header 静态方法 ──────────────────────────────────────────


def test_looks_like_header_all_text():
    """全文本字段（如 temp,volt）→ True。"""

    assert FireWaterProtocol._looks_like_header(["temp", "volt", "current"]) is True


def test_looks_like_header_contains_number():
    """含数字字段 → False。"""

    assert FireWaterProtocol._looks_like_header(["temp", "25.0"]) is False


def test_looks_like_header_all_numbers():
    """全数字 → False。"""

    assert FireWaterProtocol._looks_like_header(["1.0", "2.0"]) is False


def test_looks_like_header_empty_field():
    """含空字段 → False（field and ... 短路）。"""

    assert FireWaterProtocol._looks_like_header(["", "temp"]) is False


def test_looks_like_header_single_text():
    """单文本字段 → True。"""

    assert FireWaterProtocol._looks_like_header(["temperature"]) is True


# ── FireWaterProtocol reset + 边界 ───────────────────────────────────────


def test_firewater_reset_clears_buffer():
    """reset 清空缓冲区 + frame_index 归零。"""

    protocol = FireWaterProtocol()
    protocol.feed(b"1.0,2.0\n")
    protocol.reset()
    # reset 后再 feed 应从 frame_index=0 开始
    events = protocol.feed(b"3.0,4.0\n")
    assert events[0].payload["frameIndex"] == 1


def test_firewater_reset_clears_channel_names():
    """reset 清空已学习的 channel_names。"""

    protocol = FireWaterProtocol()
    protocol.feed(b"temp,volt\n")  # 学习 header
    protocol.reset()
    # reset 后无 header，measurement 用默认 ch1/ch2
    events = protocol.feed(b"1.0,2.0\n")
    assert events[0].payload["channelNames"] == ["ch1", "ch2"]


def test_firewater_empty_feed_returns_empty():
    """feed(b"") → 空事件列表。"""

    protocol = FireWaterProtocol()
    assert protocol.feed(b"") == []


def test_firewater_crlf_line_ending():
    """CRLF（\\r\\n）双换行被正确消费（不产生空行事件）。"""

    protocol = FireWaterProtocol()
    events = protocol.feed(b"1.0\r\n2.0\r\n")
    assert len(events) == 2
    assert events[0].payload["values"] == [1.0]
    assert events[1].payload["values"] == [2.0]


def test_firewater_custom_delimiter():
    """自定义分隔符（如分号）。"""

    protocol = FireWaterProtocol(delimiter=";")
    events = protocol.feed(b"1.0;2.0;3.0\n")
    assert events[0].payload["values"] == [1.0, 2.0, 3.0]


def test_firewater_frame_index_increments():
    """多帧 frame_index 递增（1, 2, 3...）。"""

    protocol = FireWaterProtocol()
    events = protocol.feed(b"1.0\n2.0\n3.0\n")
    assert events[0].payload["frameIndex"] == 1
    assert events[1].payload["frameIndex"] == 2
    assert events[2].payload["frameIndex"] == 3


def test_firewater_partial_feed_buffers():
    """分片 feed 累积到完整行才解析。"""

    protocol = FireWaterProtocol()
    assert protocol.feed(b"1.0,") == []  # 不完整
    events = protocol.feed(b"2.0\n")  # 补全
    assert len(events) == 1
    assert events[0].payload["values"] == [1.0, 2.0]


def test_firewater_prefix_stripped():
    """带前缀（如 fw:）的行被正确解析。"""

    protocol = FireWaterProtocol()
    events = protocol.feed(b"fw:1.0,2.0\n")
    assert events[0].payload["values"] == [1.0, 2.0]


# ── JustFloatProtocol tail 常量 + reset ──────────────────────────────────


def test_justfloat_tail_constant():
    """tail = 00 00 80 7F（JustFloat 帧尾）。"""

    assert JustFloatProtocol.tail == b"\x00\x00\x80\x7f"


def test_justfloat_reset_clears_buffer():
    """reset 清空缓冲区 + frame_index 归零。"""

    protocol = JustFloatProtocol()
    frame = struct.pack("<2f", 1.0, 2.0) + JustFloatProtocol.tail
    protocol.feed(frame)
    protocol.reset()
    events = protocol.feed(frame)
    assert events[0].payload["frameIndex"] == 1


def test_justfloat_invalid_payload_length_error():
    """payload 长度不是 4 的倍数 → error 事件。"""

    protocol = JustFloatProtocol()
    # 5 字节 payload + 4 字节 tail = 9 字节（5 % 4 != 0）
    bad_payload = b"\x00\x00\x80\x3f\x00"  # 5 bytes
    events = protocol.feed(bad_payload + JustFloatProtocol.tail)
    assert events[0].type == "error"
    assert events[0].payload["reason"] == "invalid_payload_length"


def test_justfloat_empty_payload_error():
    """空 payload（仅 tail）→ error 事件。"""

    protocol = JustFloatProtocol()
    events = protocol.feed(JustFloatProtocol.tail)
    assert events[0].type == "error"
    assert events[0].payload["reason"] == "invalid_payload_length"


def test_justfloat_empty_feed_returns_empty():
    """feed(b"") → 空事件列表。"""

    protocol = JustFloatProtocol()
    assert protocol.feed(b"") == []


def test_justfloat_negative_values():
    """负浮点数正确解析（小端 float32）。"""

    protocol = JustFloatProtocol()
    frame = struct.pack("<2f", -1.5, -3.14) + JustFloatProtocol.tail
    events = protocol.feed(frame)
    assert events[0].payload["values"][0] == pytest.approx(-1.5)
    assert events[0].payload["values"][1] == pytest.approx(-3.14)


def test_justfloat_channel_names_default():
    """channelNames 默认 ch1/ch2/...。"""

    protocol = JustFloatProtocol()
    frame = struct.pack("<3f", 1.0, 2.0, 3.0) + JustFloatProtocol.tail
    events = protocol.feed(frame)
    assert events[0].payload["channelNames"] == ["ch1", "ch2", "ch3"]


def test_justfloat_partial_feed_buffers():
    """分片 feed 累积到完整帧才解析。"""

    protocol = JustFloatProtocol()
    frame = struct.pack("<2f", 1.0, 2.0) + JustFloatProtocol.tail
    # 分两半喂入
    half = frame[:4]
    rest = frame[4:]
    assert protocol.feed(half) == []  # 不完整
    events = protocol.feed(rest)
    assert len(events) == 1
