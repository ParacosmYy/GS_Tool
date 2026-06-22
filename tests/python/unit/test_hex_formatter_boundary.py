"""HexFormatter 边界扩展单元测试。

补强 test_hex_formatter.py 未直接断言的边角：
- format_int：8 字节 struct 快速路径 + 非标准宽度（3/5/7 字节 int.from_bytes 回退）。
- format_bytes：bytes_per_line clamp（0/>16 → 合法范围）+ 单字节 + offset。
- format_float：32 位大端 + 64 位小端对称 + 空数据 ValueError。
- format_ascii：边界字符（0x1F/0x7F/0x20/0x7E）+ 空串。
"""

from __future__ import annotations

import struct

import pytest

from embeddebug.serial_station.data_inspector.hex_formatter import HexFormatter


# ── HexFormatter.format_int 8 字节 + 非标准宽度 ──────────────────────────


def test_format_int_8_bytes_signed_little_endian():
    """8 字节走 struct 快速路径（'q' 格式符）。"""

    data = struct.pack("<q", 1234567890)
    assert HexFormatter.format_int(data, signed=True) == 1234567890


def test_format_int_8_bytes_unsigned_big_endian():
    """8 字节大端无符号。"""

    data = struct.pack(">Q", 0xFFFFFFFFFFFFFFFF)
    assert HexFormatter.format_int(data, signed=False, big_endian=True) == 0xFFFFFFFFFFFFFFFF


def test_format_int_8_bytes_negative():
    """8 字节负数（signed）。"""

    data = struct.pack("<q", -42)
    assert HexFormatter.format_int(data, signed=True) == -42


def test_format_int_3_bytes_falls_back_to_from_bytes():
    """3 字节（非 1/2/4/8）走 int.from_bytes 回退路径。"""

    # 3 字节小端 0x010203 = 66051
    assert HexFormatter.format_int(b"\x03\x02\x01", signed=False) == 0x010203


def test_format_int_5_bytes_falls_back_to_from_bytes():
    """5 字节走 int.from_bytes 回退。"""

    data = b"\xFF\x00\x00\x00\x00"  # 小端 = 255
    assert HexFormatter.format_int(data, signed=False) == 255


def test_format_int_7_bytes_falls_back_signed():
    """7 字节有符号回退。"""

    # 7 字节小端，最高字节 0x80 → 负数
    data = b"\x00\x00\x00\x00\x00\x00\x80"
    assert HexFormatter.format_int(data, signed=True) < 0


def test_format_int_2_bytes_unsigned_boundary():
    """2 字节无符号 0xFFFF = 65535（不回退到 -1）。"""

    assert HexFormatter.format_int(b"\xFF\xFF", signed=False) == 65535


# ── HexFormatter.format_bytes bytes_per_line clamp ───────────────────────


def test_format_bytes_bytes_per_line_zero_clamps_to_one():
    """bytes_per_line=0 → clamp 到 1（每行 1 字节）。"""

    data = b"AB"
    lines = HexFormatter.format_bytes(data, bytes_per_line=0)
    assert len(lines) == 2  # 2 字节，每行 1 → 2 行


def test_format_bytes_bytes_per_line_over_16_clamps_to_16():
    """bytes_per_line=100 → clamp 到 16。"""

    data = bytes(range(20))
    lines = HexFormatter.format_bytes(data, bytes_per_line=100)
    assert len(lines) == 2  # 20 字节，每行 16 → 2 行


def test_format_bytes_custom_bytes_per_line_8():
    """bytes_per_line=8 → 每 8 字节一行。"""

    data = bytes(range(16))
    lines = HexFormatter.format_bytes(data, bytes_per_line=8)
    assert len(lines) == 2


def test_format_bytes_single_byte():
    """单字节 → 1 行。"""

    lines = HexFormatter.format_bytes(b"\x41")
    assert len(lines) == 1
    assert "41" in lines[0]


def test_format_bytes_offset_affects_address():
    """非零 offset 影响地址列。"""

    lines = HexFormatter.format_bytes(b"AB", offset=0x100)
    assert lines[0].startswith("00000100")


# ── HexFormatter.format_float 对称 + 空数据 ─────────────────────────────


def test_format_float_32bit_big_endian():
    """32 位大端浮点。"""

    data = struct.pack(">f", 1.5)
    assert HexFormatter.format_float(data, big_endian=True) == pytest.approx(1.5)


def test_format_float_64bit_little_endian():
    """64 位小端浮点。"""

    data = struct.pack("<d", 3.14159)
    assert HexFormatter.format_float(data) == pytest.approx(3.14159)


def test_format_float_empty_raises():
    """空数据 → ValueError（长度 0 不在 4/8）。"""

    with pytest.raises(ValueError):
        HexFormatter.format_float(b"")


# ── HexFormatter.format_ascii 边界字符 ───────────────────────────────────


def test_format_ascii_boundary_chars():
    """边界字符：0x1F=点 / 0x20=空格保留 / 0x7E=~保留 / 0x7F=点。"""

    result = HexFormatter.format_ascii(bytes([0x1F, 0x20, 0x7E, 0x7F]))
    assert result == ". ~."  # 0x1F→. 0x20→space 0x7E→~ 0x7F→.


def test_format_ascii_empty_string():
    """空数据 → 空串。"""

    assert HexFormatter.format_ascii(b"") == ""


def test_format_ascii_all_printable():
    """全可打印 ASCII 原样保留。"""

    assert HexFormatter.format_ascii(b"Hello!@#") == "Hello!@#"
