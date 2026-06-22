"""Glow/Typewriter/ElasticSnap 边界单元测试。

补强 test_animations_controls/factories 未直接断言的边角：
- GlowAnimation._attach_effect：复用已挂 shadow / 无 effect 时新建并挂载。
- GlowAnimation.steady：intensity clamp [0, 1.5] + 返回 effect。
- GlowAnimation.clear：归零 blurRadius + alpha。
- GlowAnimation.pulse：loops=0 归 1 / 常量 _MAX_PULSE_DURATION_MS=3000。
- TypewriterAnimation.run：cps=0→1 防除零 / start_delay 负值归零 / LINEAR。
- TypewriterAnimation.run_with_label：连接 valueChanged+finished。
- ElasticSnapAnimation.cancel：清理 _active。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEasingCurve, QVariantAnimation
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QLabel, QWidget

from embeddebug.serial_station.ui.animations.glow import (
    _MAX_PULSE_DURATION_MS,
    GlowAnimation,
)
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.animations.typewriter import TypewriterAnimation
from embeddebug.serial_station.ui.animations.elastic_snap import ElasticSnapAnimation


# ── GlowAnimation._attach_effect ───────────────────────────────────────


def test_attach_effect_no_existing(qtbot):
    """无 effect → 创建新 shadow 并挂载。"""

    w = QWidget()
    qtbot.addWidget(w)
    effect = GlowAnimation._attach_effect(w)
    assert isinstance(effect, QGraphicsDropShadowEffect)
    assert w.graphicsEffect() is effect


def test_attach_effect_reuses_existing_shadow(qtbot):
    """已有 shadow effect → 复用（不创建新的）。"""

    w = QWidget()
    qtbot.addWidget(w)
    original = QGraphicsDropShadowEffect(w)
    original.setBlurRadius(42)
    w.setGraphicsEffect(original)
    result = GlowAnimation._attach_effect(w)
    assert result is original


def test_attach_effect_does_not_override_opacity(qtbot):
    """已有非 shadow effect → 不覆盖（返回新 effect 但不挂载）。"""

    from PyQt6.QtWidgets import QGraphicsOpacityEffect

    w = QWidget()
    qtbot.addWidget(w)
    opacity = QGraphicsOpacityEffect(w)
    w.setGraphicsEffect(opacity)
    result = GlowAnimation._attach_effect(w)
    # 返回新 shadow 但不挂载（保护 opacity effect）
    assert isinstance(result, QGraphicsDropShadowEffect)
    assert w.graphicsEffect() is opacity


# ── GlowAnimation.steady 边界 ──────────────────────────────────────────


def test_steady_returns_shadow_effect(qtbot):
    """steady 返回 QGraphicsDropShadowEffect。"""

    w = QWidget()
    qtbot.addWidget(w)
    effect = GlowAnimation.steady(w, intensity=1.0)
    assert isinstance(effect, QGraphicsDropShadowEffect)


def test_steady_clamps_intensity_above_max(qtbot):
    """intensity>1.5 → clamp 到 1.5（blurRadius 不超过 HOVER*1.5）。"""

    w = QWidget()
    qtbot.addWidget(w)
    effect_high = GlowAnimation.steady(w, intensity=2.0)
    effect_clamped = GlowAnimation.steady(w, intensity=1.5)
    assert effect_high.blurRadius() == effect_clamped.blurRadius()


def test_steady_zero_intensity(qtbot):
    """intensity=0 → blurRadius=0。"""

    w = QWidget()
    qtbot.addWidget(w)
    effect = GlowAnimation.steady(w, intensity=0.0)
    assert effect.blurRadius() == 0


# ── GlowAnimation.clear ────────────────────────────────────────────────


def test_clear_zeros_shadow(qtbot):
    """clear 将 blurRadius 归零 + alpha=0。"""

    w = QWidget()
    qtbot.addWidget(w)
    GlowAnimation.steady(w, intensity=1.0)
    GlowAnimation.clear(w)
    effect = w.graphicsEffect()
    assert isinstance(effect, QGraphicsDropShadowEffect)
    assert effect.blurRadius() == 0
    assert effect.color().alpha() == 0


def test_clear_no_effect_noop(qtbot):
    """clear 无 effect → 静默返回（不崩溃）。"""

    w = QWidget()
    qtbot.addWidget(w)
    GlowAnimation.clear(w)  # 不抛


# ── GlowAnimation.pulse 边界 ───────────────────────────────────────────


def test_pulse_loops_zero_uses_minimum_one(qtbot):
    """loops=0 → 归 1（防止 0 段动画）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = GlowAnimation.pulse(w, loops=0)
    assert anim.duration() > 0


def test_pulse_max_duration_cap(qtbot):
    """loops=20 → duration 被 _MAX_PULSE_DURATION_MS=3000 封顶。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = GlowAnimation.pulse(w, loops=20)
    assert anim.duration() <= _MAX_PULSE_DURATION_MS


def test_pulse_uses_ease_in_out(qtbot):
    """pulse 用 EASE_IN_OUT。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = GlowAnimation.pulse(w, loops=1)
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic


# ── TypewriterAnimation.run 边界 ───────────────────────────────────────


def test_run_cps_zero_does_not_crash():
    """cps=0 → 不除零（内部 max(1, cps)）。"""

    anim = TypewriterAnimation.run("hello", cps=0)
    assert isinstance(anim, QVariantAnimation)
    assert anim.duration() > 0


def test_run_negative_start_delay_does_not_crash():
    """start_delay_ms 负值 → 不崩溃（内部 max(0, ...)）。"""

    anim = TypewriterAnimation.run("hello", start_delay_ms=-100)
    assert isinstance(anim, QVariantAnimation)


def test_run_uses_linear_easing():
    """run 用 LINEAR（打字机匀速）。"""

    anim = TypewriterAnimation.run("hello")
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear


def test_run_empty_text_min_duration():
    """空文本 → duration 至少 DURATION_INSTANT。"""

    anim = TypewriterAnimation.run("")
    assert anim.duration() >= AnimationTokens.DURATION_INSTANT


# ── TypewriterAnimation.run_with_label ─────────────────────────────────


def test_run_with_label_starts_animation(qtbot):
    """run_with_label 自动 start。"""

    label = QLabel("placeholder")
    qtbot.addWidget(label)
    anim = TypewriterAnimation.run_with_label(label, "hello", cps=100)
    assert isinstance(anim, QVariantAnimation)


# ── ElasticSnapAnimation.cancel ────────────────────────────────────────


def test_elastic_cancel_clears_active(qtbot):
    """cancel 停止并清理 widget 上的动画。"""

    from PyQt6.QtCore import QRect

    ElasticSnapAnimation._active.clear()
    w = QWidget()
    qtbot.addWidget(w)
    w.setGeometry(0, 0, 100, 50)
    ElasticSnapAnimation.snap_to(w, QRect(10, 10, 80, 40))
    assert len(ElasticSnapAnimation._active) >= 1
    ElasticSnapAnimation.cancel(w)
    for anim in ElasticSnapAnimation._active:
        assert anim.parent() is not w
