"""路径弹跳动画（BouncePathAnimation）单元测试。

覆盖：
- drop_in：返回 QPropertyAnimation、时长/缓动对齐、GC 防护注册、终点几何归位。
- slide_bounce：返回 QPropertyAnimation、时长/缓动对齐（EASE_OUT_BACK）。

测试范式对齐 ``test_scale_press.py``：offscreen Qt + qtbot.addWidget + 直接校验
``QPropertyAnimation`` 的 duration/easingCurve/keyValues/endValue。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.bounce_path import BouncePathAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def _make_widget(qtbot, w: int = 100, h: int = 40) -> QWidget:
    """构造一个有固定几何的 QWidget（geometry 动画需要实际尺寸）。"""

    wid = QWidget()
    wid.setGeometry(0, 0, w, h)
    qtbot.addWidget(wid)
    return wid


# ── drop_in 基础 ────────────────────────────────────────────────────
def test_drop_in_smoke(qtbot):
    """drop_in 应返回 QPropertyAnimation 且不抛异常。"""

    wid = _make_widget(qtbot)
    anim = BouncePathAnimation.drop_in(wid)
    assert isinstance(anim, QPropertyAnimation)
    BouncePathAnimation._active.clear()


def test_drop_in_duration_easing(qtbot):
    """drop_in 时长 == DURATION_SLOW，缓动 == EASE_OUT_BOUNCE。"""

    wid = _make_widget(qtbot)
    anim = BouncePathAnimation.drop_in(wid)
    assert anim.duration() == AnimationTokens.DURATION_SLOW
    assert anim.easingCurve().type() == QEasingCurve.Type.OutBounce
    BouncePathAnimation._active.clear()


# ── drop_in GC 防护 ─────────────────────────────────────────────────
def test_drop_in_registered_for_gc(qtbot):
    """drop_in 后动画应在 _active 中；finished 触发后应被移除。"""

    wid = _make_widget(qtbot)
    BouncePathAnimation._active.clear()
    anim = BouncePathAnimation.drop_in(wid)
    assert anim in BouncePathAnimation._active
    # 启动动画，等待完成（finished 信号触发 _discard）。
    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000
    )
    assert anim not in BouncePathAnimation._active
    BouncePathAnimation._active.clear()


# ── drop_in 终点几何 ────────────────────────────────────────────────
def test_drop_in_end_geometry_restored(qtbot):
    """动画跑完后 widget.geometry() 应等于原始几何（关键帧终点 == orig）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = BouncePathAnimation.drop_in(wid)
    # 直接快进到结束时刻，跳过事件循环。
    anim.setCurrentTime(anim.duration())
    assert wid.geometry() == orig
    BouncePathAnimation._active.clear()


def test_drop_in_end_value_is_original(qtbot):
    """endValue 应等于原 geometry（4 个关键帧的最后一段归位）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = BouncePathAnimation.drop_in(wid)
    end = anim.endValue()
    assert end is not None
    assert QRect(end) == orig
    BouncePathAnimation._active.clear()


# ── drop_in 中间关键帧 ──────────────────────────────────────────────
def test_drop_in_has_squash_keyframe(qtbot):
    """drop_in 应在 0.75 处设置挤压关键帧（高度 < 原，宽度 > 原）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = BouncePathAnimation.drop_in(wid)
    # keyValues 返回 [(position, value), ...]。
    kvs = anim.keyValues()
    # 找 0.75 位置的关键帧。
    squash_kv = next((v for p, v in kvs if abs(p - 0.75) < 0.01), None)
    assert squash_kv is not None, "expected squash keyframe at 0.75"
    squash_rect = QRect(squash_kv)
    assert squash_rect.height() < orig.height(), "squash should compress height"
    assert squash_rect.width() >= orig.width(), "squash should expand width"
    BouncePathAnimation._active.clear()


# ── slide_bounce 基础 ───────────────────────────────────────────────
def test_slide_bounce_smoke(qtbot):
    """slide_bounce 应返回 QPropertyAnimation，时长 == DURATION_NORMAL。"""

    wid = _make_widget(qtbot)
    anim = BouncePathAnimation.slide_bounce(wid, from_x=-120)
    assert isinstance(anim, QPropertyAnimation)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    BouncePathAnimation._active.clear()


def test_slide_bounce_easing_out_back(qtbot):
    """slide_bounce 缓动应为 EASE_OUT_BACK（横向过冲回弹）。"""

    wid = _make_widget(qtbot)
    anim = BouncePathAnimation.slide_bounce(wid, from_x=-120)
    assert anim.easingCurve().type() == QEasingCurve.Type.OutBack
    BouncePathAnimation._active.clear()


def test_slide_bounce_start_from_x(qtbot):
    """slide_bounce 起点左上角 x 应等于 from_x。"""

    wid = _make_widget(qtbot)
    anim = BouncePathAnimation.slide_bounce(wid, from_x=-120)
    start = anim.startValue()
    assert start is not None
    assert QRect(start).x() == -120
    BouncePathAnimation._active.clear()


def test_slide_bounce_end_is_original(qtbot):
    """slide_bounce 终点应等于原 geometry（滑入归位）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = BouncePathAnimation.slide_bounce(wid, from_x=-120)
    end = anim.endValue()
    assert end is not None
    assert QRect(end) == orig
    BouncePathAnimation._active.clear()


# ── slide_bounce GC 防护 ────────────────────────────────────────────
def test_slide_bounce_registered_for_gc(qtbot):
    """slide_bounce 后动画应在 _active 中；完成应被移除。"""

    wid = _make_widget(qtbot)
    BouncePathAnimation._active.clear()
    anim = BouncePathAnimation.slide_bounce(wid, from_x=-120)
    assert anim in BouncePathAnimation._active
    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000
    )
    assert anim not in BouncePathAnimation._active
    BouncePathAnimation._active.clear()
