"""按钮按下/释放的分离式 scale 弹性反馈测试。

覆盖（对齐验证清单「press→scale<1.0、release→scale==1.0」）：
- ScaleAnimation.press_down：动画终点 geometry < 原尺寸（scale < 1.0）。
- ScaleAnimation.press_up：动画终点 geometry == 原尺寸（scale == 1.0）。
- press_down / press_up 注册到 _active 防 GC（_track 范式）。
- install_scale_press：pressed 信号触发 press_down，released 触发 press_up。
- 时长/缓动对齐（press_down EASE_OUT 即时、press_up EASE_OUT_BACK 回弹）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QRect
from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.animations.scale import ScaleAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.micro_interactions import install_scale_press


def _make_button(qtbot, w: int = 100, h: int = 40) -> QPushButton:
    """构造一个有固定几何的按钮（geometry 缩放需要实际尺寸）。"""

    btn = QPushButton("test")
    btn.setGeometry(0, 0, w, h)
    qtbot.addWidget(btn)
    return btn


def _dim_scale(rect: QRect, orig: QRect) -> tuple[float, float]:
    """返回 (宽度比, 高度比)。geometry 缩放等比改 w/h，单维比 == scale factor。"""

    return (rect.width() / orig.width(), rect.height() / orig.height())


# ── press_down：scale < 1.0 ───────────────────────────────────────
def test_press_down_end_scale_below_one(qtbot):
    btn = _make_button(qtbot)
    orig = QRect(btn.geometry())
    anim = ScaleAnimation.press_down(btn)
    assert anim.endValue() is not None
    w_ratio, h_ratio = _dim_scale(anim.endValue(), orig)
    assert w_ratio < 1.0 and h_ratio < 1.0, "press_down end scale should be < 1.0 in both dims"
    # 每维应接近 SCALE_PRESS_DOWN（0.96），允许取整误差（40*0.96=38.4→38 偏小）。
    assert 0.93 < w_ratio < 0.99
    assert 0.93 < h_ratio < 0.99


def test_press_down_center_anchored(qtbot):
    """陷下应保持中心点不变（仅改尺寸，不位移）。"""

    btn = _make_button(qtbot)
    orig_center = QRect(btn.geometry()).center()
    anim = ScaleAnimation.press_down(btn)
    assert anim.endValue().center() == orig_center


def test_press_down_uses_ease_out_and_instant_duration(qtbot):
    """press_down 应即时陷下（EASE_OUT + DURATION_INSTANT）。"""

    btn = _make_button(qtbot)
    anim = ScaleAnimation.press_down(btn)
    assert anim.easingCurve().type == QEasingCurve.Type.OutCubic
    assert anim.duration() == AnimationTokens.DURATION_INSTANT


# ── press_up：scale == 1.0 ────────────────────────────────────────
def test_press_up_end_scale_is_one(qtbot):
    btn = _make_button(qtbot)
    # 先 press_down 让按钮处于陷下态（press_up 从陷下态回弹）。
    ScaleAnimation.press_down(btn).start()
    # press_up 的终点应恢复到原全尺寸（每维 scale == 1.0）。
    anim = ScaleAnimation.press_up(btn)
    assert anim.endValue() is not None
    # 终点几何应 == 原始几何（100×40）。press_up 通过 ÷SCALE_PRESS_DOWN 反算原尺寸。
    assert anim.endValue().width() == 100
    assert anim.endValue().height() == 40


def test_press_up_restores_original_geometry(qtbot):
    btn = _make_button(qtbot)
    orig = QRect(btn.geometry())
    anim = ScaleAnimation.press_up(btn)
    assert anim.endValue() == orig


def test_press_up_uses_ease_out_back(qtbot):
    """press_up 应 OutBack 回弹（轻微过冲）。"""

    btn = _make_button(qtbot)
    anim = ScaleAnimation.press_up(btn)
    assert anim.easingCurve().type == QEasingCurve.Type.OutBack


# ── GC 防护（_track 范式） ────────────────────────────────────────
def test_press_down_registered_in_active(qtbot):
    btn = _make_button(qtbot)
    ScaleAnimation._active.clear()
    anim = ScaleAnimation.press_down(btn)
    assert anim in ScaleAnimation._active
    anim.stop()
    ScaleAnimation._active.clear()


def test_press_up_discarded_after_finished(qtbot):
    btn = _make_button(qtbot)
    ScaleAnimation._active.clear()
    anim = ScaleAnimation.press_up(btn)
    assert anim in ScaleAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    # finished 信号触发 _discard，从 _active 移除。
    assert anim not in ScaleAnimation._active


# ── install_scale_press 接线 ──────────────────────────────────────
def test_install_scale_press_connects_pressed_and_released(qtbot):
    """pressed/released 接线验证：emit 信号后应产生对应动画（间接证明已连接）。"""

    btn = _make_button(qtbot)
    install_scale_press(btn)

    ScaleAnimation._active.clear()
    btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1, "pressed should trigger press_down"
    ScaleAnimation._active.clear()

    btn.released.emit()
    assert len(ScaleAnimation._active) >= 1, "released should trigger press_up"
    ScaleAnimation._active.clear()


def test_install_scale_press_pressed_triggers_press_down(qtbot):
    """模拟 pressed 信号 → 应产生一个 press_down 动画（陷下）。"""

    btn = _make_button(qtbot)
    install_scale_press(btn)
    ScaleAnimation._active.clear()
    btn.pressed.emit()  # 模拟手指按下
    # pressed 触发 press_down，注册到 _active。
    assert len(ScaleAnimation._active) >= 1
    ScaleAnimation._active.clear()


def test_install_scale_press_released_triggers_press_up(qtbot):
    """模拟 released 信号 → 应产生一个 press_up 动画（回弹）。"""

    btn = _make_button(qtbot)
    install_scale_press(btn)
    ScaleAnimation._active.clear()
    btn.released.emit()  # 模拟手指松开
    assert len(ScaleAnimation._active) >= 1
    ScaleAnimation._active.clear()


def test_install_scale_press_full_press_release_cycle(qtbot):
    """完整按压周期：pressed 陷下（scale<1）→ released 回弹（scale==1）。"""

    btn = _make_button(qtbot)
    orig = QRect(btn.geometry())
    install_scale_press(btn)

    # 按下：press_down 跑完，按钮应处于陷下态（每维 scale < 1.0）。
    ScaleAnimation._active.clear()
    btn.pressed.emit()
    down_anim = ScaleAnimation._active[0]
    down_anim.start()
    qtbot.waitUntil(lambda: down_anim.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    w_d, h_d = _dim_scale(btn.geometry(), orig)
    assert w_d < 1.0 and h_d < 1.0, "after press_down, scale should be < 1.0"

    # 松开：press_up 跑完，按钮应回原态（每维 scale == 1.0，即原 100×40）。
    ScaleAnimation._active.clear()
    btn.released.emit()
    up_anim = ScaleAnimation._active[0]
    up_anim.start()
    qtbot.waitUntil(lambda: up_anim.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    assert btn.geometry().width() == orig.width()
    assert btn.geometry().height() == orig.height()
    ScaleAnimation._active.clear()
