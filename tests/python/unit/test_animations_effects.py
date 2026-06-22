"""效果类动画测试（合并自 stagger_fade/cleanup/ui_animations_effects）。

（ColorTweenAnimation / StaggerCoordinator / CollapsiblePanel 已作为 dead code 删除，
相关测试同步移除；保留 stagger_fade / Collapse / Fade / Shake / Pulse / Controller。）
"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect
import re

from PyQt6.QtWidgets import QFrame, QLabel, QWidget

from embeddebug.serial_station.ui.animations import (
    AnimationController,
    AnimationTokens,
    CollapseAnimation,
    FadeTransition,
    PulseAnimation,
    ScaleAnimation,
    ShakeAnimation,
)
from embeddebug.serial_station.ui.panel_animations import stagger_fade


# stagger_fade
def test_stagger_fade_one_per_card(qtbot):
    cards = [QFrame() for _ in range(4)]
    for c in cards: qtbot.addWidget(c)
    assert len(stagger_fade(cards, delay_ms=50)) == 4


def test_stagger_fade_no_move(qtbot):
    card = QFrame(); qtbot.addWidget(card); card.move(100, 100)
    pos = card.pos(); stagger_fade([card], delay_ms=10)
    assert card.pos() == pos


def test_stagger_fade_empty_and_starts(qtbot):
    assert stagger_fade([], delay_ms=50) == []
    card = QFrame(); qtbot.addWidget(card)
    a = stagger_fade([card], delay_ms=10)
    assert len(a) == 1 and a[0].duration() > 0


def test_main_window_staggers_cards(qtbot):
    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow
    w = SerialStationMainWindow(); qtbot.addWidget(w)
    assert len(w._card_enter_anims) > 0


def test_main_window_and_panel_stagger_methods():
    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow
    from embeddebug.serial_station.ui import panel_animations
    src = inspect.getsource(SerialStationMainWindow)
    assert "_stagger_enter_cards" in src and "stagger_fade" in src
    assert callable(panel_animations.stagger_fade)


# stagger cleanup
def test_stagger_removed_fade_present_apis():
    from embeddebug.serial_station.ui import panel_animations
    assert not hasattr(panel_animations, "stagger")
    assert callable(panel_animations.stagger_fade)
    for name in ("card_enter", "fade_in", "fade_out", "slide_in"):
        assert hasattr(panel_animations, name)


def test_stagger_docstring_and_main_window():
    from embeddebug.serial_station.ui import panel_animations, main_window
    doc = inspect.getdoc(panel_animations)
    assert doc is not None and "stagger_fade" in doc
    assert "- ``stagger``：" not in doc
    src = inspect.getsource(main_window)
    assert "stagger_fade" in src
    assert re.findall(r"\bstagger\s*\(", src) == []


def test_stagger_fade_works(qtbot):
    cards = [QFrame() for _ in range(3)]
    for c in cards: qtbot.addWidget(c)
    assert len(stagger_fade(cards, delay_ms=50)) == 3


# 基础效果：Collapse / Fade / Shake / Pulse / Controller
def test_collapse(qtbot):
    w = QWidget(); w.setMaximumHeight(200); qtbot.addWidget(w)
    assert CollapseAnimation.expand(w, 300).endValue() == 300
    assert CollapseAnimation.collapse(w).endValue() == 0


def test_fade(qtbot):
    w1 = QLabel("hi"); qtbot.addWidget(w1)
    a1 = FadeTransition.fade_in(w1)
    assert a1.duration() == AnimationTokens.DURATION_NORMAL
    assert a1.startValue() == 0.0 and a1.endValue() == 1.0
    w2 = QLabel("bye"); w2.show(); qtbot.addWidget(w2)
    a2 = FadeTransition.fade_out(w2)
    assert a2.startValue() == 1.0 and a2.endValue() == 0.0
    assert FadeTransition.fade_in(QWidget(), 500).duration() == 500


def test_shake_returns_animation(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w, amplitude=10, count=3)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL and anim.endValue() == w.pos()


def test_shake_amplitude(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    orig = w.pos().x()
    anim = ShakeAnimation.shake(w, amplitude=15, count=2)
    offs = [anim.keyValueAt(i / 10).x() - orig for i in range(10) if anim.keyValueAt(i / 10)]
    assert any(abs(o) > 0 for o in offs)


def test_pulse_and_breathing(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    a = PulseAnimation.pulse(w, 0.3, 1.0)
    assert a.startValue() == 1.0 and a.endValue() == 1.0 and a.keyValueAt(0.5) == 0.3
    assert PulseAnimation.breathing(QWidget()).loopCount() == -1


def test_controller_lifecycle(qtbot):
    w1 = QWidget(); w2 = QWidget(); w = QWidget()
    for x in (w1, w2, w): qtbot.addWidget(x)
    ctrl = AnimationController()
    assert ctrl.active_count == 0
    ctrl.add(ScaleAnimation.press(w1)); ctrl.add(ShakeAnimation.shake(w2))
    assert len(ctrl._animations) == 2
    p = ctrl.play_parallel([ScaleAnimation.press(w1), ShakeAnimation.shake(w2)])
    assert p.duration() > 0
    s = ctrl.play_sequential([ScaleAnimation.press(w), ShakeAnimation.shake(w)])
    assert s.duration() > 0
    ctrl.stop_all()
    assert len(ctrl._animations) == 0
