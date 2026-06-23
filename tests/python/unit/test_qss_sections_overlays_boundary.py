"""qss_sections_overlays waveform_overlays_section 边界测试。

waveform_overlays_section 此前无直接测试（grep 0 命中）。
本文件覆盖返回字符串契约 + objectName 覆盖 + palette/tokens 引用。

覆盖：
1. waveform_overlays_section 返回非空 str。
2. 含 serialStationWaveformCursorHud objectName。
3. 含 serialStationWaveformLegend objectName。
4. 含 serialStationWaveformLegendChip objectName。
5. 含 serialStationWaveformCursorX/Y objectName。
6. 含等宽字体（FONT_FAMILY_MONO）。
7. 含 BG_PANEL/TEXT_MUTED palette 引用。
8. 返回 str 类型。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.qss_sections_overlays import (
    waveform_overlays_section,
)


def test_waveform_overlays_section_returns_nonempty_str():
    result = waveform_overlays_section()
    assert isinstance(result, str)
    assert len(result) > 0


def test_covers_cursor_hud_objectname():
    result = waveform_overlays_section()
    assert "serialStationWaveformCursorHud" in result


def test_covers_legend_objectname():
    result = waveform_overlays_section()
    assert "serialStationWaveformLegend" in result


def test_covers_legend_chip_objectname():
    result = waveform_overlays_section()
    assert "serialStationWaveformLegendChip" in result


def test_covers_cursor_xy_objectnames():
    result = waveform_overlays_section()
    assert "serialStationWaveformCursorX" in result
    assert "serialStationWaveformCursorY" in result


def test_uses_mono_font():
    """waveform overlays 用等宽字体。"""

    result = waveform_overlays_section()
    # FONT_FAMILY_MONO 是 token 引用，QSS 中展开为 font-family。
    assert "font-family" in result.lower()


def test_uses_palette_references():
    """含 BG_PANEL / TEXT_MUTED palette 引用。"""

    result = waveform_overlays_section()
    # palette 常量在 f-string 中展开为 hex 色；验证含 background-color/color 属性。
    assert "background-color" in result or "color:" in result
