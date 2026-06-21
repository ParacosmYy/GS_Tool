"""控件级动画测试：Glow / Rotate / Skeleton（shimmer）。

合并自 test_glow.py / test_rotate.py / test_skeleton.py。
覆盖脉冲发光、旋转、骨架屏呼吸的构造、缓动、时长、GC 防护与释放。
"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QVariantAnimation
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QLabel, QWidget

from embeddebug.serial_station.ui.animations.glow import GlowAnimation
from embeddebug.serial_station.ui.animations.rotate import RotateAnimation
from embeddebug.serial_station.ui.animations.skeleton import SkeletonAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


@pytest.fixture(autouse=True)
def _clean_active():
    for c in (GlowAnimation, RotateAnimation, SkeletonAnimation):
        c._active.clear()


def _label(qtbot):
    w = QLabel("test"); w.setGeometry(0, 0, 100, 40); qtbot.addWidget(w)
    return w


def _widget(qtbot, w=100, h=40):
    wid = QWidget(); wid.setGeometry(0, 0, w, h); qtbot.addWidget(wid)
    return wid


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


# RotateAnimation.spin
def test_spin_smoke():
    anim = RotateAnimation.spin()
    assert isinstance(anim, QVariantAnimation)
    RotateAnimation.release(anim)


def test_spin_values_range():
    anim = RotateAnimation.spin(duration_ms=800, loops=2, clockwise=True)
    assert anim.startValue() == 0.0
    assert anim.endValue() == 360.0 * 2
    RotateAnimation.release(anim)


def test_spin_direction():
    cw = RotateAnimation.spin(clockwise=True)
    ccw = RotateAnimation.spin(clockwise=False)
    assert cw.endValue() > 0, "clockwise should produce positive endValue"
    assert ccw.endValue() < 0, "counterclockwise should produce negative endValue"
    RotateAnimation.release(cw); RotateAnimation.release(ccw)


def test_spin_duration_scales_with_loops():
    base = RotateAnimation.spin(duration_ms=800, loops=1)
    triple = RotateAnimation.spin(duration_ms=800, loops=3)
    assert triple.duration() == 3 * base.duration() == 800 * 3
    RotateAnimation.release(base); RotateAnimation.release(triple)


def test_spin_easing_linear():
    anim = RotateAnimation.spin()
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear
    assert anim.easingCurve().type() == AnimationTokens.LINEAR
    RotateAnimation.release(anim)


def test_spin_loopcount_is_one():
    anim = RotateAnimation.spin(loops=4)
    assert anim.loopCount() == 1
    RotateAnimation.release(anim)


def test_spin_registered_for_gc(qtbot):
    anim = RotateAnimation.spin()
    assert anim in RotateAnimation._active
    anim.finished.emit()
    assert anim not in RotateAnimation._active


def test_discard_idempotent():
    anim = RotateAnimation.spin()
    RotateAnimation._discard(anim)
    assert anim not in RotateAnimation._active
    RotateAnimation._discard(anim)  # 幂等
    RotateAnimation._discard(anim)


# RotateAnimation.spin_continuous
def test_spin_continuous_infinite():
    anim = RotateAnimation.spin_continuous()
    assert anim.loopCount() == -1
    RotateAnimation.release(anim)


def test_spin_continuous_easing_linear():
    anim = RotateAnimation.spin_continuous()
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear
    RotateAnimation.release(anim)


def test_spin_continuous_registered():
    anim = RotateAnimation.spin_continuous()
    assert anim in RotateAnimation._active
    RotateAnimation.release(anim)
    assert anim not in RotateAnimation._active


def test_spin_continuous_direction():
    cw = RotateAnimation.spin_continuous(clockwise=True)
    ccw = RotateAnimation.spin_continuous(clockwise=False)
    assert cw.endValue() > 0 and ccw.endValue() < 0
    RotateAnimation.release(cw); RotateAnimation.release(ccw)


# RotateAnimation.release
def test_release_stops_and_removes():
    anim = RotateAnimation.spin()
    assert anim in RotateAnimation._active
    RotateAnimation.release(anim)
    assert anim not in RotateAnimation._active
    assert anim.state() == QAbstractAnimation.State.Stopped


def test_release_idempotent():
    anim = RotateAnimation.spin_continuous()
    RotateAnimation.release(anim)
    assert anim not in RotateAnimation._active
    RotateAnimation.release(anim)  # 幂等
    RotateAnimation.release(anim)


# SkeletonAnimation.shimmer
def test_shimmer_smoke(qtbot):
    anim = SkeletonAnimation.shimmer(_widget(qtbot))
    assert isinstance(anim, QVariantAnimation)


def test_shimmer_duration(qtbot):
    anim = SkeletonAnimation.shimmer(_widget(qtbot))
    assert anim.duration() == AnimationTokens.DURATION_SLOWER


def test_shimmer_default_loops(qtbot):
    assert SkeletonAnimation.shimmer(_widget(qtbot)).loopCount() == 3


def test_shimmer_custom_loops(qtbot):
    assert SkeletonAnimation.shimmer(_widget(qtbot), loops=5).loopCount() == 5


def test_shimmer_easing(qtbot):
    anim = SkeletonAnimation.shimmer(_widget(qtbot))
    assert anim.easingCurve().type() == AnimationTokens.EASE_IN_OUT
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic


def test_shimmer_keyframes(qtbot):
    anim = SkeletonAnimation.shimmer(_widget(qtbot))
    kvs = anim.keyValues()
    assert len(kvs) == 3
    positions = [p for p, _ in kvs]
    values = [v for _, v in kvs]
    assert positions == pytest.approx([0.0, 0.5, 1.0])
    assert values[0] == pytest.approx(0.3)
    assert values[1] == pytest.approx(1.0)
    assert values[2] == pytest.approx(0.3)


def test_shimmer_registered_for_gc(qtbot):
    anim = SkeletonAnimation.shimmer(_widget(qtbot), loops=1)
    assert anim in SkeletonAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000)
    assert anim not in SkeletonAnimation._active
