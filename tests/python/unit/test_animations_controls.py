"""控件级动画测试：Glow（脉冲发光）。

合并自 test_glow.py。覆盖 GlowAnimation 的构造、缓动、时长、GC 防护与释放。
（RotateAnimation / SkeletonAnimation 已作为 dead code 删除，相关测试同步移除。）
"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QEasingCurve
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QLabel

from embeddebug.serial_station.ui.animations.glow import GlowAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


@pytest.fixture(autouse=True)
def _clean_active():
    GlowAnimation._active.clear()


def _label(qtbot):
    w = QLabel("test"); w.setGeometry(0, 0, 100, 40); qtbot.addWidget(w)
    return w


# GlowAnimation
def test_pulse_creates_animation(qtbot):
    from PyQt6.QtCore import QPropertyAnimation

    w = _label(qtbot)
    anim = GlowAnimation.pulse(w, loops=3)
    assert isinstance(anim, QPropertyAnimation)
    assert anim.duration() > 0
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic


def test_pulse_attaches_effect(qtbot):
    w = _label(qtbot)
    GlowAnimation.pulse(w)
    assert isinstance(w.graphicsEffect(), QGraphicsDropShadowEffect)


def test_pulse_registered_for_gc(qtbot):
    w = _label(qtbot)
    anim = GlowAnimation.pulse(w)
    assert anim in GlowAnimation._active
    anim.finished.emit()
    assert anim not in GlowAnimation._active


def test_pulse_duration_scales_with_loops(qtbot):
    w = _label(qtbot)
    d1 = GlowAnimation.pulse(w, loops=1).duration()
    d2 = GlowAnimation.pulse(w, loops=2).duration()
    assert d2 == 2 * d1


def test_steady_sets_constants(qtbot):
    w = _label(qtbot)
    effect = GlowAnimation.steady(w, intensity=0.5)
    assert isinstance(effect, QGraphicsDropShadowEffect)
    assert 0 < effect.blurRadius() < AnimationTokens.SHADOW_BLUR_HOVER


def test_clear_zeros_effect(qtbot):
    w = _label(qtbot)
    GlowAnimation.steady(w, intensity=1.0)
    assert w.graphicsEffect() is not None
    GlowAnimation.clear(w)
    effect = w.graphicsEffect()
    assert isinstance(effect, QGraphicsDropShadowEffect)
    assert effect.blurRadius() == 0


def test_pulse_multiple_widgets_independent(qtbot):
    w1 = _label(qtbot); w2 = _label(qtbot)
    GlowAnimation.pulse(w1, loops=1); GlowAnimation.pulse(w2, loops=1)
    e1, e2 = w1.graphicsEffect(), w2.graphicsEffect()
    assert isinstance(e1, QGraphicsDropShadowEffect) and isinstance(e2, QGraphicsDropShadowEffect)
    assert e1 is not e2
