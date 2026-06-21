"""CrcCalculatorPanel 单元测试。

覆盖：
- ``compute_crc`` 纯函数：4 个 catalog check value（CRC-8/MAXIM、CRC-16/MODBUS、
  CRC-16/CCITT-FALSE、CRC-32/ISO-HDLC over ``b"123456789"``）+ 非法 width 抛错。
- ``CRC_PRESETS`` 自洽性：每个预设的 ``check`` 与 ``compute_crc(b"123456789", ...)``
  完全一致（防止预设表与算法核漂移）。
- ``CrcCalculatorPanel`` widget：objectName、构造不抛、预设按钮填配置并触发
  round-trip 计算。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.ui.tools.crc_calculator import (
    CRC_PRESETS,
    CrcCalculatorPanel,
    CrcPreset,
    compute_crc,
)


# ══════════════════════════════════════════════════════════════════
#  CRC 算法核（纯函数，无 Qt 依赖）
# ══════════════════════════════════════════════════════════════════
def test_compute_crc_crc8_maxim_check():
    """CRC-8/MAXIM over b"123456789" 应得 catalog check value 0xA1。"""

    crc = compute_crc(b"123456789", 8, 0x31, 0x00, True, True, 0x00)
    assert crc == 0xA1


def test_compute_crc_crc16_modbus_check():
    """CRC-16/MODBUS over b"123456789" 应得 catalog check value 0x4B37。"""

    crc = compute_crc(b"123456789", 16, 0x8005, 0xFFFF, True, True, 0x0000)
    assert crc == 0x4B37


def test_compute_crc_crc16_ccitt_false_check():
    """CRC-16/CCITT-FALSE over b"123456789" 应得 catalog check value 0x29B1。"""

    crc = compute_crc(b"123456789", 16, 0x1021, 0xFFFF, False, False, 0x0000)
    assert crc == 0x29B1


def test_compute_crc_crc32_iso_hdlc_check():
    """CRC-32/ISO-HDLC over b"123456789" 应得 catalog check value 0xCBF43926。"""

    crc = compute_crc(
        b"123456789", 32, 0x04C11DB7, 0xFFFFFFFF, True, True, 0xFFFFFFFF
    )
    assert crc == 0xCBF43926


def test_compute_crc_invalid_width_raises():
    """width 非 8 / 16 / 32 应抛 ValueError。"""

    with pytest.raises(ValueError):
        compute_crc(b"abc", 7, 0x00, 0x00, False, False, 0x00)


def test_compute_crc_empty_data_returns_init_xor_xor_out():
    """空输入下 CRC 应等于 (init ^ xor_out) & mask（无字节处理）。"""

    # init=0xA5, xor_out=0x5A, width=8 → 0xA5 ^ 0x5A = 0xFF
    crc = compute_crc(b"", 8, 0x31, 0xA5, True, True, 0x5A)
    assert crc == 0xFF


def test_all_presets_have_valid_check():
    """每个预设的 check value 必须与 compute_crc(b"123456789", ...) 一致。"""

    for name, preset in CRC_PRESETS.items():
        crc = compute_crc(
            b"123456789",
            preset.width,
            preset.poly,
            preset.init,
            preset.ref_in,
            preset.ref_out,
            preset.xor_out,
        )
        assert crc == preset.check, (
            f"{name}: got 0x{crc:X}, expected 0x{preset.check:X}"
        )


def test_crc_preset_is_frozen_dataclass():
    """CrcPreset 必须是 frozen dataclass（不可变）。"""

    preset = CRC_PRESETS["CRC-8/MAXIM"]
    with pytest.raises(Exception):
        preset.poly = 0xFF  # type: ignore[misc]


def test_crc_presets_contains_expected_names():
    """CRC_PRESETS 应含 4 个标准预设。"""

    expected = {"CRC-8/MAXIM", "CRC-16/MODBUS", "CRC-16/CCITT-FALSE", "CRC-32/ISO-HDLC"}
    assert set(CRC_PRESETS.keys()) == expected


# ══════════════════════════════════════════════════════════════════
#  CrcCalculatorPanel widget（需 qtbot）
# ══════════════════════════════════════════════════════════════════
def test_panel_objectname(qtbot):
    """面板 objectName 必须为 serialStationCrcCalculator。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    assert panel.objectName() == "serialStationCrcCalculator"


def test_panel_constructs_without_error(qtbot):
    """构造面板不抛异常；初始结果字段为有效 hex 字符串。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    text = panel._result_edit.text()
    assert text.startswith("0x"), f"initial result must be hex, got {text!r}"


def test_panel_subcontainer_objectnames(qtbot):
    """三个子容器应有指定 objectName。"""

    from PyQt6.QtWidgets import QWidget

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    names = {w.objectName() for w in panel.findChildren(QWidget)}
    assert "serialStationCrcCalculatorInput" in names
    assert "serialStationCrcCalculatorConfig" in names
    assert "serialStationCrcCalculatorResult" in names


def test_preset_button_fills_config(qtbot):
    """点击预设按钮（_apply_preset）后，配置控件应反映预设值。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    preset = CRC_PRESETS["CRC-16/MODBUS"]
    panel._apply_preset(preset)
    assert panel._width_combo.currentText() == "16"
    assert int(panel._poly_edit.text(), 16) == 0x8005
    assert int(panel._init_edit.text(), 16) == 0xFFFF
    assert panel._ref_in_check.isChecked() is True
    assert panel._ref_out_check.isChecked() is True
    assert int(panel._xor_edit.text(), 16) == 0x0000


def test_preset_roundtrip_produces_check_value(qtbot):
    """端到端：应用 CRC-16/MODBUS 预设 → 输入 ASCII "123456789" → 结果 == 0x4B37。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    panel._apply_preset(CRC_PRESETS["CRC-16/MODBUS"])
    panel._mode_ascii.setChecked(True)
    panel._input_edit.setText("123456789")  # 触发 textChanged → _recompute
    crc_text = panel._result_edit.text()
    assert int(crc_text, 16) == 0x4B37


def test_hex_mode_parses_spaced_bytes(qtbot):
    """Hex 模式下空格分隔的字节串应被正确解析。"""

    panel = CrcCalculatorPanel()
    qtbot.addWidget(panel)
    panel._apply_preset(CRC_PRESETS["CRC-8/MAXIM"])
    panel._mode_hex.setChecked(True)
    # "123456789" ASCII = 0x31 0x32 ... 0x39
    panel._input_edit.setText("31 32 33 34 35 36 37 38 39")
    crc_text = panel._result_edit.text()
    assert int(crc_text, 16) == 0xA1
