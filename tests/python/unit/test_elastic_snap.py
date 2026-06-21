"""弹性吸附动画测试。

覆盖 ``ElasticSnapAnimation``：
- ``snap_to``：start/end 值、默认/自定义时长、OutElastic 缓动、目标 == 当前的无操作守卫。
- ``_track`` 防 GC：动画进入 ``_active``，``finished`` 后自动移除。
- ``snap_to_pos`` / ``snap_center_to`` 便捷重载构造正确的 ``target_rect``。
- ``cancel``：中断指定 widget 的进行中动画。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPoint, QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.elastic_snap import ElasticSnapAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def _make_widget(qtbot, x: int = 10, y: int = 20, w: int = 100, h: int = 80) -> QWidget:
    """构造一个有固定几何的控件（geometry 动画需要实际位置/尺寸）。"""

    widget = QWidget()
    widget.setGeometry(x, y, w, h)
    qtbot.addWidget(widget)
    return widget


# ── snap_to 基础 ──────────────────────────────────────────────────
def test_snap_to_smoke(qtbot):
    """snap_to 应返回 QPropertyAnimation 实例。"""

    widget = _make_widget(qtbot)
    target = QRect(200, 300, 100, 80)
    anim = ElasticSnapAnimation.snap_to(widget, target)
    assert isinstance(anim, QPropertyAnimation)
    ElasticSnapAnimation._active.clear()


def test_snap_to_start_end_values(qtbot):
    """startValue == 当前 geometry，endValue == target_rect。"""

    widget = _make_widget(qtbot)
    current_before = QRect(widget.geometry())
    target = QRect(200, 300, 100, 80)
    anim = ElasticSnapAnimation.snap_to(widget, target)
    assert anim.startValue() == current_before
    assert anim.endValue() == target
    ElasticSnapAnimation._active.clear()


def test_snap_to_duration_default(qtbot):
    """未传 duration_ms → 使用 DURATION_SLOWER（600ms）。"""

    widget = _make_widget(qtbot)
    target = QRect(200, 300, 100, 80)
    anim = ElasticSnapAnimation.snap_to(widget, target)
    assert anim.duration() == AnimationTokens.DURATION_SLOWER
    ElasticSnapAnimation._active.clear()


def test_snap_to_duration_custom(qtbot):
    """传 duration_ms=500 → duration == 500。"""

    widget = _make_widget(qtbot)
    target = QRect(200, 300, 100, 80)
    anim = ElasticSnapAnimation.snap_to(widget, target, duration_ms=500)
    assert anim.duration() == 500
    ElasticSnapAnimation._active.clear()


def test_snap_to_easing_elastic(qtbot):
    """缓动曲线 == EASE_OUT_ELASTIC。"""

    widget = _make_widget(qtbot)
    target = QRect(200, 300, 100, 80)
    anim = ElasticSnapAnimation.snap_to(widget, target)
    # QEasingCurve.type 是方法，须 .type() 调用取枚举。
    assert anim.easingCurve().type() == QEasingCurve.Type.OutElastic
    ElasticSnapAnimation._active.clear()


def test_snap_to_noop_when_already_at_target(qtbot):
    """target_rect == 当前 geometry → 返回 tracked 无操作动画（duration=1）。

    守卫：调用方永远拿到非 None 对象，但动画瞬时结束不产生视觉位移。
    """

    widget = _make_widget(qtbot)
    target = QRect(widget.geometry())  # 与当前完全一致
    anim = ElasticSnapAnimation.snap_to(widget, target)
    # 仍返回合法动画（非 None，且已 tracked）。
    assert isinstance(anim, QPropertyAnimation)
    assert anim in ElasticSnapAnimation._active
    # duration=1 表示无操作守卫已生效。
    assert anim.duration() == 1
    ElasticSnapAnimation._active.clear()


# ── GC 防护（_track 范式） ────────────────────────────────────────
def test_snap_to_registered_for_gc(qtbot):
    """snap_to 返回的动画应注册到 _active 防 GC，finished 后自动移除。"""

    widget = _make_widget(qtbot)
    target = QRect(200, 300, 100, 80)
    ElasticSnapAnimation._active.clear()
    anim = ElasticSnapAnimation.snap_to(widget, target)
    assert anim in ElasticSnapAnimation._active

    # 启动后等待 finished 触发 _discard。
    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped,
        timeout=3000,
    )
    assert anim not in ElasticSnapAnimation._active
    ElasticSnapAnimation._active.clear()


# ── 便捷重载 ──────────────────────────────────────────────────────
def test_snap_to_pos_convenience(qtbot):
    """snap_to_pos(x, y) 构造的 target_rect 左上角 == (x, y)，尺寸保持当前。"""

    widget = _make_widget(qtbot, w=120, h=60)
    anim = ElasticSnapAnimation.snap_to_pos(widget, 250, 350)
    end = anim.endValue()
    assert isinstance(end, QRect)
    assert end.x() == 250
    assert end.y() == 350
    assert end.width() == 120
    assert end.height() == 60
    ElasticSnapAnimation._active.clear()


def test_snap_center_to_convenience(qtbot):
    """snap_center_to(cx, cy) 构造的 target_rect 中心 == (cx, cy)，尺寸保持当前。"""

    widget = _make_widget(qtbot, w=120, h=60)
    anim = ElasticSnapAnimation.snap_center_to(widget, 400, 500)
    end = anim.endValue()
    assert isinstance(end, QRect)
    assert end.center() == QPoint(400, 500)
    assert end.width() == 120
    assert end.height() == 60
    ElasticSnapAnimation._active.clear()


# ── cancel ────────────────────────────────────────────────────────
def test_cancel_stops_animations_for_widget(qtbot):
    """cancel(widget) 应停止所有作用于该 widget 的活跃动画。"""

    widget = _make_widget(qtbot)
    target = QRect(500, 600, 100, 80)
    ElasticSnapAnimation._active.clear()
    anim = ElasticSnapAnimation.snap_to(widget, target)
    anim.start()
    # 立即调用 cancel（动画仍在 Running）。
    assert anim.state() == QAbstractAnimation.State.Running or anim in ElasticSnapAnimation._active
    ElasticSnapAnimation.cancel(widget)
    assert anim.state() != QAbstractAnimation.State.Running
    # cancel 手动从 _active 移除。
    assert anim not in ElasticSnapAnimation._active
    ElasticSnapAnimation._active.clear()
