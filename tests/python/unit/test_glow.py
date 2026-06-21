"""GlowAnimation 单元测试：accent 色脉冲 / 持续发光 / 清除。

覆盖：
- pulse 返回 QPropertyAnimation、duration > 0、easing EASE_IN_OUT。
- pulse 附加 QGraphicsDropShadowEffect 到 widget。
- pulse 注册到 _active 防 GC，finished 后移除。
- pulse 总时长随 loops 线性扩展（loops=2 时长 == 2 × loops=1 时长）。
- steady 设置 blurRadius 在合理范围。
- clear 将 effect blurRadius 归零。
- 多个 widget 各自 pulse，effect 互相独立。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEasingCurve
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QLabel

from embeddebug.serial_station.ui.animations.glow import GlowAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def _make_widget(qtbot):
    """构造一个带固定几何的标签控件（发光需要可见控件载体）。"""

    w = QLabel("test")
    w.setGeometry(0, 0, 100, 40)
    qtbot.addWidget(w)
    return w


def test_pulse_creates_animation(qtbot):
    """pulse 返回 QPropertyAnimation，duration > 0，easing EASE_IN_OUT。"""

    w = _make_widget(qtbot)
    anim = GlowAnimation.pulse(w, loops=3)
    from PyQt6.QtCore import QPropertyAnimation

    assert isinstance(anim, QPropertyAnimation)
    assert anim.duration() > 0
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic


def test_pulse_attaches_effect(qtbot):
    """pulse 后 widget 应持有 QGraphicsDropShadowEffect。"""

    w = _make_widget(qtbot)
    GlowAnimation.pulse(w)
    assert isinstance(w.graphicsEffect(), QGraphicsDropShadowEffect)


def test_pulse_registered_for_gc(qtbot):
    """pulse 的动画注册到 _active；emit finished 后移除。"""

    w = _make_widget(qtbot)
    GlowAnimation._active.clear()
    anim = GlowAnimation.pulse(w)
    assert anim in GlowAnimation._active
    anim.finished.emit()
    assert anim not in GlowAnimation._active
    GlowAnimation._active.clear()


def test_pulse_duration_scales_with_loops(qtbot):
    """pulse(loops=2) 时长 == 2 × pulse(loops=1) 时长。"""

    w = _make_widget(qtbot)
    d1 = GlowAnimation.pulse(w, loops=1).duration()
    d2 = GlowAnimation.pulse(w, loops=2).duration()
    assert d2 == 2 * d1, f"duration should double with loops (got {d1} → {d2})"
    GlowAnimation._active.clear()


def test_steady_sets_constants(qtbot):
    """steady(intensity=0.5) 应返回 blurRadius 在 (0, SHADOW_BLUR_HOVER) 间的 effect。"""

    w = _make_widget(qtbot)
    effect = GlowAnimation.steady(w, intensity=0.5)
    assert isinstance(effect, QGraphicsDropShadowEffect)
    blur = effect.blurRadius()
    assert 0 < blur < AnimationTokens.SHADOW_BLUR_HOVER, (
        f"steady(0.5) blurRadius should be between 0 and "
        f"SHADOW_BLUR_HOVER ({AnimationTokens.SHADOW_BLUR_HOVER}), got {blur}"
    )


def test_clear_zeros_effect(qtbot):
    """clear 后，若 widget 持有 QGraphicsDropShadowEffect，其 blurRadius 归零。"""

    w = _make_widget(qtbot)
    GlowAnimation.steady(w, intensity=1.0)
    assert w.graphicsEffect() is not None
    GlowAnimation.clear(w)
    effect = w.graphicsEffect()
    assert isinstance(effect, QGraphicsDropShadowEffect)
    assert effect.blurRadius() == 0


def test_pulse_multiple_widgets_independent(qtbot):
    """两个 widget 顺序 pulse，各自持有独立的 effect。"""

    w1 = _make_widget(qtbot)
    w2 = _make_widget(qtbot)
    GlowAnimation.pulse(w1, loops=1)
    GlowAnimation.pulse(w2, loops=1)
    e1 = w1.graphicsEffect()
    e2 = w2.graphicsEffect()
    assert isinstance(e1, QGraphicsDropShadowEffect)
    assert isinstance(e2, QGraphicsDropShadowEffect)
    assert e1 is not e2, "two widgets should get independent effects"
    GlowAnimation._active.clear()
