"""CommandSlider _build_command + set_formatter + value 边界测试。

test_controls_led_slider 覆盖基础；本文件补 _build_command 格式化路径 +
formatter 优先级 + set_value 代码设值 + label() 返回值。

覆盖：
1. _build_command 用 command_template 格式化。
2. _build_command 用 formatter（覆盖 template）。
3. _build_command formatter 优先于 template。
4. set_value 后 value() 返回新值。
5. value() 初始默认值。
6. label() 返回标签文本。
7. set_formatter 后 _build_command 用 formatter。
8. _build_command 多次值格式化。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.slider import CommandSlider


# ── _build_command template 路径 ─────────────────────────────────
def test_build_command_uses_template(qtbot):
    """_build_command 用 command_template.format(value=...)。"""

    slider = CommandSlider(label="PWM", command_template="PWM {value}")
    qtbot.addWidget(slider)
    assert slider._build_command(42) == "PWM 42"


def test_build_command_template_with_hex(qtbot):
    """template 可含 hex 格式。"""

    slider = CommandSlider(label="addr", command_template="SET 0x{value:04X}")
    qtbot.addWidget(slider)
    cmd = slider._build_command(255)
    assert "00FF" in cmd


# ── _build_command formatter 路径 ────────────────────────────────
def test_build_command_uses_formatter(qtbot):
    """_build_command 用 formatter（覆盖 template）。"""

    slider = CommandSlider(label="x", command_template="T={value}")
    slider.set_formatter(lambda v: f"CUSTOM {v * 2}")
    qtbot.addWidget(slider)
    assert slider._build_command(5) == "CUSTOM 10"


def test_build_command_formatter_priority_over_template(qtbot):
    """formatter 设置后优先于 template。"""

    slider = CommandSlider(label="x", command_template="TEMPLATE {value}")
    qtbot.addWidget(slider)
    assert slider._build_command(1) == "TEMPLATE 1"
    slider.set_formatter(lambda v: f"FMT {v}")
    assert slider._build_command(1) == "FMT 1"


# ── set_value + value() ──────────────────────────────────────────
def test_set_value_updates_value(qtbot):
    """set_value 后 value() 返回新值。"""

    slider = CommandSlider(label="x", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider.set_value(75)
    assert slider.value() == 75


def test_value_initial_default(qtbot):
    """value() 初始默认值。"""

    slider = CommandSlider(label="x", minimum=0, maximum=100, value=30)
    qtbot.addWidget(slider)
    assert slider.value() == 30


def test_set_value_clamps_to_range(qtbot):
    """set_value 超范围 clamp（QSlider 行为）。"""

    slider = CommandSlider(label="x", minimum=0, maximum=100, value=50)
    qtbot.addWidget(slider)
    slider.set_value(200)  # 超过 maximum
    assert slider.value() <= 100


# ── label() ──────────────────────────────────────────────────────
def test_label_returns_text(qtbot):
    """label() 返回标签文本。"""

    slider = CommandSlider(label="Brightness")
    qtbot.addWidget(slider)
    assert slider.label() == "Brightness"


# ── _build_command 多次值 ────────────────────────────────────────
def test_build_command_multiple_values(qtbot):
    """_build_command 对不同值返回不同命令。"""

    slider = CommandSlider(label="x", command_template="V={value}")
    qtbot.addWidget(slider)
    assert slider._build_command(0) == "V=0"
    assert slider._build_command(100) == "V=100"
    assert slider._build_command(50) == "V=50"


# ── set_formatter 后 _build_command ──────────────────────────────
def test_set_formatter_changes_build_output(qtbot):
    """set_formatter 后 _build_command 输出改变。"""

    slider = CommandSlider(label="x", command_template="OLD {value}")
    qtbot.addWidget(slider)
    old = slider._build_command(10)
    slider.set_formatter(lambda v: f"NEW {v}")
    new = slider._build_command(10)
    assert old != new
    assert new == "NEW 10"
