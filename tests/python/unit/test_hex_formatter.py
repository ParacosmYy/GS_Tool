"""HexFormatter 单元测试 — hex dump 格式化 + 数值解码。

覆盖：format_bytes（地址/hex/ASCII 三栏）、format_int（有/无符号 + 大小端）、
format_float（32/64 位）、format_ascii（非可打印替换）、边界（空数据/单字节）。
"""

from __future__ import annotations

import struct

from embeddebug.serial_station.data_inspector.hex_formatter import HexFormatter


def test_format_bytes_empty():
    assert HexFormatter.format_bytes(b"") == []


def test_format_bytes_single_line():
    """16 字节一行，含地址 + hex + ASCII 三栏。"""
    data = b"Hello, World!!!"
    lines = HexFormatter.format_bytes(data, offset=0)
    assert len(lines) == 1
    assert lines[0].startswith("00000000")
    assert "48" in lines[0]  # 'H' = 0x48
    assert "|Hello, World!!!|" in lines[0]


def test_format_bytes_multi_line_offset():
    """多行 + 非 0 起始偏移地址。"""
    data = bytes(range(32))
    lines = HexFormatter.format_bytes(data, offset=0x1000)
    assert len(lines) == 2
    assert lines[0].startswith("00001000")
    assert lines[1].startswith("00001010")


def test_format_bytes_non_printable_as_dot():
    """非可打印字符显示为点。"""
    data = bytes([0x00, 0x01, 0x41, 0x7F, 0x80])
    lines = HexFormatter.format_bytes(data)
    assert "|..A.." in lines[0]


def test_format_int_signed_little_endian():
    """有符号小端 2 字节。"""
    assert HexFormatter.format_int(b"\x01\x00", signed=True) == 1
    assert HexFormatter.format_int(b"\xFF\xFF", signed=True) == -1


def test_format_int_unsigned():
    """无符号 4 字节。"""
    assert HexFormatter.format_int(b"\xFF\xFF\xFF\xFF", signed=False) == 0xFFFFFFFF


def test_format_int_big_endian():
    """大端 2 字节。"""
    assert HexFormatter.format_int(b"\x00\x01", signed=False, big_endian=True) == 1


def test_format_int_single_byte():
    """单字节。"""
    assert HexFormatter.format_int(b"\x80", signed=True) == -128
    assert HexFormatter.format_int(b"\x80", signed=False) == 128


def test_format_int_empty_raises():
    """空数据抛 ValueError。"""
    import pytest
    with pytest.raises(ValueError):
        HexFormatter.format_int(b"")


def test_format_float_32bit():
    """32 位浮点。"""
    data = struct.pack("<f", 3.14)
    result = HexFormatter.format_float(data)
    assert abs(result - 3.14) < 0.001


def test_format_float_64bit_big_endian():
    """64 位大端浮点。"""
    data = struct.pack(">d", 2.718)
    result = HexFormatter.format_float(data, big_endian=True)
    assert abs(result - 2.718) < 0.001


def test_format_float_invalid_width_raises():
    """不支持的浮点位宽抛 ValueError。"""
    import pytest
    with pytest.raises(ValueError):
        HexFormatter.format_float(b"\x00\x00\x00")  # 3 bytes


def test_format_ascii_printable():
    """可打印 ASCII 保留。"""
    assert HexFormatter.format_ascii(b"ABC123") == "ABC123"


def test_format_ascii_non_printable_dots():
    """非可打印替换为点。"""
    assert HexFormatter.format_ascii(bytes([0x00, 0x41, 0x80])) == ".A."
