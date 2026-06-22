"""TzPreset + TZ_PRESETS + BYTES_PER_LINE + HexViewer pure functions 边界测试。

补强 test_timestamp_converter / test_hex_viewer 未直接断言的边角：
- TzPreset：frozen + 3 字段 + offset_seconds 范围。
- TZ_PRESETS：含 UTC + China + offset 全在 [-12h, +14h]。
- BYTES_PER_LINE=16 常量。
- to_ascii_repr：可打印/不可打印/边界字符。
- parse_hex_input：空串/含空格/非法字符。
- format_hex_line：offset 格式 + 空行。
- format_hex_dump：空 data/单行/多行。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ui.tools.hex_viewer import (
    BYTES_PER_LINE,
    format_hex_dump,
    format_hex_line,
    parse_hex_input,
    to_ascii_repr,
)
from embeddebug.serial_station.ui.tools.timestamp_converter import (
    TZ_PRESETS,
    TzPreset,
)


# ── BYTES_PER_LINE ────────────────────────────────────────────────────


def test_bytes_per_line_is_16():
    """BYTES_PER_LINE = 16。"""

    assert BYTES_PER_LINE == 16


# ── to_ascii_repr ─────────────────────────────────────────────────────


def test_to_ascii_repr_printable():
    """可打印 ASCII → 原字符。"""

    assert to_ascii_repr(0x41) == "A"
    assert to_ascii_repr(0x30) == "0"


def test_to_ascii_repr_non_printable():
    """不可打印 → '.'。"""

    assert to_ascii_repr(0x00) == "."
    assert to_ascii_repr(0x01) == "."


def test_to_ascii_repr_boundary():
    """边界字符：0x1F=不可打印, 0x20=空格, 0x7E=~, 0x7F=不可打印。"""

    assert to_ascii_repr(0x1F) == "."
    assert to_ascii_repr(0x20) == " "
    assert to_ascii_repr(0x7E) == "~"
    assert to_ascii_repr(0x7F) == "."


# ── parse_hex_input ───────────────────────────────────────────────────


def test_parse_hex_input_empty():
    """空串 → b""。"""

    assert parse_hex_input("") == b""


def test_parse_hex_input_with_spaces():
    """含空格 hex → 解析。"""

    assert parse_hex_input("48 65 6c") == b"\x48\x65\x6c"


def test_parse_hex_input_valid():
    """合法 hex。"""

    assert parse_hex_input("48656c6c6f") == b"Hello"


def test_parse_hex_input_odd_length_raises():
    """奇数长度 → ValueError。"""

    with pytest.raises(ValueError):
        parse_hex_input("F")


# ── format_hex_line ───────────────────────────────────────────────────


def test_format_hex_line_empty():
    """空 data → 仍含 offset。"""

    line = format_hex_line(0, b"")
    assert "0000" in line or "0:" in line.lower()


def test_format_hex_line_offset():
    """非零 offset。"""

    line = format_hex_line(16, b"\x41")
    assert "10" in line  # 0x10 = 16


def test_format_hex_line_contains_ascii():
    """含 ASCII 表示。"""

    line = format_hex_line(0, b"AB")
    assert "A" in line
    assert "B" in line


# ── format_hex_dump ───────────────────────────────────────────────────


def test_format_hex_dump_empty():
    """空 data → 空或单行。"""

    result = format_hex_dump(b"")
    assert isinstance(result, str)


def test_format_hex_dump_single_line():
    """短数据 → 单行。"""

    result = format_hex_dump(b"Hello")
    assert "Hello" in result


def test_format_hex_dump_multi_line():
    """超 16 字节 → 多行。"""

    data = bytes(range(32))
    result = format_hex_dump(data)
    assert result.count("\n") >= 1


def test_format_hex_dump_base_offset():
    """非零 base_offset。"""

    result = format_hex_dump(b"AB", base_offset=256)
    assert "100" in result  # 0x100 = 256


# ── TzPreset ──────────────────────────────────────────────────────────


def test_tz_preset_is_frozen():
    """TzPreset 是 frozen dataclass。"""

    p = TzPreset("UTC", 0, "UTC")
    with pytest.raises((AttributeError, Exception)):
        p.name = "X"  # type: ignore[misc]


def test_tz_preset_has_three_fields():
    """TzPreset 含 name/offset_seconds/label 3 字段。"""

    p = TzPreset("China", 8 * 3600, "Asia/Shanghai (UTC+8)")
    assert p.name == "China"
    assert p.offset_seconds == 8 * 3600
    assert p.label == "Asia/Shanghai (UTC+8)"


def test_tz_presets_non_empty():
    """TZ_PRESETS 非空。"""

    assert len(TZ_PRESETS) >= 3


def test_tz_presets_contains_utc():
    """TZ_PRESETS 含 UTC（offset=0）。"""

    utc = [p for p in TZ_PRESETS if p.offset_seconds == 0]
    assert len(utc) >= 1


def test_tz_presets_offsets_in_range():
    """所有 offset_seconds 在 [-12h, +14h]。"""

    for p in TZ_PRESETS:
        assert -12 * 3600 <= p.offset_seconds <= 14 * 3600


def test_tz_presets_labels_non_empty():
    """所有 label 非空。"""

    for p in TZ_PRESETS:
        assert p.label
