"""HexViewerPanel 单测。

覆盖：
- 纯函数核：``to_ascii_repr`` / ``format_hex_line`` / ``format_hex_dump`` / ``parse_hex_input``。
- UI 层：``HexViewerPanel`` 构造 / objectName / ``set_data`` 端到端刷新。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.ui.tools.hex_viewer import (
    BYTES_PER_LINE,
    HexViewerPanel,
    format_hex_dump,
    format_hex_line,
    parse_hex_input,
    to_ascii_repr,
)


# ── to_ascii_repr ──────────────────────────────────────────────────
def test_to_ascii_repr_printable():
    """0x41 ('A') → 'A'。"""
    assert to_ascii_repr(0x41) == "A"
    assert to_ascii_repr(0x20) == " "  # 空格是可打印边界
    assert to_ascii_repr(0x7E) == "~"  # 上界


def test_to_ascii_repr_non_printable():
    """0x00 / 0x01 / 0xFF → '.'。"""
    assert to_ascii_repr(0x00) == "."
    assert to_ascii_repr(0x01) == "."
    assert to_ascii_repr(0xFF) == "."
    assert to_ascii_repr(0x1F) == "."  # 下界外
    assert to_ascii_repr(0x7F) == "."  # 上界外


# ── format_hex_line ────────────────────────────────────────────────
def test_format_hex_line_basic():
    """format_hex_line(0, b'Hello') 应含偏移 / hex / ASCII 三段。"""
    result = format_hex_line(0, b"Hello")
    assert result.startswith("00000000  ")
    assert "48 65 6c 6c 6f" in result
    # ASCII 列在尾部
    assert result.rstrip().endswith("Hello")


def test_format_hex_line_offset_format():
    """偏移按 8 位 hex 格式化。"""
    assert format_hex_line(0x10, b"AB").startswith("00000010  ")
    assert format_hex_line(0xFFFFFF, b"AB").startswith("00ffffff  ")


def test_format_hex_line_padding():
    """数据短于 bytes_per_line 时，字节列以空格补齐保持列宽恒定。"""
    full = format_hex_line(0, b"\x00" * BYTES_PER_LINE)
    partial = format_hex_line(0, b"\x00" * 5)

    # 结构：offset(8) + "  " + bytes_col(48) + "  " + ascii
    # bytes_col = g1(23) + "  " + g2(23) = 48
    full_bytes = full[10:58]
    partial_bytes = partial[10:58]
    assert len(full_bytes) == 48
    assert len(partial_bytes) == 48
    # 两者前缀都应是 "00 00 00 00 00"
    assert full_bytes.startswith("00 00 00 00 00")
    assert partial_bytes.startswith("00 00 00 00 00")
    # partial 末尾应有补齐的空格（g2 全空）
    assert partial_bytes[23:].strip() == ""


def test_format_hex_line_group_separator():
    """16 字节满行：g1 与 g2 之间应有 2 空格分隔，ASCII 列 g1/g2 间 1 空格。"""
    result = format_hex_line(0, b"A" * 16)
    # 字节列："41 41 41 41 41 41 41 41  41 41 41 41 41 41 41 41"
    assert "41 41 41 41 41 41 41 41  41 41 41 41 41 41 41 41" in result
    # ASCII 列："AAAAAAAA AAAAAAAA"（8 + space + 8）
    assert result.endswith("AAAAAAAA AAAAAAAA")


# ── format_hex_dump ────────────────────────────────────────────────
def test_format_hex_dump_multiline():
    """b'Hello World!' → 1 行；b'\\x00'*32 → 2 行。"""
    one_line = format_hex_dump(b"Hello World!")
    assert one_line.count("\n") == 0
    assert one_line.startswith("00000000  ")

    two_line = format_hex_dump(b"\x00" * 32)
    assert two_line.count("\n") == 1
    lines = two_line.split("\n")
    assert lines[0].startswith("00000000  ")
    assert lines[1].startswith("00000010  ")


def test_format_hex_dump_empty():
    """空数据 → 空字符串。"""
    assert format_hex_dump(b"") == ""


def test_format_hex_dump_base_offset():
    """base_offset=16 → 首行偏移字段为 0x10。"""
    result = format_hex_dump(b"Hello", base_offset=16)
    assert result.startswith("00000010  ")


# ── parse_hex_input ────────────────────────────────────────────────
def test_parse_hex_input_simple():
    """'48 65 6c 6c 6f' → b'Hello'。"""
    assert parse_hex_input("48 65 6c 6c 6f") == b"Hello"


def test_parse_hex_input_tolerates_prefix():
    """'0x48 0x65' → b'He'（容忍 0x 前缀）。"""
    assert parse_hex_input("0x48 0x65") == b"He"
    # 大写 0X 与混合也应工作
    assert parse_hex_input("0X41 0x42") == b"AB"


def test_parse_hex_input_tolerates_separators():
    """容忍换行 / 逗号 / 制表符。"""
    assert parse_hex_input("48,65\n6c\t6c 6f") == b"Hello"


def test_parse_hex_input_empty():
    """空字符串 / 纯空白 → b''。"""
    assert parse_hex_input("") == b""
    assert parse_hex_input("   \n\t  ") == b""


def test_parse_hex_input_invalid_raises():
    """'xy' 含非 hex 字符 → ValueError。"""
    with pytest.raises(ValueError):
        parse_hex_input("xy")
    with pytest.raises(ValueError):
        parse_hex_input("0x4G")


def test_parse_hex_input_odd_length_raises():
    """奇数 hex 位 → ValueError。"""
    with pytest.raises(ValueError):
        parse_hex_input("486")


# ── HexViewerPanel（UI） ──────────────────────────────────────────
def test_panel_constructs_without_error(qtbot):
    """面板应能无异常构造。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    assert panel is not None


