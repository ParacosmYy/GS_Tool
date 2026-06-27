"""QSS helper 分区边界测试。

聚合原 aux / controls_parts / domain_parts / overlays 的纯字符串契约测试，
减少同域小文件数量，保持 objectName、状态选择器和基础样式覆盖。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ui.theme.qss_sections_controls_parts import (
    _auxiliary_widgets_part,
    _dashboard_panel_part,
    _gauge_value_part,
    _led_part,
    _slider_part,
)
from embeddebug.serial_station.ui.theme.qss_sections_domain_parts import (
    _checkboxes,
    _field_labels,
    _inputs,
    _main_buttons,
    _panel_roots,
    _secondary_buttons,
    _status_labels,
    _tables_and_trees,
    _text_views,
)
from embeddebug.serial_station.ui.theme.qss_sections_domain_parts_aux import (
    _about_labels,
    _accent_swatches,
    _settings_tabs,
)
from embeddebug.serial_station.ui.theme.qss_sections_overlays import (
    waveform_overlays_section,
)


@pytest.mark.parametrize(
    ("factory", "required"),
    [
        (_settings_tabs, ("serialStationSettingsTab", "selected", "hover")),
        (_accent_swatches, ("serialStationAccentSwatch", "checked")),
        (_about_labels, ("serialStationSettings", "color:")),
        (_led_part, ("serialStationStatusLed", "background")),
        (_slider_part, ("serialStationCommandSlider", "groove", "handle")),
        (_gauge_value_part, ("serialStation",)),
        (_dashboard_panel_part, ("serialStation",)),
        (_auxiliary_widgets_part, ("serialStation", "transparent")),
        (_panel_roots, ("serialStation",)),
        (_field_labels, ("serialStation",)),
        (_status_labels, ("serialStation",)),
        (_text_views, ("serialStation",)),
        (_main_buttons, ("serialStation", "hover")),
        (_secondary_buttons, ("serialStation", "disabled")),
        (_inputs, ("serialStation",)),
        (_checkboxes, ("serialStation",)),
        (_tables_and_trees, ("serialStation",)),
    ],
)
def test_qss_helper_returns_nonempty_contract(factory, required):
    qss = factory()

    assert isinstance(qss, str)
    assert qss
    for token in required:
        assert token.lower() in qss.lower()


@pytest.mark.parametrize(
    "object_name",
    [
        "serialStationWaveformCursorHud",
        "serialStationWaveformLegend",
        "serialStationWaveformLegendChip",
        "serialStationWaveformCursorX",
        "serialStationWaveformCursorY",
    ],
)
def test_waveform_overlays_section_covers_objectnames(object_name):
    qss = waveform_overlays_section()

    assert object_name in qss


def test_waveform_overlays_section_uses_font_and_palette_properties():
    qss = waveform_overlays_section()

    assert "font-family" in qss.lower()
    assert "background-color" in qss or "color:" in qss
