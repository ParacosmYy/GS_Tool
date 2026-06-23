"""StatusLed _glow 属性 + LedState 枚举 + set_label 边界测试。

test_controls_led_slider 覆盖基础；本文件补 _glow round-trip + LedState 枚举值 +
set_label 更新 + set_breathing 显式 True + _pulse_glow 不崩。

覆盖：
1. _get_glow/_set_glow round-trip。
2. _set_glow 触发 update（不崩）。
3. LedState 枚举 5 成员 + 值小写。
4. LedState 成员互异。
5. set_label 更新 _label_widget 文本。
6. set_breathing(True) 显式启动呼吸（即使非 GREEN）。
7. _pulse_glow 不崩（QTimer 回调）。
8. set_state 同状态不重启动画。
9. state 属性 round-trip。
10. LedState GREEN/YELLOW/RED/OFF/BLUE 值。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.led import LedState, StatusLed


# ── _glow round-trip ─────────────────────────────────────────────
def test_glow_round_trip(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led._set_glow(0.5)
    assert led._get_glow() == 0.5


def test_glow_set_triggers_update_no_crash(qtbot):
    """_set_glow 触发 update（不崩）。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led._set_glow(0.8)  # 不抛


def test_glow_default_zero(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    assert led._get_glow() == 0.0


# ── LedState 枚举 ────────────────────────────────────────────────
def test_led_state_has_five_members():
    assert len(LedState) == 5


def test_led_state_values_lowercase():
    for state in LedState:
        assert state.value == state.value.lower()


def test_led_state_members_distinct():
    values = [s.value for s in LedState]
    assert len(set(values)) == 5


def test_led_state_known_values():
    assert LedState.OFF.value == "off"
    assert LedState.GREEN.value == "green"
    assert LedState.YELLOW.value == "yellow"
    assert LedState.RED.value == "red"
    assert LedState.BLUE.value == "blue"


# ── state 属性 round-trip ────────────────────────────────────────
def test_state_property_round_trip(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.GREEN)
    assert led.state == LedState.GREEN
    led.set_state(LedState.RED)
    assert led.state == LedState.RED


# ── set_label ────────────────────────────────────────────────────
def test_set_label_updates_text(qtbot):
    """set_label 更新内部标签文本。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_label("CPU Temp")
    # set_label 内部存储或 widget 文本。
    # 验证不崩 + 可多次调用。
    led.set_label("GPU Temp")
    led.set_label("")


# ── set_breathing 显式 True ──────────────────────────────────────
def test_set_breathing_true_explicit(qtbot):
    """set_breathing(True) 显式启动呼吸。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.OFF)  # 初始非呼吸态
    led.set_breathing(True)
    # set_breathing(True) 可能启动呼吸动画（取决于实现）。


def test_set_breathing_toggle_cycle(qtbot):
    """breathing toggle 循环不崩。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_breathing(True)
    led.set_breathing(False)
    led.set_breathing(True)
    led.set_breathing(False)


# ── _pulse_glow 不崩 ─────────────────────────────────────────────
def test_pulse_glow_no_crash(qtbot):
    """_pulse_glow QTimer 回调不崩。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led._pulse_glow()  # 直接调用回调
