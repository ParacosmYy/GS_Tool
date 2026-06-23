"""qss_sections_controls_parts _led_part/_slider_part/_gauge_value_part 边界测试。

私有 QSS helper 此前无直接测试（grep 0 命中）。
本文件覆盖返回字符串契约 + objectName 覆盖。

覆盖：
1. _led_part 返回非空 str + 含 serialStationStatusLed objectName。
2. _slider_part 返回非空 str + 含 serialStationCommandSlider objectName。
3. _gauge_value_part 返回非空 str + 含 serialStation objectName。
4. _dashboard_panel_part 返回非空 str。
5. _auxiliary_widgets_part 返回非空 str。
6. 所有 helper 返回 str 类型。
7. _led_part 含 color/background 引用。
8. _slider_part 含 groove/handle 引用。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.qss_sections_controls_parts import (
    _auxiliary_widgets_part,
    _dashboard_panel_part,
    _gauge_value_part,
    _led_part,
    _slider_part,
)


# ── _led_part ────────────────────────────────────────────────────
def test_led_part_returns_nonempty_str():
    result = _led_part()
    assert isinstance(result, str)
    assert len(result) > 0


def test_led_part_covers_objectname():
    result = _led_part()
    assert "serialStationStatusLed" in result


def test_led_part_has_color_reference():
    result = _led_part()
    assert "color" in result.lower() or "background" in result.lower()


# ── _slider_part ─────────────────────────────────────────────────
def test_slider_part_returns_nonempty_str():
    result = _slider_part()
    assert isinstance(result, str)
    assert len(result) > 0


def test_slider_part_covers_objectname():
    result = _slider_part()
    assert "serialStationCommandSlider" in result


def test_slider_part_has_groove_or_handle():
    result = _slider_part()
    assert "groove" in result.lower() or "handle" in result.lower()


# ── _gauge_value_part ────────────────────────────────────────────
def test_gauge_value_part_returns_nonempty_str():
    result = _gauge_value_part()
    assert isinstance(result, str)
    assert len(result) > 0


def test_gauge_value_part_covers_objectname():
    result = _gauge_value_part()
    # GaugeWidget 或 ValueDisplay objectName。
    assert "serialStation" in result


# ── _dashboard_panel_part ────────────────────────────────────────
def test_dashboard_panel_part_returns_nonempty_str():
    result = _dashboard_panel_part()
    assert isinstance(result, str)
    assert len(result) > 0


def test_dashboard_panel_part_covers_objectname():
    result = _dashboard_panel_part()
    assert "serialStation" in result


# ── _auxiliary_widgets_part ──────────────────────────────────────
def test_auxiliary_widgets_part_returns_nonempty_str():
    result = _auxiliary_widgets_part()
    assert isinstance(result, str)
    assert len(result) > 0


def test_auxiliary_widgets_part_covers_objectname():
    result = _auxiliary_widgets_part()
    assert "serialStation" in result
