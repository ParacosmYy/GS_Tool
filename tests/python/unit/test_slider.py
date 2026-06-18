"""CommandSlider 跟手气泡 + release 发包语义测试。

覆盖诊断报告剩余项『Slider 原生无跟手』：
1. 跟手气泡：拖拽时显示 value 气泡跟随 handle 位置，释放隐藏。
2. release 发包：拖拽过程不发 command（避免刷屏），sliderReleased 发最终命令。

拆分自 test_controls_led_slider.py（满足 test_python_tests_are_split_by_behavior_domain
250 行门禁）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls import CommandSlider


def test_command_slider_bubble_hidden_by_default(qtbot):
    """跟手气泡默认隐藏（拖拽时才显示）。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    assert slider._bubble.isHidden()


def test_command_slider_bubble_shown_on_press(qtbot):
    """sliderPressed 后气泡应显示（isHidden=False）。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider._slider.sliderPressed.emit()
    assert not slider._bubble.isHidden()


def test_command_slider_bubble_hidden_on_release(qtbot):
    """sliderReleased 后气泡应隐藏。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider._slider.sliderPressed.emit()
    assert not slider._bubble.isHidden()
    slider._slider.sliderReleased.emit()
    assert slider._bubble.isHidden()


def test_command_slider_drag_suppresses_command(qtbot):
    """拖拽过程中不应发 command（release 才发，避免刷屏）。"""

    slider = CommandSlider(label="PWM", command_template="PWM={value}",
                           minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    commands: list[str] = []
    slider.command.connect(lambda cmd: commands.append(cmd))
    # 模拟拖拽：pressed → 多次 valueChanged → released。
    slider._slider.sliderPressed.emit()
    slider._slider.setValue(60)
    slider._slider.setValue(70)
    slider._slider.setValue(80)
    # 拖拽中不应发包。
    assert commands == []
    # released 后发一次最终命令。
    slider._slider.sliderReleased.emit()
    assert commands == ["PWM=80"]


def test_command_slider_non_drag_set_value_emits(qtbot):
    """非拖拽场景（代码 set_value）正常发包。"""

    slider = CommandSlider(label="PWM", command_template="PWM={value}")
    qtbot.addWidget(slider)
    commands: list[str] = []
    slider.command.connect(lambda cmd: commands.append(cmd))
    slider.set_value(42)
    assert "PWM=42" in commands


def test_command_slider_bubble_updates_text_on_drag(qtbot):
    """拖拽时气泡文字应跟随当前值更新。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider._slider.sliderPressed.emit()
    slider._slider.setValue(77)
    assert slider._bubble.text() == "77"
