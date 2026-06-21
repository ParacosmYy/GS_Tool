"""骨架屏闪烁动画（SkeletonAnimation）单元测试。

覆盖：
- shimmer：返回 QVariantAnimation、时长/缓动/循环次数/关键帧/GC 防护注册。

测试范式对齐 ``test_bounce_path.py``：offscreen Qt + qtbot.addWidget + 直接校验
``QVariantAnimation`` 的 duration/easingCurve/keyValues/loopCount/state。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QVariantAnimation
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.skeleton import SkeletonAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def _make_widget(qtbot, w: int = 100, h: int = 40) -> QWidget:
    """构造一个有固定几何的 QWidget。"""

    wid = QWidget()
    wid.setGeometry(0, 0, w, h)
    qtbot.addWidget(wid)
    return wid


# ── shimmer 基础 ────────────────────────────────────────────────────
def test_shimmer_smoke(qtbot):
    """shimmer 应返回 QVariantAnimation 且不抛异常。"""

    wid = _make_widget(qtbot)
    anim = SkeletonAnimation.shimmer(wid)
    assert isinstance(anim, QVariantAnimation)
    SkeletonAnimation._active.clear()


def test_shimmer_duration(qtbot):
    """shimmer 单次时长 == DURATION_SLOWER（呼吸灯节奏）。"""

    wid = _make_widget(qtbot)
    anim = SkeletonAnimation.shimmer(wid)
    assert anim.duration() == AnimationTokens.DURATION_SLOWER
    SkeletonAnimation._active.clear()


def test_shimmer_default_loops(qtbot):
    """shimmer 默认 loopCount == 3（SHIMMER_DEFAULT_LOOPS）。"""

    wid = _make_widget(qtbot)
    anim = SkeletonAnimation.shimmer(wid)
    assert anim.loopCount() == 3
    SkeletonAnimation._active.clear()


def test_shimmer_custom_loops(qtbot):
    """传入 loops=5 时 loopCount == 5。"""

    wid = _make_widget(qtbot)
    anim = SkeletonAnimation.shimmer(wid, loops=5)
    assert anim.loopCount() == 5
    SkeletonAnimation._active.clear()


# ── shimmer 缓动与关键帧 ────────────────────────────────────────────
def test_shimmer_easing(qtbot):
    """shimmer 缓动 == EASE_IN_OUT（InOutCubic），对称呼吸。"""

    wid = _make_widget(qtbot)
    anim = SkeletonAnimation.shimmer(wid)
    assert anim.easingCurve().type() == AnimationTokens.EASE_IN_OUT
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic
    SkeletonAnimation._active.clear()


def test_shimmer_keyframes(qtbot):
    """shimmer 应有 3 个关键帧，位置 0.0/0.5/1.0，值 0.3/1.0/0.3。"""

    wid = _make_widget(qtbot)
    anim = SkeletonAnimation.shimmer(wid)
    kvs = anim.keyValues()
    assert len(kvs) == 3
    positions = [p for p, _ in kvs]
    values = [v for _, v in kvs]
    assert positions == pytest.approx([0.0, 0.5, 1.0])
    assert values[0] == pytest.approx(0.3)
    assert values[1] == pytest.approx(1.0)
    assert values[2] == pytest.approx(0.3)
    SkeletonAnimation._active.clear()


# ── shimmer GC 防护 ─────────────────────────────────────────────────
def test_shimmer_registered_for_gc(qtbot):
    """shimmer 后动画应在 _active 中；finished 触发后应被移除。"""

    wid = _make_widget(qtbot)
    SkeletonAnimation._active.clear()
    # 用 1 次循环加快测试（避免默认 3 次循环 × 600ms 接近 timeout）。
    anim = SkeletonAnimation.shimmer(wid, loops=1)
    assert anim in SkeletonAnimation._active

    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped,
        timeout=3000,
    )
    assert anim not in SkeletonAnimation._active
    SkeletonAnimation._active.clear()
