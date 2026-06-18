"""UI 动画效果测试：折叠/淡入淡出/抖动/脉冲 + AnimationController。

从 test_ui_animations 拆出（守 250 行门禁）。覆盖 animations 模块的效果类
动画与生命周期控制器。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLabel, QWidget

from embeddebug.serial_station.ui.animations import (
    AnimationController,
    AnimationTokens,
    CollapseAnimation,
    CollapsiblePanel,
    FadeTransition,
    PulseAnimation,
    ScaleAnimation,
    ShakeAnimation,
)


# CollapseAnimation + CollapsiblePanel
def test_collapse_expand(qtbot):
    w = QWidget()
    w.setMaximumHeight(200)
    qtbot.addWidget(w)
    anim = CollapseAnimation.expand(w, 300)
    assert anim.endValue() == 300
    assert anim.duration() == AnimationTokens.DURATION_NORMAL


def test_collapse_to_zero(qtbot):
    w = QWidget()
    w.setMaximumHeight(200)
    qtbot.addWidget(w)
    anim = CollapseAnimation.collapse(w)
    assert anim.endValue() == 0


def test_collapsible_panel_toggle(qtbot):
    panel = CollapsiblePanel(title="test")
    panel.set_target_height(150)
    qtbot.addWidget(panel)
    assert panel.is_expanded is True
    panel.collapse()
    assert panel.is_expanded is False
    panel.expand()
    assert panel.is_expanded is True


def test_collapsible_panel_toggle_signal(qtbot):
    panel = CollapsiblePanel(title="test")
    qtbot.addWidget(panel)
    states: list[bool] = []
    panel.toggled.connect(lambda s: states.append(s))
    panel.toggle()
    panel.toggle()
    assert states == [False, True]


# FadeTransition
def test_fade_in(qtbot):
    w = QLabel("hello")
    qtbot.addWidget(w)
    anim = FadeTransition.fade_in(w)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert anim.startValue() == 0.0
    assert anim.endValue() == 1.0


def test_fade_out(qtbot):
    w = QLabel("bye")
    w.show()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_out(w)
    assert anim.startValue() == 1.0
    assert anim.endValue() == 0.0


def test_fade_in_custom_duration(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_in(w, 500)
    assert anim.duration() == 500


# ShakeAnimation
def test_shake_returns_animation(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w, amplitude=10, count=3)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert anim.endValue() == w.pos()


def test_shake_amplitude(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    orig = w.pos().x()
    anim = ShakeAnimation.shake(w, amplitude=15, count=2)
    offsets = []
    for i in range(10):
        v = anim.keyValueAt(i / 10)
        if v is not None:
            offsets.append(v.x() - orig)
    assert any(abs(o) > 0 for o in offsets)


# PulseAnimation
def test_pulse(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.pulse(w, 0.3, 1.0)
    assert anim.startValue() == 1.0
    assert anim.endValue() == 1.0
    assert anim.keyValueAt(0.5) == 0.3


def test_breathing_loops_forever(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.breathing(w)
    assert anim.loopCount() == -1


# AnimationController
def test_controller_add_and_stop(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    ctrl = AnimationController()
    ctrl.add(ScaleAnimation.press(w))
    ctrl.add(ShakeAnimation.shake(w))
    assert len(ctrl._animations) == 2
    ctrl.stop_all()
    assert len(ctrl._animations) == 0


def test_controller_parallel(qtbot):
    w1 = QWidget()
    w2 = QWidget()
    qtbot.addWidget(w1)
    qtbot.addWidget(w2)
    ctrl = AnimationController()
    group = ctrl.play_parallel([ScaleAnimation.press(w1), ShakeAnimation.shake(w2)])
    assert group.duration() > 0


def test_controller_sequential(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    ctrl = AnimationController()
    group = ctrl.play_sequential([ScaleAnimation.press(w), ShakeAnimation.shake(w)])
    assert group.duration() > 0


def test_controller_active_count(qtbot):
    ctrl = AnimationController()
    assert ctrl.active_count == 0
