"""页面切换滑入动画（PageSlideAnimation）单元测试。

覆盖：
- slide_in：四个方向均返回 QPropertyAnimation（smoke）。
- slide_in：时长 == DURATION_NORMAL、缓动 == OutQuart。
- slide_in：终点 == 原 geometry、LEFT 方向起点 x < 原位 x。
- slide_in：GC 防护注册（_active 注册 + finished 后移除）。

测试范式对齐 ``test_bounce_path.py``：offscreen Qt + qtbot.addWidget + 直接
校验 ``QPropertyAnimation`` 的 duration/easingCurve/startValue/endValue。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.page_slide import PageSlideAnimation
from embeddebug.serial_station.ui.animations.slide import SlideDirection
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

# 全部四个方向，参数化复用。
_ALL_DIRECTIONS = [
    SlideDirection.LEFT,
    SlideDirection.RIGHT,
    SlideDirection.UP,
    SlideDirection.DOWN,
]


def _make_widget(qtbot, w: int = 100, h: int = 40) -> QWidget:
    """构造一个有固定几何的 QWidget（geometry 动画需要实际尺寸）。"""

    wid = QWidget()
    wid.setGeometry(20, 30, w, h)
    qtbot.addWidget(wid)
    return wid


# ── slide_in 基础 smoke ────────────────────────────────────────────
@pytest.mark.parametrize("direction", _ALL_DIRECTIONS)
def test_slide_in_smoke(qtbot, direction):
    """slide_in 对每个方向都应返回 QPropertyAnimation 且不抛异常。"""

    wid = _make_widget(qtbot)
    anim = PageSlideAnimation.slide_in(wid, direction)
    assert isinstance(anim, QPropertyAnimation)
    PageSlideAnimation._active.clear()


# ── slide_in 时长 ──────────────────────────────────────────────────
@pytest.mark.parametrize("direction", _ALL_DIRECTIONS)
def test_slide_in_duration(qtbot, direction):
    """slide_in 时长应 == DURATION_NORMAL（240ms）。"""

    wid = _make_widget(qtbot)
    anim = PageSlideAnimation.slide_in(wid, direction)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    PageSlideAnimation._active.clear()


# ── slide_in 缓动 ──────────────────────────────────────────────────
@pytest.mark.parametrize("direction", _ALL_DIRECTIONS)
def test_slide_in_easing(qtbot, direction):
    """slide_in 缓动应 == OutQuart（减速至停止）。"""

    wid = _make_widget(qtbot)
    anim = PageSlideAnimation.slide_in(wid, direction)
    assert anim.easingCurve().type() == QEasingCurve.Type.OutQuart
    PageSlideAnimation._active.clear()


# ── slide_in 终点几何 ──────────────────────────────────────────────
@pytest.mark.parametrize("direction", _ALL_DIRECTIONS)
def test_slide_in_end_is_original(qtbot, direction):
    """endValue 应等于控件原 geometry（滑入归位）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, direction)
    end = anim.endValue()
    assert end is not None
    assert QRect(end) == orig
    PageSlideAnimation._active.clear()


# ── slide_in LEFT 起点偏移 ─────────────────────────────────────────
def test_slide_in_left_start_offset_negative(qtbot):
    """LEFT 方向起点 QRect 的 x 应 < 原位 x（控件移到原位左侧屏幕外）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.LEFT)
    start = anim.startValue()
    assert start is not None
    start_rect = QRect(start)
    assert start_rect.x() < orig.x(), "LEFT start x should be < original x"
    # 尺寸应保持不变。
    assert start_rect.size() == orig.size()
    PageSlideAnimation._active.clear()


def test_slide_in_left_start_offset_exact(qtbot):
    """LEFT 方向起点 x 应精确 == 原位 x − 控件宽度。"""

    wid = _make_widget(qtbot, w=100, h=40)
    orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.LEFT)
    start_rect = QRect(anim.startValue())
    assert start_rect.x() == orig.x() - orig.width()
    PageSlideAnimation._active.clear()


# ── slide_in UP 起点偏移（纵向对称校验）────────────────────────────
def test_slide_in_up_start_offset_negative(qtbot):
    """UP 方向起点 QRect 的 y 应 < 原位 y（控件移到原位上方屏幕外）。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.UP)
    start_rect = QRect(anim.startValue())
    assert start_rect.y() < orig.y(), "UP start y should be < original y"
    assert start_rect.size() == orig.size()
    PageSlideAnimation._active.clear()


# ── slide_in GC 防护 ───────────────────────────────────────────────
def test_slide_in_registered_for_gc(qtbot):
    """slide_in 后动画应在 _active 中；finished 触发后应被移除。"""

    wid = _make_widget(qtbot)
    PageSlideAnimation._active.clear()
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.LEFT)
    assert anim in PageSlideAnimation._active
    # 启动动画，等待完成（finished 信号触发 _discard）。
    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000
    )
    assert anim not in PageSlideAnimation._active
    PageSlideAnimation._active.clear()


# ── slide_in 跑完后几何归位 ────────────────────────────────────────
def test_slide_in_end_geometry_restored(qtbot):
    """动画快进到结束时刻后 widget.geometry() 应 == 原始几何。"""

    wid = _make_widget(qtbot)
    orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.RIGHT)
    anim.setCurrentTime(anim.duration())
    assert wid.geometry() == orig
    PageSlideAnimation._active.clear()


# ── slide_in 默认方向 ──────────────────────────────────────────────
def test_slide_in_default_direction_is_left(qtbot):
    """不传 direction 时默认 LEFT，起点应在原位左侧。"""

    wid = _make_widget(qtbot)
    anim_default = PageSlideAnimation.slide_in(wid)
    anim_left = PageSlideAnimation.slide_in(wid, SlideDirection.LEFT)
    assert QRect(anim_default.startValue()) == QRect(anim_left.startValue())
    PageSlideAnimation._active.clear()
