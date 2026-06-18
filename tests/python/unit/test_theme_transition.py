"""主题切换 windowOpacity 过渡动画测试。

覆盖：
- ``ThemeTransition.run`` 创建动画组、目标属性是 ``windowOpacity``、起止 1.0/1.0。
- ``apply_fn`` 在暗淡段 finished 时被调用（换 QSS 的时机）。
- 防重入：上一次未完成时第二次调用返回 None，且 ``transition_theme`` 仍同步执行 apply_fn。
- 常量节奏对齐（OPACITY_DIP / DIP_MS / RISE_MS 量级）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation, QPropertyAnimation, QSequentialAnimationGroup
from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.theme.theme_transition import (
    DIP_MS,
    OPACITY_DIP,
    RISE_MS,
    ThemeTransition,
    transition_theme,
)


def test_opacity_dip_and_durations_in_expected_range():
    assert 0.3 < OPACITY_DIP < 0.9
    assert 50 <= DIP_MS <= 300
    assert 50 <= RISE_MS <= 400
    assert DIP_MS + RISE_MS <= 600  # 总时长克制（< 600ms）


def test_run_creates_window_opacity_animation_group(qtbot):
    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()  # 防止前序测试残留
    called = {"n": 0}

    def apply_fn() -> None:
        called["n"] += 1

    group = ThemeTransition.run(app, apply_fn)
    try:
        assert group is not None
        assert isinstance(group, QSequentialAnimationGroup)
        # 子动画数 = 暗淡段 + 回亮段（apply_fn 接 dip.finished，不占独立段）。
        assert group.animationCount() == 2
        # 两段都是 windowOpacity 属性动画。
        for i in range(group.animationCount()):
            child = group.animationAt(i)
            assert isinstance(child, QPropertyAnimation)
            assert child.propertyName() == b"windowOpacity"
        # 起止：第一段 1.0->DIP，第二段 DIP->1.0。
        dip = group.animationAt(0)
        rise = group.animationAt(1)
        assert dip.startValue() == 1.0
        assert dip.endValue() == OPACITY_DIP
        assert rise.startValue() == OPACITY_DIP
        assert rise.endValue() == 1.0
    finally:
        if group is not None:
            group.stop()
        ThemeTransition._active.clear()


def test_apply_fn_called_on_dip_finished(qtbot):
    """apply_fn 应在暗淡段完成时被调用（即换 QSS 的时机）。"""

    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()
    called = {"n": 0}

    def apply_fn() -> None:
        called["n"] += 1

    group = ThemeTransition.run(app, apply_fn, dip_ms=1, rise_ms=1)
    assert group is not None
    try:
        # 驱动动画跑完（极短时长），等 finished。
        qtbot.waitUntil(lambda: group.state() == QAbstractAnimation.State.Stopped, timeout=2000)
        qtbot.wait(50)  # 让 dip.finished 信号处理完
    finally:
        group.stop()
        ThemeTransition._active.clear()
    assert called["n"] == 1


def test_run_returns_none_when_already_running(qtbot):
    """防重入：上一次过渡未完成时第二次 run 返回 None。"""

    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()

    group1 = ThemeTransition.run(app, lambda: None, dip_ms=500, rise_ms=500)
    assert group1 is not None
    try:
        assert ThemeTransition.is_running() is True
        group2 = ThemeTransition.run(app, lambda: None)
        assert group2 is None  # 防重入
    finally:
        group1.stop()
        ThemeTransition._active.clear()
    assert ThemeTransition.is_running() is False


def test_transition_theme_falls_back_to_sync_on_reentry(qtbot):
    """transition_theme 在防重入时仍同步执行 apply_fn（不丢操作）。"""

    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()

    # 先启动一个长过渡占住 _active。
    blocker = ThemeTransition.run(app, lambda: None, dip_ms=500, rise_ms=500)
    assert blocker is not None
    try:
        called = {"n": 0}

        def apply_fn() -> None:
            called["n"] += 1

        # 此时仍在进行 -> transition_theme 应同步执行 apply_fn 并返回 False。
        started = transition_theme(app, apply_fn)
        assert started is False
        assert called["n"] == 1
    finally:
        blocker.stop()
        ThemeTransition._active.clear()
