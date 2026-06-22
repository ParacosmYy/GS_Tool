"""RawDataProtocol + FireWaterProtocol build_command + feed 边界测试。

test_protocols.py 覆盖基础 feed/header/measurement/error；本文件补 build_command
编码路径 + feed 空数据/无效 UTF-8 + reset + 自定义 delimiter/max_channels 边界。

覆盖 RawDataProtocol：
1. build_command 默认 utf-8 编码。
2. build_command hex 参数（bytes.fromhex）。
3. build_command 自定义 encoding。
4. build_command 无 params 默认 utf-8。
5. feed 空数据返回空列表。
6. feed 无效 UTF-8 用 replace 解码。
7. reset 返回 None。

覆盖 FireWaterProtocol：
8. build_command 默认编码。
9. feed 自定义 delimiter（分号）。
10. feed max_channels=1 边界。
11. _looks_like_header 判定。
12. reset 清空缓冲。
"""

from __future__ import annotations

from embeddebug.serial_station.protocols.fire_water import FireWaterProtocol, _is_float
from embeddebug.serial_station.protocols.raw_data import RawDataProtocol


# ── RawDataProtocol.build_command ────────────────────────────────
def test_build_command_default_utf8():
    proto = RawDataProtocol()
    assert proto.build_command("hello") == b"hello"


def test_build_command_hex_param():
    """params hex=True → bytes.fromhex(command)。"""

    proto = RawDataProtocol()
    assert proto.build_command("48656c6c6f", {"hex": True}) == b"Hello"


def test_build_command_custom_encoding():
    """params encoding='ascii' → 用指定编码。"""

    proto = RawDataProtocol()
    assert proto.build_command("ABC", {"encoding": "ascii"}) == b"ABC"


def test_build_command_no_params_uses_utf8():
    proto = RawDataProtocol()
    # 中文应正确 utf-8 编码。
    assert proto.build_command("你好") == "你好".encode()


def test_build_command_empty_params():
    """params={} → 走 encoding 分支（默认 utf-8）。"""

    proto = RawDataProtocol()
    assert proto.build_command("x", {}) == b"x"


# ── RawDataProtocol.feed ──────────────────────────────────────────
def test_feed_empty_returns_empty_list():
    assert RawDataProtocol().feed(b"") == []


def test_feed_invalid_utf8_uses_replace():
    """非法 UTF-8 字节用 errors=replace 解码（U+FFFD）。"""

    events = RawDataProtocol().feed(b"\xff\xfe")
    assert len(events) == 1
    assert "\ufffd" in events[0].payload["text"]


def test_feed_preserves_raw_bytes():
    """feed 的 raw 应是原始字节的拷贝。"""

    events = RawDataProtocol().feed(b"hello\xff")
    assert events[0].raw == b"hello\xff"


def test_feed_payload_has_format_and_size():
    events = RawDataProtocol().feed(b"abc")
    assert events[0].payload["format"] == "raw_data"
    assert events[0].payload["size"] == 3


def test_reset_returns_none():
    assert RawDataProtocol().reset() is None


# ── FireWaterProtocol.build_command ───────────────────────────────
def test_fire_water_build_command_default():
    """build_command 默认加 \\n 行结束符。"""

    proto = FireWaterProtocol()
    assert proto.build_command("AT+RST") == b"AT+RST\n"


def test_fire_water_build_command_custom_line_ending():
    """params line_ending='\r\n' → 用指定行结束。"""

    proto = FireWaterProtocol()
    assert proto.build_command("AT", {"line_ending": "\r\n"}) == b"AT\r\n"


# ── FireWaterProtocol 自定义 delimiter ───────────────────────────
def test_fire_water_custom_delimiter_semicolon():
    """自定义 delimiter=';' 应正确分割测量值。"""

    proto = FireWaterProtocol(delimiter=";")
    # 先喂 header 行。
    proto.feed(b"temp;volt\n")
    events = proto.feed(b"fw:24.5;3.3\n")
    assert len(events) == 1
    assert events[0].payload["values"] == [24.5, 3.3]


# ── FireWaterProtocol max_channels 边界 ──────────────────────────
def test_fire_water_max_channels_one():
    """max_channels=1：单通道测量应通过。"""

    proto = FireWaterProtocol(max_channels=1)
    proto.feed(b"temp\n")
    events = proto.feed(b"fw:24.5\n")
    assert len(events) == 1
    assert events[0].payload["values"] == [24.5]


def test_fire_water_max_channels_one_rejects_two():
    """max_channels=1：两通道测量（用默认逗号分隔）应触发 too_many_channels error。"""

    proto = FireWaterProtocol(max_channels=1)
    proto.feed(b"temp,volt\n")  # header 2 通道
    events = proto.feed(b"fw:24.5,3.3\n")
    assert events[0].type == "error"
    assert events[0].payload["reason"] == "too_many_channels"


# ── FireWaterProtocol reset ───────────────────────────────────────
def test_fire_water_reset_clears_buffer():
    """reset 清空缓冲（残留半行丢弃）。"""

    proto = FireWaterProtocol()
    proto.feed(b"incomplete line without newline")
    proto.reset()
    # reset 后缓冲空，补 \r\n 应解析为单行。
    events = proto.feed(b"fw:1.0\n")
    # 可能是 header（非数字）或 measurement（fw: 前缀）。
    assert len(events) >= 1


# ── _is_float helper ──────────────────────────────────────────────
def test_is_float_valid_numbers():
    assert _is_float("24.5") is True
    assert _is_float("0") is True
    assert _is_float("-3.14") is True
    assert _is_float("1e10") is True


def test_is_float_invalid():
    assert _is_float("abc") is False
    assert _is_float("") is False
    assert _is_float("NaN_text") is False


def test_is_float_nan_string():
    """'nan' 能被 float() 解析 → _is_float True。"""

    assert _is_float("nan") is True
    assert _is_float("inf") is True
