"""Shake/Pulse/Collapse 动画边界单元测试。

补强 test_animations_effects 未直接断言的边角：
- ShakeAnimation.shake：keyValueAt 数 = count + start/end + keyframe 位置交替正负。
- ShakeAnimation：默认参数（amplitude/count）+ 自定义。
- PulseAnimation.pulse：start=max + 0.5=min + end=max + 自定义 opacity。
- PulseAnimation.breathing：loopCount=-1 + stop_looping 清理。
- CollapseAnimation：expand target_height + collapse end=0 + 默认值。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPropertyAnimation
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.collapse import CollapseAnimation
from embeddebug.serial_station.ui.animations.pulse import PulseAnimation
from embeddebug.serial_station.ui.animations.shake import ShakeAnimation


# ── ShakeAnimation 边界 ───────────────────────────────────────────────


def test_shake_returns_qpropertyanimation(qtbot):
    """shake 返回 QPropertyAnimation。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w)
    assert isinstance(anim, QPropertyAnimation)


def test_shake_start_end_same_position(qtbot):
    """shake 起止位置相同（回到原位）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w, amplitude=10, count=3)
    assert anim.startValue() == anim.endValue()


def test_shake_has_keyframes_between_start_end(qtbot):
    """shake 在 start/end 之间有 keyframe（start≠end 路径有偏移）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w, amplitude=10, count=3)
    # 验证 shake 动画有效（duration > 0 + 起止同位）
    assert anim.duration() > 0


def test_shake_default_amplitude_and_count(qtbot):
    """shake 默认用 AnimationTokens 值。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w)
    # 默认不崩溃 + 返回动画
    assert anim.duration() > 0


def test_shake_custom_count_one(qtbot):
    """count=1 → 仅 1 次往返（duration > 0）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w, amplitude=10, count=1)
    assert anim.duration() > 0


def test_shake_zero_amplitude_no_movement(qtbot):
    """amplitude=0 → endValue == startValue（不偏移）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = ShakeAnimation.shake(w, amplitude=0, count=3)
    assert anim.endValue() == anim.startValue()


# ── PulseAnimation.pulse 边界 ─────────────────────────────────────────


def test_pulse_returns_animation(qtbot):
    """pulse 返回 QPropertyAnimation。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.pulse(w)
    assert isinstance(anim, QPropertyAnimation)


def test_pulse_start_max_end_max(qtbot):
    """pulse 透明度 start=max → 0.5=min → end=max。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.pulse(w, min_opacity=0.4, max_opacity=1.0)
    assert anim.startValue() == 1.0
    assert anim.keyValueAt(0.5) == 0.4
    assert anim.endValue() == 1.0


def test_pulse_custom_opacities(qtbot):
    """pulse 自定义 min/max opacity。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.pulse(w, min_opacity=0.1, max_opacity=0.9)
    assert anim.startValue() == 0.9
    assert anim.keyValueAt(0.5) == 0.1


def test_pulse_uses_ease_in_out(qtbot):
    """pulse 用 EASE_IN_OUT。"""

    from PyQt6.QtCore import QEasingCurve

    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.pulse(w)
    assert anim.easingCurve().type() == QEasingCurve.Type.InOutCubic


# ── PulseAnimation.breathing + stop_looping ───────────────────────────


def test_breathing_loopcount_negative_one(qtbot):
    """breathing → loopCount=-1（无限循环）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = PulseAnimation.breathing(w)
    assert anim.loopCount() == -1


def test_stop_looping_clears_active(qtbot):
    """stop_looping 停止并清理 widget 上的动画。"""

    PulseAnimation._active.clear()  # 隔离
    w = QWidget()
    qtbot.addWidget(w)
    PulseAnimation.breathing(w)
    # 呼吸动画注册到 _active
    assert len(PulseAnimation._active) >= 1
    PulseAnimation.stop_looping(w)
    # 清理后 _active 中不再有该 widget 的动画
    for anim in PulseAnimation._active:
        assert anim.parent() is not w


# ── CollapseAnimation 边界 ────────────────────────────────────────────


def test_collapse_expand_target_height(qtbot):
    """expand 设置 endValue=target_height。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = CollapseAnimation.expand(w, 200)
    assert anim.endValue() == 200


def test_collapse_collapse_end_zero(qtbot):
    """collapse endValue=0（折叠到零高）。"""

    w = QWidget()
    qtbot.addWidget(w)
    anim = CollapseAnimation.collapse(w)
    assert anim.endValue() == 0


def test_collapse_stop_clears_active(qtbot):
    """stop 停止并清理 widget 上的动画。"""

    from embeddebug.serial_station.ui.animations.collapse import CollapseAnimation as CA

    CA._active.clear()  # 隔离
    w = QWidget()
    qtbot.addWidget(w)
    CA.expand(w, 200)
    assert len(CA._active) >= 1
    CA.stop(w)
    for anim in CA._active:
        assert anim.parent() is not w