def test_panel_objectname(qtbot):
    """根 objectName 应为 'serialStationHexViewer'。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    assert panel.objectName() == "serialStationHexViewer"


def test_panel_subwidgets_objectnames(qtbot):
    """子控件应设置 objectName 便于 QSS 命中。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    assert panel._input_edit.objectName() == "serialStationHexViewerInputEdit"
    assert panel._view.objectName() == "serialStationHexViewerDump"


def test_panel_default_mode_is_ascii(qtbot):
    """默认输入模式应为 ASCII。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    assert panel._mode_ascii.isChecked()
    assert not panel._mode_hex.isChecked()


def test_set_data_updates_view(qtbot):
    """set_data(b'Hello') → 主视图 ASCII 列应包含 'Hello'。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    panel.set_data(b"Hello")
    text = panel._view.toPlainText()
    assert "Hello" in text
    # 字节列也应包含 "48 65 6c 6c 6f"
    assert "48 65 6c 6c 6f" in text
    # 状态栏应显示字节数 5
    assert "5" in panel._status_label.text()


def test_set_data_switches_to_hex_mode(qtbot):
    """set_data 应切到 Hex 模式并填入 hex 文本。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    panel.set_data(b"Hi")
    assert panel._mode_hex.isChecked()
    assert "48 69" in panel._input_edit.toPlainText()


def test_format_button_with_ascii_input(qtbot):
    """ASCII 模式下点击格式化按钮 → 主视图显示 dump。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    panel._input_edit.setPlainText("Hi")
    panel._format_btn.click()
    text = panel._view.toPlainText()
    assert "48 69" in text  # 'H'=0x48, 'i'=0x69
    assert "Hi" in text  # ASCII 列


def test_format_button_with_hex_input(qtbot):
    """Hex 模式下点击格式化按钮 → 解析并显示 dump。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    panel._mode_hex.setChecked(True)
    panel._input_edit.setPlainText("48 69")
    panel._format_btn.click()
    text = panel._view.toPlainText()
    assert "48 69" in text
    assert "Hi" in text  # ASCII 列


def test_format_button_invalid_hex_clears_view(qtbot):
    """Hex 模式下输入非法 → 主视图清空，状态栏提示无效。"""
    panel = HexViewerPanel()
    qtbot.addWidget(panel)
    panel._mode_hex.setChecked(True)
    panel._input_edit.setPlainText("xy")
    panel._format_btn.click()
    assert panel._view.toPlainText() == ""
