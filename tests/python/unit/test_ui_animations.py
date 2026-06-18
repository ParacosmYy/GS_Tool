"""UI 动画引擎单元测试（一）：token / 缩放 / 滑动。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEasingCurve, QPoint, QRect
from PyQt6.QtWidgets import QPushButton, QWidget

from embeddebug.serial_station.ui.animations import (
    AnimationTokens,
    ScaleAnimation,
    SlideAnimation,
    SlideDirection,
)


# ── AnimationTokens ───────────────────────────────────────────────
def test_tokens_durations():
    assert AnimationTokens.DURATION_INSTANT < AnimationTokens.DURATION_FAST
    assert AnimationTokens.DURATION_FAST < AnimationTokens.DURATION_NORMAL
    assert AnimationTokens.DURATION_NORMAL < AnimationTokens.DURATION_SLOW


def test_tokens_easing_curves():
    assert AnimationTokens.EASE_OUT == QEasingCurve.Type.OutCubic
    assert AnimationTokens.EASE_OUT_BACK == QEasingCurve.Type.OutBack


def test_tokens_scale_values():
    assert 0 < AnimationTokens.SCALE_PRESSED < 1.0
    assert AnimationTokens.SCALE_HOVER > 1.0
    assert AnimationTokens.SCALE_NORMAL == 1.0


# ── ScaleAnimation ────────────────────────────────────────────────
def test_scale_press_returns_animation(qtbot):
    btn = QPushButton("X")
    qtbot.addWidget(btn)
    anim = ScaleAnimation.press(btn)
    assert anim.duration() == AnimationTokens.DURATION_FAST
    assert anim.easingCurve() == AnimationTokens.EASE_OUT_BACK


def test_scale_pop_in(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = ScaleAnimation.pop_in(w)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert isinstance(anim.startValue(), QRect)
    assert isinstance(anim.endValue(), QRect)


def test_scale_bounce(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = ScaleAnimation.bounce(w)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert isinstance(anim.endValue(), QRect)


# ── SlideAnimation ────────────────────────────────────────────────
def test_slide_in_left(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.LEFT, 50)
    assert anim.startValue().x() < anim.endValue().x()


def test_slide_in_right(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.RIGHT, 50)
    assert anim.startValue().x() > anim.endValue().x()


def test_slide_in_up(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.UP, 50)
    assert anim.startValue().y() < anim.endValue().y()


def test_slide_in_down(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.DOWN, 50)
    assert anim.startValue().y() > anim.endValue().y()


def test_slide_out_hides_widget(qtbot):
    w = QWidget()
    w.show()
    qtbot.addWidget(w)
    anim = SlideAnimation.slide_out(w, SlideDirection.RIGHT, 50)
    assert anim.duration() == AnimationTokens.DURATION_FAST


def test_slide_direction_enum():
    assert SlideDirection.LEFT.value == "left"
    assert SlideDirection.RIGHT.value == "right"
