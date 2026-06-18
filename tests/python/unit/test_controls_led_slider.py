"""B6 控件库测试（一）：状态 LED / 命令滑块。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QSlider

from embeddebug.serial_station.ui.controls import CommandSlider, LedState, StatusLed


# ── 状态 LED ──────────────────────────────────────────────────────
def test_status_led_has_objectname(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    assert led.objectName() == "serialStationStatusLed"


def test_status_led_default_state_is_off(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    assert led.state == LedState.OFF


def test_status_led_set_state(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.GREEN)
    assert led.state == LedState.GREEN


def test_status_led_set_label(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_label("Link")
    assert led.toolTip() == "Link"


def test_status_led_threshold_green(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(60.0, (20.0, 10.0, 50.0))
    assert led.state == LedState.GREEN


def test_status_led_threshold_yellow(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(30.0, (20.0, 10.0, 50.0))
    assert led.state == LedState.YELLOW


def test_status_led_threshold_red(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(12.0, (20.0, 10.0, 50.0))
    assert led.state == LedState.RED


def test_status_led_threshold_off(qtbot):
    led = StatusLed()
    qtbot.addWidget(led)
    led.set_from_value(5.0, (20.0, 10.0, 50.0))
    assert led.state == LedState.OFF


def test_led_state_enum_values():
    assert LedState.OFF.value == "off"
    assert LedState.GREEN.value == "green"
    assert LedState.RED.value == "red"


# ── 命令滑块 ──────────────────────────────────────────────────────
def test_command_slider_has_objectname(qtbot):
    slider = CommandSlider(label="PWM")
    qtbot.addWidget(slider)
    assert slider.objectName() == "serialStationCommandSlider"


def test_command_slider_default_value(qtbot):
    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    assert slider.value() == 50


def test_command_slider_emits_command_on_change(qtbot):
    slider = CommandSlider(label="PWM", command_template="PWM={value}")
    qtbot.addWidget(slider)
    commands: list[str] = []
    slider.command.connect(lambda cmd: commands.append(cmd))
    slider.set_value(75)
    assert "PWM=75" in commands


def test_command_slider_custom_formatter(qtbot):
    slider = CommandSlider(label="PWM")
    qtbot.addWidget(slider)
    slider.set_formatter(lambda v: f"SET {v * 10}")
    commands: list[str] = []
    slider.command.connect(lambda cmd: commands.append(cmd))
    slider.set_value(5)
    assert "SET 50" in commands


def test_command_slider_label_accessible(qtbot):
    slider = CommandSlider(label="Duty")
    qtbot.addWidget(slider)
    assert slider.label() == "Duty"


def test_command_slider_track_objectname(qtbot):
    slider = CommandSlider()
    qtbot.addWidget(slider)
    track = slider.findChild(QSlider, "serialStationSliderTrack")
    assert track is not None


# ── Batch 3 (B1): LED 常亮呼吸 / Gauge 指针 tween / Button 按压 ──────
def test_status_led_breathing_starts_on_green(qtbot):
    """切到 GREEN（连接态）应启动常亮呼吸动画。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.GREEN)
    assert led.state == LedState.GREEN
    # 呼吸动画应已启动（_breathing_anim 非 None）。
    assert led._breathing_anim is not None
    assert led._breathing_anim.loopCount() == -1  # 无限循环


def test_status_led_no_breathing_on_red(qtbot):
    """RED（错误态）不应呼吸（避免干扰，错误态应稳定醒目）。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.RED)
    assert led._breathing_anim is None


def test_status_led_breathing_can_be_disabled(qtbot):
    """set_breathing(False) 应停止呼吸。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.GREEN)
    assert led._breathing_anim is not None
    led.set_breathing(False)
    assert led._breathing_anim is None


def test_status_led_state_switch_stops_previous_breathing(qtbot):
    """从 GREEN 切到 RED 应停止呼吸（避免呼吸态泄漏到错误态）。"""

    led = StatusLed()
    qtbot.addWidget(led)
    led.set_state(LedState.GREEN)
    green_anim = led._breathing_anim
    led.set_state(LedState.RED)
    assert led._breathing_anim is None
    assert green_anim is not None  # 确实曾经有呼吸


def test_gauge_value_tween_uses_displayed_value(qtbot):
    """Gauge set_value 应启动 tween，displayed_value 从旧值插值到新值。"""

    from embeddebug.serial_station.ui.controls import GaugeWidget

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_value(50.0)
    # tween 关闭时 displayed 直接等于 value。
    g.set_tween(False)
    g.set_value(80.0)
    assert g.displayed_value == 80.0


def test_gauge_tween_disabled_snaps(qtbot):
    """关闭 tween 后 set_value 应立即 snap（无动画）。"""

    from embeddebug.serial_station.ui.controls import GaugeWidget

    g = GaugeWidget()
    qtbot.addWidget(g)
    g.set_tween(False)
    g.set_value(42.0)
    assert g.displayed_value == 42.0
    assert g.value() == 42.0


def test_configurable_button_press_animation(qtbot):
    """点击 ConfigurableButton 应触发按压动画并发出 command。"""

    from embeddebug.serial_station.ui.controls import ConfigurableButton

    btn = ConfigurableButton("Send", command_template="AT+SEND")
    qtbot.addWidget(btn)
    commands: list[str] = []
    btn.command.connect(lambda cmd: commands.append(cmd))
    btn.click()
    assert "AT+SEND" in commands


def test_configurable_button_press_animation_can_disable(qtbot):
    """set_press_animation(False) 后点击不触发动画但仍发命令。"""

    from embeddebug.serial_station.ui.controls import ConfigurableButton

    btn = ConfigurableButton("Send", command_template="AT+PING")
    qtbot.addWidget(btn)
    btn.set_press_animation(False)
    commands: list[str] = []
    btn.command.connect(lambda cmd: commands.append(cmd))
    btn.click()
    assert "AT+PING" in commands
