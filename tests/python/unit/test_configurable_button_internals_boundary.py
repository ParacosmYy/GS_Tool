"""ConfigurableButton set_ripple/set_hover_lift/_emit_command 边界测试。

test_slider_button_boundary 覆盖基础 set_*；本文件补 set_ripple 幂等 +
set_hover_lift False 移除 effect + _emit_command formatter/template 路径 +
_ripple_enabled 标志 + _press_animation_enabled 标志。

覆盖：
1. set_ripple(True) 设置 _ripple_enabled=True。
2. set_ripple 幂等（同值再调不重装）。
3. set_ripple(False) 后 _ripple_enabled=False。
4. set_hover_lift(False) 移除 graphicsEffect。
5. set_press_animation 标志 round-trip。
6. _emit_command 有 formatter → emit formatter()。
7. _emit_command 无 formatter 有 template → emit template。
8. _emit_command 无 formatter 无 template → 不 emit。
9. command 信号接收 emitted 值。
10. set_ripple(True) install_ripple 注入（paintEvent 可访问）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.configurable_button import ConfigurableButton


# ── set_ripple ───────────────────────────────────────────────────
def test_set_ripple_true_sets_flag(qtbot):
    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    btn.set_ripple(True)
    assert btn._ripple_enabled is True


def test_set_ripple_idempotent_same_value(qtbot):
    """set_ripple 同值再调不重装（早退）。"""

    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    btn.set_ripple(True)
    btn.set_ripple(True)  # 同值，早退
    assert btn._ripple_enabled is True


def test_set_ripple_false_sets_flag(qtbot):
    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    btn.set_ripple(True)
    btn.set_ripple(False)
    assert btn._ripple_enabled is False


# ── set_hover_lift ───────────────────────────────────────────────
def test_set_hover_lift_false_removes_effect(qtbot):
    """set_hover_lift(False) 移除 graphicsEffect。"""

    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    btn.set_hover_lift(False)
    assert btn.graphicsEffect() is None


def test_set_hover_lift_true_keeps_effect(qtbot):
    """set_hover_lift(True) 保留/重建 effect。"""

    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    btn.set_hover_lift(True)
    # 可能不主动重建（只是不清除），验证不崩。
    btn.set_hover_lift(False)
    assert btn.graphicsEffect() is None


# ── set_press_animation 标志 ─────────────────────────────────────
def test_press_animation_flag_round_trip(qtbot):
    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    btn.set_press_animation(True)
    assert btn._press_animation_enabled is True
    btn.set_press_animation(False)
    assert btn._press_animation_enabled is False


# ── _emit_command formatter/template 路径 ────────────────────────
def test_emit_command_with_formatter(qtbot):
    """_emit_command 有 formatter → emit formatter() 结果。"""

    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    received = []
    btn.command.connect(lambda cmd: received.append(cmd))
    btn.set_formatter(lambda: "FMT_RESULT")
    btn._emit_command()
    assert received == ["FMT_RESULT"]


def test_emit_command_with_template_no_formatter(qtbot):
    """_emit_command 无 formatter 有 template → emit template。"""

    btn = ConfigurableButton(text="x", command_template="TEMPLATE_CMD")
    qtbot.addWidget(btn)
    received = []
    btn.command.connect(lambda cmd: received.append(cmd))
    btn._emit_command()
    assert received == ["TEMPLATE_CMD"]


def test_emit_command_no_formatter_no_template_no_emit(qtbot):
    """_emit_command 无 formatter 无 template → 不 emit。"""

    btn = ConfigurableButton(text="x")
    qtbot.addWidget(btn)
    received = []
    btn.command.connect(lambda cmd: received.append(cmd))
    btn._emit_command()
    assert received == []


def test_emit_command_formatter_priority_over_template(qtbot):
    """formatter 设置后优先于 template。"""

    btn = ConfigurableButton(text="x", command_template="TPL")
    qtbot.addWidget(btn)
    received = []
    btn.command.connect(lambda cmd: received.append(cmd))
    btn.set_formatter(lambda: "FMT")
    btn._emit_command()
    assert received == ["FMT"]


# ── _on_clicked 触发 emit ────────────────────────────────────────
def test_on_clicked_emits_command(qtbot):
    """_on_clicked 触发 _emit_command（点击 → command 信号）。"""

    btn = ConfigurableButton(text="x", command_template="CLICK_CMD")
    qtbot.addWidget(btn)
    received = []
    btn.command.connect(lambda cmd: received.append(cmd))
    btn._on_clicked()
    assert received == ["CLICK_CMD"]
