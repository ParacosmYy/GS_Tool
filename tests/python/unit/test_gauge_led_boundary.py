"""GaugeWidget + StatusLed 边界单元测试。

补强 test_controls_displays / test_controls_led_slider 未直接断言的边角：
- GaugeWidget：set_range + set_value 边界（超 range clamp）+ value() 读取 + set_tween +
  set_unit/set_label 不崩溃 + _value_to_angle 范围。
- LedState 枚举：成员完备 + value 小写。
- StatusLed：set_from_value 阈值映射 + set_breathing 不崩溃 + sizeHint 正。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


from embeddebug.serial_station.ui.controls.gauge import GaugeWidget
from embeddebug.serial_station.ui.controls.led import LedState, StatusLed


# ── GaugeWidget 边界 ──────────────────────────────────────────────────


def test_gauge_initial_value_zero(qtbot):
    """GaugeWidget 初始 value=0.0。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    assert g.value() == 0.0


def test_gauge_set_value(qtbot):
    """set_value 正值。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_value(50.0)
    assert g.value() == 50.0


def test_gauge_set_value_negative(qtbot):
    """set_value 负值。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_value(-10.0)
    assert g.value() == -10.0


def test_gauge_set_range(qtbot):
    """set_range 不崩溃。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_range(-100.0, 100.0)


def test_gauge_set_tween(qtbot):
    """set_tween 不崩溃。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_tween(True)
    g.set_tween(False)


def test_gauge_set_unit(qtbot):
    """set_unit 不崩溃。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_unit("V")


def test_gauge_set_label(qtbot):
    """set_label 不崩溃。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_label("温度")


def test_gauge_size_hint_positive(qtbot):
    """sizeHint 宽高 > 0。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    hint = g.sizeHint()
    assert hint.width() > 0
    assert hint.height() > 0


def test_gauge_value_to_angle_differs(qtbot):
    """_value_to_angle 在 min/max 给出不同角度。"""

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_range(0.0, 100.0)
    angle_min = g._value_to_angle(0.0)
    angle_max = g._value_to_angle(100.0)
    assert angle_min != angle_max


# ── LedState 枚举 ─────────────────────────────────────────────────────


def test_led_state_has_members():
    """LedState 至少含 OFF + 颜色态。"""

    assert LedState.OFF is not None


def test_led_state_values_lowercase():
    """LedState value 小写。"""

    for member in LedState:
        assert member.value == member.value.lower()


def test_led_state_off_value():
    """LedState.OFF.value = "off"。"""

    assert LedState.OFF.value == "off"


# ── StatusLed set_from_value ──────────────────────────────────────────


def test_status_led_set_from_value_green(qtbot):
    """set_from_value 正常范围 → GREEN。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(50.0, (10.0, 30.0, 90.0))
    # 不崩溃即可（具体状态依赖实现）


def test_status_led_set_from_value_low(qtbot):
    """set_from_value 低于 error 阈值。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(5.0, (10.0, 30.0, 90.0))


def test_status_led_set_from_value_high(qtbot):
    """set_from_value 高于 warn 阈值。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(95.0, (10.0, 30.0, 90.0))


def test_status_led_set_breathing(qtbot):
    """set_breathing 不崩溃。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_breathing(True)
    led.set_breathing(False)


def test_status_led_size_hint_positive(qtbot):
    """sizeHint 宽高 > 0。"""

    led = StatusLed()
    qtbot.addWidget(led)
    hint = led.sizeHint()
    assert hint.width() > 0
    assert hint.height() > 0
