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
