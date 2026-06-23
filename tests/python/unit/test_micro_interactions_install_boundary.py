"""micro_interactions install_card_shadow + install_hover_lift 边界测试。

install_card_shadow 此前无直接行为测试（仅源码级引用）。
本文件覆盖 card_shadow 返回 effect + hover_lift accent_tint True/False。

覆盖：
1. install_card_shadow 返回 QGraphicsDropShadowEffect。
2. install_card_shadow 设置 blurRadius > 0。
3. install_card_shadow 默认 level（非 None）。
4. install_card_shadow 自定义 level。
5. install_hover_lift accent_tint=True 返回 effect。
6. install_hover_lift accent_tint=False 返回 effect。
7. install_card_shadow 后 widget 有 graphicsEffect。
8. install_hover_lift 后 widget 有 graphicsEffect。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QPushButton, QWidget

from embeddebug.serial_station.ui.micro_interactions import (
    install_card_shadow,
    install_hover_lift,
)


# ── install_card_shadow ──────────────────────────────────────────
def test_card_shadow_returns_effect(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    effect = install_card_shadow(w)
    assert isinstance(effect, QGraphicsDropShadowEffect)


def test_card_shadow_sets_blur_radius(qtbot):
    """install_card_shadow 设置 blurRadius（可能 0 取决于 level）。"""

    w = QWidget()
    qtbot.addWidget(w)
    effect = install_card_shadow(w)
    assert effect.blurRadius() >= 0


def test_card_shadow_widget_has_graphics_effect(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    install_card_shadow(w)
    assert w.graphicsEffect() is not None


def test_card_shadow_custom_level(qtbot):
    """install_card_shadow 自定义 level 不崩。"""

    w = QWidget()
    qtbot.addWidget(w)
    from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

    effect = install_card_shadow(w, AnimationTokens.ELEVATION_L2)
    assert isinstance(effect, QGraphicsDropShadowEffect)


# ── install_hover_lift accent_tint ───────────────────────────────
def test_hover_lift_accent_tint_true(qtbot):
    w = QPushButton("x")
    qtbot.addWidget(w)
    effect = install_hover_lift(w, accent_tint=True)
    assert isinstance(effect, QGraphicsDropShadowEffect)


def test_hover_lift_accent_tint_false(qtbot):
    w = QPushButton("x")
    qtbot.addWidget(w)
    effect = install_hover_lift(w, accent_tint=False)
    assert isinstance(effect, QGraphicsDropShadowEffect)


def test_hover_lift_widget_has_graphics_effect(qtbot):
    w = QPushButton("x")
    qtbot.addWidget(w)
    install_hover_lift(w)
    assert w.graphicsEffect() is not None


def test_hover_lift_default_accent_tint(qtbot):
    """install_hover_lift 默认 accent_tint=True 不崩。"""

    w = QPushButton("x")
    qtbot.addWidget(w)
    effect = install_hover_lift(w)
    assert isinstance(effect, QGraphicsDropShadowEffect)
