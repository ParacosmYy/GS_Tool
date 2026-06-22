"""CAN DBC 纯 helper 单元测试（_extract_intel / _extract_motorola / decode_signal 边界）。

补强 test_can_dbc.py 未直接断言的 can/dbc.py 私有位提取 helper：
- _extract_intel：小端字节序位提取（单字节/跨字节/start_bit 偏移/bit_length mask）。
- _extract_motorola：大端字节序位提取（简化模型，从 start_bit 向低位扫描）。
- decode_signal：Motorola 字节序 + factor=0 + offset 单独 + bit_length 超字节范围。
- DbcSignal/DbcMessage frozen/lookup 边界。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.can.dbc import (
    DbcDatabase,
    DbcMessage,
    DbcSignal,
    _extract_intel,
    _extract_motorola,
    decode_signal,
)


# ── _extract_intel（小端位提取） ─────────────────────────────────────────


def test_extract_intel_single_byte_full():
    """单字节全 8 位提取 = 原字节值。"""

    assert _extract_intel(b"\xFF", 0, 8) == 0xFF
    assert _extract_intel(b"\x05", 0, 8) == 5


def test_extract_intel_single_byte_partial():
    """单字节部分位（start_bit=2, length=4）。"""

    # 0b11010100 = 0xD4, start=2 length=4 → bits[2..5] = 0b0101 = 5
    assert _extract_intel(b"\xD4", 2, 4) == 0b0101


def test_extract_intel_cross_byte_boundary():
    """跨字节边界（start_bit=4, length=8 跨 byte0 高 4 位 + byte1 低 4 位）。"""

    # byte0=0xF0 (高 4 位 1111), byte1=0x0F (低 4 位 1111)
    # 小端：value = 0xF0 | (0x0F << 8) = 0x0FF0
    # start=4 length=8 → (0x0FF0 >> 4) & 0xFF = 0xFF
    assert _extract_intel(b"\xF0\x0F", 4, 8) == 0xFF


def test_extract_intel_two_bytes_little_endian():
    """两字节小端组合值。"""

    # b"\x01\x02" 小端 = 0x0201 = 513
    assert _extract_intel(b"\x01\x02", 0, 16) == 0x0201


def test_extract_intel_zero_length_returns_zero():
    """bit_length=0 → mask (1<<0)-1 = 0 → 结果 0。"""

    assert _extract_intel(b"\xFF", 0, 0) == 0


def test_extract_intel_start_bit_beyond_byte():
    """start_bit 超单字节范围时从跨字节值提取。"""

    # b"\x00\x01" 小端 = 0x0100，start=8 length=4 → (0x0100 >> 8) & 0xF = 1
    assert _extract_intel(b"\x00\x01", 8, 4) == 1


# ── _extract_motorola（大端位提取，简化模型：start_bit=0 = MSB） ──────────


def test_extract_motorola_single_byte_high_bit():
    """大端单字节从 start_bit=0（MSB）取 length=1。"""

    # 0x80 = 0b10000000，start=0 length=1 → MSB = 1
    assert _extract_motorola(b"\x80", 0, 1) == 1


def test_extract_motorola_single_byte_multiple_bits():
    """大端单字节取高 4 位（start_bit=3, length=4 → MSB 起向下 4 位）。"""

    # 0xF0 = 0b11110000，start=3 length=4 → bits 3,2,1,0 = 1111 = 15
    assert _extract_motorola(b"\xF0", 3, 4) == 15


def test_extract_motorola_zero_length():
    """bit_length=0 → 不进循环，结果 0。"""

    assert _extract_motorola(b"\xFF", 7, 0) == 0


def test_extract_motorola_start_bit_out_of_range():
    """start_bit 指向不存在的字节 → 跳过，结果 0。"""

    # 只有 1 字节，start_bit=23 指向 byte_index=(23-0)//8=2，超出范围
    assert _extract_motorola(b"\xFF", 23, 4) == 0


# ── decode_signal 边界 ───────────────────────────────────────────────────


def test_decode_signal_motorola_byte_order():
    """Motorola 大端信号解码（start_bit=3 = 高 4 位起点）。"""

    sig = DbcSignal(name="x", start_bit=3, bit_length=4, is_little_endian=False)
    # 0xF0 高 4 位 = 15，factor=1 offset=0 → 15.0
    assert decode_signal(b"\xF0", sig) == 15.0


def test_decode_signal_factor_zero():
    """factor=0 → 物理值恒为 offset（忽略 raw）。"""

    sig = DbcSignal(name="x", start_bit=0, bit_length=8, is_little_endian=True,
                    factor=0.0, offset=42.0)
    assert decode_signal(b"\xFF", sig) == 42.0


def test_decode_signal_offset_only():
    """offset 单独应用（factor=1）。"""

    sig = DbcSignal(name="x", start_bit=0, bit_length=8, is_little_endian=True,
                    factor=1.0, offset=100.0)
    assert decode_signal(b"\x05", sig) == 105.0


def test_decode_signal_negative_factor():
    """负 factor + offset（反向比例转换）。"""

    sig = DbcSignal(name="temp", start_bit=0, bit_length=8, is_little_endian=True,
                    factor=-1.0, offset=255.0)
    # raw=0x05 → 5 * -1 + 255 = 250
    assert decode_signal(b"\x05", sig) == 250.0


def test_decode_signal_bit_length_exceeding_bytes():
    """bit_length 超过 raw_bytes 总位数 → mask 截断。"""

    sig = DbcSignal(name="x", start_bit=0, bit_length=16, is_little_endian=True)
    # 单字节 b"\xFF"，16 位提取：value=0xFF, mask 0xFFFF → 0xFF
    assert decode_signal(b"\xFF", sig) == 255.0


def test_decode_signal_fractional_factor():
    """小数 factor（如 0.1 分辨率）。"""

    sig = DbcSignal(name="volt", start_bit=0, bit_length=8, is_little_endian=True,
                    factor=0.1, offset=0.0)
    # raw=100 → 100 * 0.1 = 10.0 V
    assert decode_signal(b"\x64", sig) == 10.0


# ── DbcSignal / DbcMessage / DbcDatabase 边界 ────────────────────────────


def test_dbc_signal_is_frozen():
    """DbcSignal 是 frozen dataclass（不可变）。"""

    sig = DbcSignal(name="x", start_bit=0, bit_length=8, is_little_endian=True)
    with pytest.raises((AttributeError, Exception)):
        sig.name = "y"  # type: ignore[misc]


def test_dbc_signal_defaults():
    """DbcSignal 默认 factor=1.0 / offset=0.0 / unit=""。"""

    sig = DbcSignal(name="x", start_bit=0, bit_length=1, is_little_endian=True)
    assert sig.factor == 1.0
    assert sig.offset == 0.0
    assert sig.unit == ""


def test_dbc_message_signal_lookup_unknown_raises():
    """DbcMessage.signal 未知名 → KeyError。"""

    msg = DbcMessage(id=1, name="M", dlc=8)
    with pytest.raises(KeyError, match="未知信号"):
        msg.signal("ghost")


def test_dbc_database_message_lookup_unknown_raises():
    """DbcDatabase.message 未知 can_id → KeyError（dict[...] 行为）。"""

    db = DbcDatabase()
    with pytest.raises(KeyError):
        db.message(999)


def test_dbc_database_empty_messages_initially():
    """新 DbcDatabase messages 为空 dict。"""

    assert DbcDatabase().messages == {}
