"""RotateAnimation 单元测试。

覆盖：
- ``spin``：smoke、起止值范围、方向、时长随 loops 线性缩放、LINEAR 缓动。
- GC 防护（``_track`` 范式）：注册到 ``_active``，``finished`` 后自动移除。
- ``spin_continuous``：``loopCount == -1`` 无限循环。
- ``release``：停止运行中的动画 + 从 ``_active`` 移除。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QVariantAnimation

from embeddebug.serial_station.ui.animations.rotate import RotateAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


# ── spin：基础 smoke ───────────────────────────────────────────────
def test_spin_smoke():
    """spin() 返回 QVariantAnimation 实例，非 None。"""

    anim = RotateAnimation.spin()
    assert anim is not None
    assert isinstance(anim, QVariantAnimation)
    RotateAnimation.release(anim)


def test_spin_values_range():
    """默认参数：startValue=0.0，endValue=360.0 * loops。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin(duration_ms=800, loops=2, clockwise=True)
    assert anim.startValue() == 0.0
    assert anim.endValue() == 360.0 * 2
    RotateAnimation.release(anim)


def test_spin_direction():
    """顺时针 endValue > 0；逆时针 endValue < 0。"""

    RotateAnimation._active.clear()
    cw = RotateAnimation.spin(clockwise=True)
    ccw = RotateAnimation.spin(clockwise=False)
    assert cw.endValue() > 0, "clockwise should produce positive endValue"
    assert ccw.endValue() < 0, "counterclockwise should produce negative endValue"
    RotateAnimation.release(cw)
    RotateAnimation.release(ccw)


def test_spin_duration_scales_with_loops():
    """duration(loops=3) == 3 * duration(loops=1)（loops 烘焙进总时长）。"""

    RotateAnimation._active.clear()
    base = RotateAnimation.spin(duration_ms=800, loops=1)
    triple = RotateAnimation.spin(duration_ms=800, loops=3)
    assert triple.duration() == 3 * base.duration()
    assert triple.duration() == 800 * 3
    RotateAnimation.release(base)
    RotateAnimation.release(triple)


def test_spin_easing_linear():
    """spin 使用 LINEAR 缓动（匀速旋转）。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin()
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear
    assert anim.easingCurve().type() == AnimationTokens.LINEAR
    RotateAnimation.release(anim)


def test_spin_loopcount_is_one():
    """loops 烘焙进 endValue，QVariantAnimation 自身 loopCount 固定为 1。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin(loops=4)
    assert anim.loopCount() == 1
    RotateAnimation.release(anim)


# ── GC 防护（_track 范式） ────────────────────────────────────────
def test_spin_registered_for_gc(qtbot):
    """spin 后动画在 _active 中；emit finished 后被移除。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin()
    assert anim in RotateAnimation._active
    # 模拟动画完成：emit finished 触发 _discard。
    anim.finished.emit()
    assert anim not in RotateAnimation._active


def test_discard_idempotent():
    """_discard 对不在列表中的动画安全（不抛异常）。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin()
    RotateAnimation._discard(anim)
    assert anim not in RotateAnimation._active
    # 再次 discard 不抛 ValueError。
    RotateAnimation._discard(anim)
    RotateAnimation._discard(anim)  # 仍然安全


# ── spin_continuous：无限循环 ────────────────────────────────────
def test_spin_continuous_infinite():
    """spin_continuous 设置 loopCount == -1（无限）。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin_continuous()
    assert anim.loopCount() == -1
    RotateAnimation.release(anim)


def test_spin_continuous_easing_linear():
    """连续旋转使用 LINEAR（避免首尾速度不均）。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin_continuous()
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear
    RotateAnimation.release(anim)


def test_spin_continuous_registered():
    """spin_continuous 同样注册到 _active（需 release 清理）。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin_continuous()
    assert anim in RotateAnimation._active
    RotateAnimation.release(anim)
    assert anim not in RotateAnimation._active


def test_spin_continuous_direction():
    """连续旋转方向：顺时针 endValue > 0，逆时针 < 0。"""

    RotateAnimation._active.clear()
    cw = RotateAnimation.spin_continuous(clockwise=True)
    ccw = RotateAnimation.spin_continuous(clockwise=False)
    assert cw.endValue() > 0
    assert ccw.endValue() < 0
    RotateAnimation.release(cw)
    RotateAnimation.release(ccw)


# ── release：停止 + 移除 ──────────────────────────────────────────
def test_release_stops_and_removes():
    """release：停止运行中的动画 + 从 _active 移除。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin()
    assert anim in RotateAnimation._active
    # release 应移除。
    RotateAnimation.release(anim)
    assert anim not in RotateAnimation._active
    # release 后状态为 Stopped。
    assert anim.state() == QAbstractAnimation.State.Stopped


def test_release_idempotent():
    """release 重复调用安全（幂等）。"""

    RotateAnimation._active.clear()
    anim = RotateAnimation.spin_continuous()
    RotateAnimation.release(anim)
    assert anim not in RotateAnimation._active
    # 再次 release 不抛异常。
    RotateAnimation.release(anim)
    RotateAnimation.release(anim)
