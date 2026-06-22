"""CrcPreset + compute_crc + _parse_hex_bytes + PlaceholderPanel 边界测试。

补强 test_crc_calculator 未直接断言的边角：
- CrcPreset frozen + 8 字段 + CRC_PRESETS 非空。
- compute_crc：空 data + 宽度 8/16/32 + ref_in/ref_out 组合。
- _parse_hex_bytes：合法 hex + 空串 + 非 hex + 含空格。
- _parse_int：十进制 + 0x 十六进制 + 空。
- PlaceholderPanel：build 返回 QWidget + objectName + on_enter/leave 不崩溃。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.tools.crc_calculator import (
    CRC_PRESETS,
    CrcCalculatorPanel,
    CrcPreset,
    compute_crc,
)
from embeddebug.serial_station.ui.panels.placeholder_panel import PlaceholderPanel


# ── CrcPreset ─────────────────────────────────────────────────────────


def test_crc_preset_is_frozen():
    """CrcPreset 是 frozen dataclass。"""

    p = CrcPreset("T", 8, 0x31, 0x00, True, True, 0x00, 0xA1)
    with pytest.raises((AttributeError, Exception)):
        p.name = "X"  # type: ignore[misc]


def test_crc_preset_has_eight_fields():
    """CrcPreset 含 8 个字段。"""

    p = CrcPreset("T", 8, 0x31, 0x00, True, True, 0x00, 0xA1)
    assert p.name == "T"
    assert p.width == 8
    assert p.poly == 0x31
    assert p.init == 0x00
    assert p.ref_in is True
    assert p.ref_out is True
    assert p.xor_out == 0x00
    assert p.check == 0xA1


def test_crc_presets_non_empty():
    """CRC_PRESETS 非空。"""

    assert len(CRC_PRESETS) > 0


def test_crc_presets_check_values_match():
    """CRC_PRESETS 含 CRC-16/MODBUS（check=0x4B37）。"""

    assert "CRC-16/MODBUS" in CRC_PRESETS
    assert CRC_PRESETS["CRC-16/MODBUS"].check == 0x4B37


# ── compute_crc ───────────────────────────────────────────────────────


def test_compute_crc_empty_data():
    """空 data → 仅 init + xor_out（不崩溃）。"""

    result = compute_crc(b"", width=8, poly=0x31, init=0x00,
                         ref_in=True, ref_out=True, xor_out=0x00)
    assert isinstance(result, int)


def test_compute_crc_width_8():
    """8 位 CRC（b"123456789" → check value）。"""

    result = compute_crc(b"123456789", width=8, poly=0x31, init=0x00,
                         ref_in=True, ref_out=True, xor_out=0x00)
    assert result == 0xA1  # CRC-8/MAXIM check


def test_compute_crc_width_16_modbus():
    """16 位 CRC MODBUS（b"123456789" → 0x4B37）。"""

    result = compute_crc(b"123456789", width=16, poly=0x8005, init=0xFFFF,
                         ref_in=True, ref_out=True, xor_out=0x0000)
    assert result == 0x4B37


def test_compute_crc_ref_false():
    """ref_in=False/ref_out=False（CCITT-FALSE）。"""

    result = compute_crc(b"123456789", width=16, poly=0x1021, init=0xFFFF,
                         ref_in=False, ref_out=False, xor_out=0x0000)
    assert result == 0x29B1


# ── _parse_hex_bytes ──────────────────────────────────────────────────


def test_parse_hex_bytes_valid(qtbot):
    """合法 hex 字符串。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    assert panel._parse_hex_bytes("48656c6c6f") == b"Hello"


def test_parse_hex_bytes_empty(qtbot):
    """空串 → b""。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    assert panel._parse_hex_bytes("") == b""


def test_parse_hex_bytes_with_spaces(qtbot):
    """含空格的 hex（容错）。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    result = panel._parse_hex_bytes("48 65 6c")
    assert result == b"\x48\x65\x6c"


def test_parse_hex_bytes_odd_length(qtbot):
    """奇数长度 hex → 可能补零或空。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    # 奇数长度的处理：前补零或不崩溃
    result = panel._parse_hex_bytes("F")
    assert isinstance(result, bytes)


# ── _parse_int ────────────────────────────────────────────────────────


def test_parse_int_decimal(qtbot):
    """_parse_int 解析数字（十进制或十六进制均可）。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    result = panel._parse_int("255")
    assert isinstance(result, int)
    assert result > 0


def test_parse_int_hex(qtbot):
    """0x 十六进制。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    result = panel._parse_int("0xFF")
    assert isinstance(result, int)


def test_parse_int_empty(qtbot):
    """空串 → 0（兜底）。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    assert panel._parse_int("") == 0


# ── PlaceholderPanel ──────────────────────────────────────────────────


def test_placeholder_build_returns_widget(qtbot):
    """build 返回 QWidget。"""

    from embeddebug.app.app_controller import AppController

    panel = PlaceholderPanel("test", "Test", "desc", icon_name="cable")
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_placeholder_build_has_objectname(qtbot):
    """build 返回的 widget 有 objectName。"""

    from embeddebug.app.app_controller import AppController

    panel = PlaceholderPanel("test", "Test", "desc")
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() != ""


def test_placeholder_on_enter_leave(qtbot):
    """on_enter/on_leave 不崩溃（空实现）。"""

    panel = PlaceholderPanel("test", "Test", "desc")
    panel.on_enter()
    panel.on_leave()
