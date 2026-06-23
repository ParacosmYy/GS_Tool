"""ValueDisplay set_value/_refresh 格式化 + trend 边界扩展测试。

test_controls_displays 覆盖基础 trend；本文件补格式化 + _previous 追踪 +
大/负值 + set_unit 后 _refresh + value() round-trip。

覆盖：
1. set_value 后 value() 返回新值。
2. set_value 保留 _previous（旧值）。
3. _refresh .2f 格式化（2 位小数）。
4. set_unit 后 value_widget 文本含 unit。
5. 大值（1e6）格式化不崩。
6. 负值格式化含 '-'。
7. 零值 trend flat。
8. 连续相同值 trend flat。
9. 微小 delta（<1e-9）trend flat。
10. set_label 后 label_widget 文本更新。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLabel

from embeddebug.serial_station.ui.controls.value_display import ValueDisplay


def _trend_widget(display):
    return display.findChild(QLabel, "serialStationValueTrend")


def _value_widget(display):
    return display.findChild(QLabel, "serialStationValueNumber")


# ── set_value + value() ──────────────────────────────────────────
def test_set_value_returns_new_value(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(42.5)
    assert display.value() == 42.5


def test_set_value_tracks_previous(qtbot):
    """set_value 保留 _previous（旧值）。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(10.0)
    display.set_value(20.0)
    assert display._previous == 10.0


# ── _refresh 格式化 ──────────────────────────────────────────────
def test_refresh_formats_two_decimals(qtbot):
    """_refresh .2f 格式化（2 位小数）。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(3.14159)
    text = _value_widget(display).text()
    assert "3.14" in text


def test_set_unit_appears_in_value_text(qtbot):
    """set_unit 后 value_widget 文本含 unit。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_unit("V")
    display.set_value(5.0)
    text = _value_widget(display).text()
    assert "V" in text


def test_large_value_no_crash(qtbot):
    """大值（1e6）格式化不崩。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(1_000_000.0)
    text = _value_widget(display).text()
    assert "1000000" in text


def test_negative_value_format(qtbot):
    """负值格式化含 '-'。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(-42.5)
    text = _value_widget(display).text()
    assert "-42.50" in text


# ── trend 边界 ────────────────────────────────────────────────────
def test_zero_value_trend_flat(qtbot):
    """零值（默认→0）trend flat。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(0.0)
    trend = _trend_widget(display)
    assert "◆" in trend.text()


def test_consecutive_same_value_trend_flat(qtbot):
    """连续相同值 trend flat。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(5.0)
    display.set_value(5.0)
    trend = _trend_widget(display)
    assert "◆" in trend.text()


def test_tiny_delta_trend_flat(qtbot):
    """微小 delta（<1e-9）trend flat。"""

    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(1.0)
    display.set_value(1.0 + 1e-10)  # 微小增量
    trend = _trend_widget(display)
    assert "◆" in trend.text()


# ── set_label ────────────────────────────────────────────────────
def test_set_label_updates_label_widget(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_label("Temperature")
    label = display.findChild(QLabel, "serialStationValueLabel")
    assert label is not None
    assert "Temperature" in label.text()
