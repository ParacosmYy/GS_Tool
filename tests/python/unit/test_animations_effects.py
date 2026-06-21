"""效果类动画测试（合并自 color_tween/stagger/stagger_fade/cleanup/ui_animations_effects）。"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect
import re

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPauseAnimation, QVariantAnimation
from PyQt6.QtWidgets import QFrame, QLabel, QWidget

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
from embeddebug.serial_station.ui.animations.color_tween import ColorTweenAnimation
from embeddebug.serial_station.ui.animations.stagger import StaggerCoordinator
from embeddebug.serial_station.ui.panel_animations import stagger_fade


@pytest.fixture(autouse=True)
def _clean_active():
    ColorTweenAnimation._active.clear()
    StaggerCoordinator._active.clear()


def _label(qtbot):
    w = QLabel("x"); qtbot.addWidget(w); return w
def _pause():
    return QPauseAnimation(50)


# ColorTween
def test_parse_color():
    assert ColorTweenAnimation._parse_color("#22d3ee") == (34, 211, 238, 255)
    assert ColorTweenAnimation._parse_color("#22d3eeff") == (34, 211, 238, 255)
    assert ColorTweenAnimation._parse_color("rgba(34, 211, 238, 0.12)") == (34, 211, 238, 31)
    with pytest.raises(ValueError):
        ColorTweenAnimation._parse_color("not-a-color")


def test_format_color_roundtrip():
    o = (34, 211, 238, 255)
    f = ColorTweenAnimation._format_color(o)
    assert f == "rgba(34, 211, 238, 255)" and ColorTweenAnimation._parse_color(f) == o


def test_tween_smoke_and_duration(qtbot):
    anim = ColorTweenAnimation.tween(_label(qtbot), "color", "#22d3ee", "#0ea5b7")
    assert isinstance(anim, QVariantAnimation)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL


def test_tween_easing_and_keyframes(qtbot):
    anim = ColorTweenAnimation.tween(_label(qtbot), "color", "#22d3ee", "#0ea5b7")
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic
    kvs = anim.keyValues()
    assert len(kvs) == 2
    assert tuple(kvs[0][1]) == (34, 211, 238, 255)
    assert tuple(kvs[1][1]) == (14, 165, 183, 255)


def test_tween_registered_for_gc(qtbot):
    anim = ColorTweenAnimation.tween(
        _label(qtbot), "color", "#22d3ee", "#0ea5b7", duration=AnimationTokens.DURATION_INSTANT)
    assert anim in ColorTweenAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000)
    assert anim not in ColorTweenAnimation._active


# StaggerCoordinator
def test_stagger_step():
    assert StaggerCoordinator()._step_ms == 60
    assert StaggerCoordinator(step_ms=100)._step_ms == 100


def test_stagger_add_factory_does_not_start():
    n = []
    c = StaggerCoordinator(step_ms=1000)
    c.add(lambda: n.append(1) or QPauseAnimation(0))
    assert n == [] and len(c._factories) == 1


def test_stagger_start_calls_factories(qtbot):
    n = []
    c = StaggerCoordinator(step_ms=10)
    for _ in range(3): c.add(lambda: n.append(1) or _pause())
    c.start()
    qtbot.waitUntil(lambda: len(n) == 3, timeout=1000)


def test_stagger_emits_finished(qtbot):
    c = StaggerCoordinator(step_ms=10)
    for _ in range(3): c.add(_pause)
    with qtbot.waitSignal(c.finished, timeout=2000): c.start()
    assert len(c._anims) == 3


def test_stagger_cancel_stops_pending(qtbot):
    n = []
    c = StaggerCoordinator(step_ms=1000)
    for _ in range(3): c.add(lambda: n.append(1) or QPauseAnimation(0))
    c.start(); c.cancel(); qtbot.wait(300)
    assert n == []


def test_stagger_registered_for_gc(qtbot):
    c = StaggerCoordinator(step_ms=10); c.add(_pause)
    assert c not in StaggerCoordinator._active
    c.start(); assert c in StaggerCoordinator._active
    qtbot.waitUntil(lambda: c not in StaggerCoordinator._active, timeout=2000)


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


def test_collapsible_panel_toggle(qtbot):
    p = CollapsiblePanel(title="t"); p.set_target_height(150); qtbot.addWidget(p)
    assert p.is_expanded is True
    p.collapse(); assert p.is_expanded is False
    p.expand(); assert p.is_expanded is True


def test_collapsible_panel_toggle_signal(qtbot):
    p = CollapsiblePanel(title="t"); qtbot.addWidget(p)
    states: list[bool] = []
    p.toggled.connect(lambda s: states.append(s))
    p.toggle(); p.toggle()
    assert states == [False, True]


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
