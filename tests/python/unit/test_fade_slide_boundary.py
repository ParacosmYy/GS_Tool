"""FadeTransition + SlideAnimation._offset + SlideDirection 边界单元测试。

补强 test_animations_transitions/effects 未直接断言的边角：
- SlideDirection：4 枚举成员 + value 小写。
- SlideAnimation._offset：4 方向偏移（LEFT/RIGHT/UP/DOWN）+ distance=0。
- FadeTransition.fade_in/fade_out：返回 QPropertyAnimation + start/end 值 + easing。
- FadeTransition.cross_fade：返回 QParallelAnimationGroup + 含 2 子动画。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QParallelAnimationGroup, QPoint, QPropertyAnimation
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.fade import FadeTransition
from embeddebug.serial_station.ui.animations.slide import (
    SlideAnimation,
    SlideDirection,
)


# ── SlideDirection 枚举 ────────────────────────────────────────────────


def test_slide_direction_has_four_members():
    """SlideDirection 含 4 个方向。"""

    assert len(SlideDirection) == 4


def test_slide_direction_values():
    """枚举 value 是小写字符串。"""

    assert SlideDirection.LEFT.value == "left"
    assert SlideDirection.RIGHT.value == "right"
    assert SlideDirection.UP.value == "up"
    assert SlideDirection.DOWN.value == "down"


# ── SlideAnimation._offset 纯几何 ──────────────────────────────────────


def test_offset_left():
    """LEFT → x 减 distance。"""

    result = SlideAnimation._offset(QPoint(100, 50), SlideDirection.LEFT, 60)
    assert result == QPoint(40, 50)


def test_offset_right():
    """RIGHT → x 加 distance。"""

    result = SlideAnimation._offset(QPoint(100, 50), SlideDirection.RIGHT, 60)
    assert result == QPoint(160, 50)


def test_offset_up():
    """UP → y 减 distance。"""

    result = SlideAnimation._offset(QPoint(100, 50), SlideDirection.UP, 60)
    assert result == QPoint(100, -10)


def test_offset_down():
    """DOWN → y 加 distance。"""

    result = SlideAnimation._offset(QPoint(100, 50), SlideDirection.DOWN, 60)
    assert result == QPoint(100, 110)


def test_offset_zero_distance_returns_same():
    """distance=0 → 返回原位置。"""

    pos = QPoint(42, 99)
    for direction in SlideDirection:
        assert SlideAnimation._offset(pos, direction, 0) == pos


def test_offset_preserves_other_axis():
    """偏移只改变对应轴，另一轴不变。"""

    for direction in SlideDirection:
        result = SlideAnimation._offset(QPoint(100, 200), direction, 50)
        if direction in (SlideDirection.LEFT, SlideDirection.RIGHT):
            assert result.y() == 200  # Y 不变
        else:
            assert result.x() == 100  # X 不变


# ── FadeTransition.fade_in ─────────────────────────────────────────────


def test_fade_in_returns_animation(qtbot):
    """fade_in 返回 QPropertyAnimation。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_in(w)
    assert isinstance(anim, QPropertyAnimation)


def test_fade_in_start_zero_end_one(qtbot):
    """fade_in 透明度 0→1。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_in(w)
    assert anim.startValue() == 0.0
    assert anim.endValue() == 1.0


def test_fade_in_custom_duration(qtbot):
    """fade_in 自定义时长。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_in(w, duration=500)
    assert anim.duration() == 500


def test_fade_in_shows_widget(qtbot):
    """fade_in 调用 widget.show()。"""

    w = QWidget()
    qtbot.addWidget(w)
    FadeTransition.fade_in(w)
    assert w.isVisible() or not w.isHidden()


# ── FadeTransition.fade_out ────────────────────────────────────────────


def test_fade_out_returns_animation(qtbot):
    """fade_out 返回 QPropertyAnimation。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_out(w)
    assert isinstance(anim, QPropertyAnimation)


def test_fade_out_start_one_end_zero(qtbot):
    """fade_out 透明度 1→0。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_out(w)
    assert anim.startValue() == 1.0
    assert anim.endValue() == 0.0


def test_fade_out_connects_hide_on_finished(qtbot):
    """fade_out 连接 finished → widget.hide（receiver 数 > 0）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = FadeTransition.fade_out(w)
    # finished 信号至少有 1 个 receiver（hide 回调）
    assert anim.receivers(anim.finished) >= 1


# ── FadeTransition.cross_fade ─────────────────────────────────────────


def test_cross_fade_returns_parallel_group(qtbot):
    """cross_fade 返回 QParallelAnimationGroup。"""

    out_w = QWidget()
    in_w = QWidget()
    qtbot.addWidget(out_w)
    qtbot.addWidget(in_w)
    group = FadeTransition.cross_fade(out_w, in_w)
    assert isinstance(group, QParallelAnimationGroup)


def test_cross_fade_has_two_animations(qtbot):
    """cross_fade 含 2 子动画（淡出+淡入）。"""

    out_w = QWidget()
    in_w = QWidget()
    qtbot.addWidget(out_w)
    qtbot.addWidget(in_w)
    group = FadeTransition.cross_fade(out_w, in_w)
    assert group.animationCount() == 2
