"""elevation_effect 阴影工厂单元测试。

覆盖 ui/animations/elevation.py 的 elevation_effect 纯工厂函数：
- L0-L5 各级阴影参数正确映射（blur/offset_y/alpha）。
- 默认 level=L1。
- color=None 用黑色 + level alpha。
- 自定义 color 保留 RGB + 用 level alpha 覆盖。
- 返回 QGraphicsDropShadowEffect 实例。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import QGraphicsDropShadowEffect

from embeddebug.serial_station.ui.animations.elevation import elevation_effect
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


# ── 返回类型 ─────────────────────────────────────────────────────────────


def test_returns_qgraphicsshadoweffect():
    """elevation_effect 返回 QGraphicsDropShadowEffect 实例。"""

    effect = elevation_effect()
    assert isinstance(effect, QGraphicsDropShadowEffect)


# ── 默认 level=L1 ────────────────────────────────────────────────────────


def test_default_level_is_l1():
    """默认 level=L1 → blur=8, offset_y=1。"""

    effect = elevation_effect()
    blur, _, _ = AnimationTokens.ELEVATION_L1
    assert effect.blurRadius() == blur


# ── L0-L5 各级参数映射 ──────────────────────────────────────────────────


def test_l0_flat_no_blur():
    """L0=(0,0,0) → blur=0 无阴影。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L0)
    assert effect.blurRadius() == 0


def test_l1_card_static():
    """L1=(8,1,60) → blur=8, offset_y=1, alpha=60。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L1)
    blur, offset_y, alpha = AnimationTokens.ELEVATION_L1
    assert effect.blurRadius() == blur
    assert effect.yOffset() == offset_y
    assert effect.color().alpha() == alpha


def test_l2_drawer():
    """L2=(12,3,80) → blur=12。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L2)
    assert effect.blurRadius() == AnimationTokens.ELEVATION_L2[0]


def test_l3_hover():
    """L3=(16,3,120) → blur=16。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L3)
    assert effect.blurRadius() == AnimationTokens.ELEVATION_L3[0]


def test_l4_popover():
    """L4=(24,4,160) → blur=24。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L4)
    assert effect.blurRadius() == AnimationTokens.ELEVATION_L4[0]


def test_l5_modal():
    """L5=(32,6,200) → blur=32。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L5)
    assert effect.blurRadius() == AnimationTokens.ELEVATION_L5[0]


def test_all_levels_offset_x_is_zero():
    """所有 level offset_x=0（阴影只向下偏移）。"""

    for level in (
        AnimationTokens.ELEVATION_L0, AnimationTokens.ELEVATION_L1,
        AnimationTokens.ELEVATION_L2, AnimationTokens.ELEVATION_L3,
        AnimationTokens.ELEVATION_L4, AnimationTokens.ELEVATION_L5,
    ):
        effect = elevation_effect(level)
        assert effect.xOffset() == 0


def test_blur_increases_with_level():
    """blur 随层级递增（更深阴影）。"""

    blurs = [
        elevation_effect(l).blurRadius()
        for l in (
            AnimationTokens.ELEVATION_L0, AnimationTokens.ELEVATION_L1,
            AnimationTokens.ELEVATION_L2, AnimationTokens.ELEVATION_L3,
            AnimationTokens.ELEVATION_L4, AnimationTokens.ELEVATION_L5,
        )
    ]
    for i in range(len(blurs) - 1):
        assert blurs[i] <= blurs[i + 1]


# ── color=None 默认黑色 + alpha ─────────────────────────────────────────


def test_color_none_uses_black_with_alpha():
    """color=None → 黑色 (0,0,0) + level alpha。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L3)
    c = effect.color()
    assert c.red() == 0
    assert c.green() == 0
    assert c.blue() == 0
    assert c.alpha() == AnimationTokens.ELEVATION_L3[2]


def test_color_none_l0_alpha_zero():
    """L0 + color=None → alpha=0（完全透明，无阴影）。"""

    effect = elevation_effect(AnimationTokens.ELEVATION_L0)
    assert effect.color().alpha() == 0


# ── 自定义 color ────────────────────────────────────────────────────────


def test_custom_color_preserves_rgb():
    """自定义 color 保留 RGB 值。"""

    accent = QColor(34, 211, 238)
    effect = elevation_effect(AnimationTokens.ELEVATION_L3, color=accent)
    c = effect.color()
    assert c.red() == 34
    assert c.green() == 211
    assert c.blue() == 238


def test_custom_color_uses_level_alpha():
    """自定义 color 用 level 的 alpha 覆盖（不是 color 原有 alpha）。"""

    accent = QColor(255, 0, 0, 255)  # alpha=255
    effect = elevation_effect(AnimationTokens.ELEVATION_L3, color=accent)
    # L3 alpha=120，覆盖 color 的 255
    assert effect.color().alpha() == AnimationTokens.ELEVATION_L3[2]


def test_custom_color_l0_alpha_zero():
    """L0 + 自定义 color → alpha=0（level alpha 覆盖）。"""

    accent = QColor(255, 0, 0, 255)
    effect = elevation_effect(AnimationTokens.ELEVATION_L0, color=accent)
    assert effect.color().alpha() == 0


# ── 自定义 level 三元组 ─────────────────────────────────────────────────


def test_custom_level_tuple():
    """自定义三元组 (20, 5, 100)。"""

    effect = elevation_effect((20, 5, 100))
    assert effect.blurRadius() == 20
    assert effect.yOffset() == 5
    assert effect.color().alpha() == 100
