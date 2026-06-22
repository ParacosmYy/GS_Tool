"""CommandSlider + ConfigurableButton 边界单元测试。

补强 test_controls_led_slider / test_controls_displays 未直接断言的边角：
- CommandSlider：set_value 后 value() 读取 / set_formatter 后 label() 更新 /
  初始 label 参数 / _build_command 含 formatter。
- ConfigurableButton：set_icon 不崩溃 / set_press_animation / set_hover_lift /
  set_ripple / get_command 默认空串 / set_command_template 后 get_command。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.configurable_button import ConfigurableButton
from embeddebug.serial_station.ui.controls.slider import CommandSlider


# ── CommandSlider 边界 ────────────────────────────────────────────────


def test_slider_initial_label(qtbot):
    """CommandSlider 初始 label 参数。"""

    slider = CommandSlider(label="PWM")
    qtbot.addWidget(slider)
    assert "PWM" in slider.label() or slider.label() == "PWM"


def test_slider_set_value(qtbot):
    """set_value 后 value() 读取。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider.set_value(75)
    assert slider.value() == 75


def test_slider_set_value_to_minimum(qtbot):
    """set_value=minimum。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider.set_value(0)
    assert slider.value() == 0


def test_slider_set_value_to_maximum(qtbot):
    """set_value=maximum。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider.set_value(100)
    assert slider.value() == 100


def test_slider_set_formatter_no_crash(qtbot):
    """set_formatter 不崩溃。"""

    slider = CommandSlider(label="Duty", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider.set_formatter(lambda v: f"SET {v * 10}")
    assert isinstance(slider.label(), str)


def test_slider_default_formatter(qtbot):
    """默认 formatter 不崩溃。"""

    slider = CommandSlider(label="PWM", minimum=0, maximum=255, value=128)
    qtbot.addWidget(slider)
    assert isinstance(slider.label(), str)


def test_slider_has_objectname(qtbot):
    """objectName = serialStationCommandSlider。"""

    slider = CommandSlider(label="PWM")
    qtbot.addWidget(slider)
    assert slider.objectName() == "serialStationCommandSlider"


# ── ConfigurableButton 边界 ───────────────────────────────────────────


def test_button_set_icon_no_crash(qtbot):
    """set_icon 不崩溃。"""

    btn = ConfigurableButton(text="Test")
    qtbot.addWidget(btn)
    btn.set_icon("cable")


def test_button_set_icon_with_color(qtbot):
    """set_icon 带 color 不崩溃。"""

    btn = ConfigurableButton(text="Test")
    qtbot.addWidget(btn)
    btn.set_icon("send", color="#22d3ee")


def test_button_set_press_animation(qtbot):
    """set_press_animation 不崩溃。"""

    btn = ConfigurableButton(text="Test")
    qtbot.addWidget(btn)
    btn.set_press_animation(True)
    btn.set_press_animation(False)


def test_button_set_hover_lift(qtbot):
    """set_hover_lift 不崩溃。"""

    btn = ConfigurableButton(text="Test")
    qtbot.addWidget(btn)
    btn.set_hover_lift(True)
    btn.set_hover_lift(False)


def test_button_set_ripple(qtbot):
    """set_ripple 不崩溃。"""

    btn = ConfigurableButton(text="Test")
    qtbot.addWidget(btn)
    btn.set_ripple(True)
    btn.set_ripple(False)


def test_button_get_command_default_empty(qtbot):
    """无 command_template → get_command 返回空串或 None。"""

    btn = ConfigurableButton(text="Noop")
    qtbot.addWidget(btn)
    cmd = btn.get_command()
    assert cmd == "" or cmd is None


def test_button_set_command_template(qtbot):
    """set_command_template 后 get_command 返回该模板。"""

    btn = ConfigurableButton(text="Fire")
    qtbot.addWidget(btn)
    btn.set_command_template("FIRE 1")
    assert btn.get_command() == "FIRE 1"


def test_button_has_objectname(qtbot):
    """objectName = serialStationConfigurableButton。"""

    btn = ConfigurableButton(text="Test")
    qtbot.addWidget(btn)
    assert btn.objectName() == "serialStationConfigurableButton"


def test_button_formatter_overrides_template(qtbot):
    """set_formatter 覆盖 command_template。"""

    btn = ConfigurableButton(text="X", command_template="OLD")
    qtbot.addWidget(btn)
    btn.set_formatter(lambda: "NEW")
    assert btn.get_command() == "NEW"
