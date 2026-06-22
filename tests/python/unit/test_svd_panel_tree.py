"""svd_panel_tree 辅助函数单元测试 — hex/access_label/trunc/field_rows。

覆盖：_hex 格式化、_access_label 枚举映射、_trunc 截断、
field_table_rows 位域行生成。
"""

from __future__ import annotations

from embeddebug.serial_station.svd.model import Access, SvdField, SvdRegister
from embeddebug.serial_station.ui.panels.svd_panel_tree import (
    _access_label,
    _hex,
    _trunc,
    field_table_rows,
)


def test_hex_default_width():
    assert _hex(0x1234) == "0x00001234"


def test_hex_custom_width():
    assert _hex(0xFF, width=2) == "0xFF"


def test_hex_zero():
    assert _hex(0) == "0x00000000"


def test_access_label_read_only():
    assert _access_label(Access.READ_ONLY) == "只读"


def test_access_label_write_only():
    assert _access_label(Access.WRITE_ONLY) == "只写"


def test_access_label_read_write():
    assert _access_label(Access.READ_WRITE) == "读写"


def test_access_label_unknown_defaults():
    """未识别 access 回退到「读写」。"""
    assert _access_label(None) == "读写"  # type: ignore[arg-type]


def test_trunc_short_text():
    assert _trunc("short") == "short"


def test_trunc_empty():
    assert _trunc("") == ""


def test_trunc_long_text():
    result = _trunc("x" * 100, limit=40)
    assert len(result) <= 43  # 40 + 省略号
    assert result.endswith("…" if result != "x" * 40 else "")


def test_field_table_rows_single_bit():
    reg = SvdRegister(name="R", description="", address_offset=0, fields=(
        SvdField(name="EN", description="", bit_offset=0, bit_width=1, access=Access.READ_WRITE),
    ))
    rows = field_table_rows(reg)
    assert len(rows) == 1
    assert rows[0][0] == "EN"
    assert "bit 0" in rows[0][1]
    assert rows[0][2] == "读写"


def test_field_table_rows_multi_bit_range():
    reg = SvdRegister(name="R", description="", address_offset=0, fields=(
        SvdField(name="DATA", description="", bit_offset=8, bit_width=4, access=Access.READ_ONLY),
    ))
    rows = field_table_rows(reg)
    assert len(rows) == 1
    assert "bit 8-11" in rows[0][1]
    assert rows[0][2] == "只读"


def test_field_table_rows_empty():
    reg = SvdRegister(name="R", description="", address_offset=0)
    assert field_table_rows(reg) == []
