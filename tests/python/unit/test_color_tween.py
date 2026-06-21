"""颜色补间动画（ColorTweenAnimation）单元测试。

覆盖：

- ``_parse_color``：``#RRGGBB`` / ``#RRGGBBAA`` / ``rgba(...)`` 三种格式及
  无效输入抛 ``ValueError``。
- ``_format_color``：与 ``_parse_color`` 的往返一致性。
- ``tween``：返回类型、默认时长 / 缓动 / 关键帧 / GC 防护注册与完成时移除。

测试范式对齐 ``test_skeleton.py``：offscreen Qt + ``qtbot.addWidget`` + 直接
校验 ``QVariantAnimation`` 的 duration / easingCurve / keyValues / state。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QVariantAnimation
from PyQt6.QtWidgets import QLabel

from embeddebug.serial_station.ui.animations.color_tween import ColorTweenAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


# ── _parse_color ────────────────────────────────────────────────────
def test_parse_color_hex_6():
    """``#RRGGBB`` 解析为 (r, g, b, 255)。"""

    assert ColorTweenAnimation._parse_color("#22d3ee") == (34, 211, 238, 255)


def test_parse_color_hex_8():
    """``#RRGGBBAA`` 解析为 (r, g, b, a)。"""

    assert ColorTweenAnimation._parse_color("#22d3eeff") == (34, 211, 238, 255)


def test_parse_color_rgba():
    """``rgba(r, g, b, 0.12)`` 解析时 a 视为 0-1 分数 → round(0.12*255)=31。"""

    assert ColorTweenAnimation._parse_color("rgba(34, 211, 238, 0.12)") == (
        34,
        211,
        238,
        31,
    )


def test_parse_color_invalid_raises():
    """不支持的格式应抛 ValueError。"""

    with pytest.raises(ValueError):
        ColorTweenAnimation._parse_color("not-a-color")


# ── _format_color ───────────────────────────────────────────────────
def test_format_color_roundtrip():
    """``_format_color`` 输出应能被 ``_parse_color`` 解析回相同元组。"""

    original = (34, 211, 238, 255)
    formatted = ColorTweenAnimation._format_color(original)
    assert formatted == "rgba(34, 211, 238, 255)"
    parsed = ColorTweenAnimation._parse_color(formatted)
    assert parsed == original


# ── tween 基础 ──────────────────────────────────────────────────────
def test_tween_smoke(qtbot):
    """``tween`` 应返回 QVariantAnimation 且不抛异常。"""

    label = QLabel("x")
    qtbot.addWidget(label)
    anim = ColorTweenAnimation.tween(label, "color", "#22d3ee", "#0ea5b7")
    assert isinstance(anim, QVariantAnimation)
    ColorTweenAnimation._active.clear()


def test_tween_duration_default(qtbot):
    """默认时长 == DURATION_NORMAL。"""

    label = QLabel("x")
    qtbot.addWidget(label)
    anim = ColorTweenAnimation.tween(label, "color", "#22d3ee", "#0ea5b7")
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    ColorTweenAnimation._active.clear()


def test_tween_easing_default(qtbot):
    """默认缓动 == EASE_IN_OUT（InOutCubic）。"""

    label = QLabel("x")
    qtbot.addWidget(label)
    anim = ColorTweenAnimation.tween(label, "color", "#22d3ee", "#0ea5b7")
    assert anim.easingCurve().type() == AnimationTokens.EASE_IN_OUT
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic
    ColorTweenAnimation._active.clear()


def test_tween_keyframes(qtbot):
    """关键帧应为 2 个，起点/终点为颜色元组。"""

    label = QLabel("x")
    qtbot.addWidget(label)
    anim = ColorTweenAnimation.tween(label, "color", "#22d3ee", "#0ea5b7")
    kvs = anim.keyValues()
    assert len(kvs) == 2
    # QVariant 可能将 tuple 序列化为 QVariantList（list 形式取回）；统一 tuple。
    start_v = tuple(kvs[0][1])
    end_v = tuple(kvs[1][1])
    assert start_v == (34, 211, 238, 255)
    assert end_v == (14, 165, 183, 255)
    ColorTweenAnimation._active.clear()


# ── tween GC 防护 ───────────────────────────────────────────────────
def test_tween_registered_for_gc(qtbot):
    """``tween`` 后动画应在 _active 中；finished 触发后应被移除。"""

    label = QLabel("x")
    qtbot.addWidget(label)
    ColorTweenAnimation._active.clear()
    # 用 DURATION_INSTANT 加快测试（避免接近 timeout）。
    anim = ColorTweenAnimation.tween(
        label,
        "color",
        "#22d3ee",
        "#0ea5b7",
        duration=AnimationTokens.DURATION_INSTANT,
    )
    assert anim in ColorTweenAnimation._active

    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped,
        timeout=3000,
    )
    assert anim not in ColorTweenAnimation._active
    ColorTweenAnimation._active.clear()
