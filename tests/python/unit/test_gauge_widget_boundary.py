"""GaugeWidget set_range/set_value/set_tween/_value_to_angle 边界测试。

test_controls_displays 覆盖基础；本文件补 set_range clamp + set_tween +
_value_to_angle 角度映射 + displayed_value 属性。

覆盖：
1. set_range min>=max → max 被 clamp 到 min+1e-9。
2. set_tween(False) 停止 tween + displayed=value。
3. set_tween(True) set_value 启动 tween 动画。
4. _value_to_angle 最小值→-210°。
5. _value_to_angle 最大值→30°。
6. _value_to_angle 中值→中间角度。
7. _value_to_angle 超范围 clamp。
8. displayed_value 属性 round-trip。
9. set_unit + set_label 后 value 不变。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import math

from embeddebug.serial_station.ui.controls.gauge import GaugeWidget


# ── set_range clamp ───────────────────────────────────────────────
def test_set_range_min_equals_max_clamped(qtbot):
    """min==max → max 被 clamp 到 min+1e-9（避免除零）。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(50.0, 50.0)
    assert gauge._maximum > gauge._minimum
    assert gauge._maximum - gauge._minimum < 1e-6  # 接近 1e-9


def test_set_range_min_greater_than_max_clamped(qtbot):
    """min>max → max 被 clamp 到 min+1e-9。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(100.0, 50.0)  # min > max
    assert gauge._maximum >= gauge._minimum


def test_set_range_normal(qtbot):
    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(0.0, 200.0)
    assert gauge._minimum == 0.0
    assert gauge._maximum == 200.0


# ── set_tween ─────────────────────────────────────────────────────
def test_set_tween_false_stops_anim(qtbot):
    """set_tween(False) 停止进行中的 tween + displayed=value。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_tween(True)
    gauge.set_value(50.0)
    gauge.set_tween(False)
    assert gauge._tween_enabled is False
    assert gauge._displayed == 50.0  # displayed 同步到 value


def test_set_tween_true_starts_anim_on_set_value(qtbot):
    """set_tween(True) + set_value 启动 QPropertyAnimation。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_tween(True)
    gauge.set_value(75.0)
    assert gauge._tween_anim is not None


def test_set_tween_false_no_anim(qtbot):
    """set_tween(False) + set_value 不启动动画（直接设 displayed）。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_tween(False)
    gauge.set_value(30.0)
    assert gauge._tween_anim is None or gauge._tween_anim.state() == 0


# ── _value_to_angle 角度映射（radians, 225°→-45° 共 270°） ───────
def test_value_to_angle_minimum(qtbot):
    """最小值（ratio=0）→ 225° in radians。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(0.0, 100.0)
    angle = gauge._value_to_angle(0.0)
    assert abs(angle - math.radians(225.0)) < 0.01


def test_value_to_angle_maximum(qtbot):
    """最大值（ratio=1）→ -45° in radians。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(0.0, 100.0)
    angle = gauge._value_to_angle(100.0)
    assert abs(angle - math.radians(-45.0)) < 0.01


def test_value_to_angle_midpoint(qtbot):
    """中值（ratio=0.5）→ 225 - 135 = 90° in radians。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(0.0, 100.0)
    angle = gauge._value_to_angle(50.0)
    assert abs(angle - math.radians(90.0)) < 0.01


def test_value_to_angle_below_min_clamped(qtbot):
    """低于 min 的值 clamp 到 min 角度（225°）。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(0.0, 100.0)
    angle = gauge._value_to_angle(-50.0)
    assert abs(angle - math.radians(225.0)) < 0.01


def test_value_to_angle_above_max_clamped(qtbot):
    """高于 max 的值 clamp 到 max 角度（-45°）。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(0.0, 100.0)
    angle = gauge._value_to_angle(200.0)
    assert abs(angle - math.radians(-45.0)) < 0.01


# ── displayed_value 属性 ─────────────────────────────────────────
def test_displayed_value_round_trip(qtbot):
    """displayed_value 属性 round-trip。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge._set_displayed(42.0)
    assert gauge._get_displayed() == 42.0


def test_set_unit_label_preserves_value(qtbot):
    """set_unit + set_label 不改变 value。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_value(99.0)
    gauge.set_unit("V")
    gauge.set_label("Voltage")
    assert gauge.value() == 99.0
    assert gauge._unit == "V"
    assert gauge._label == "Voltage"
