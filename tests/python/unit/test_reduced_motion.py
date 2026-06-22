"""ReducedMotionState 单元测试 — 全局可访问性偏好状态机。

直接 import 子模块（不依赖 animations/__init__.py 的 re-export，避免环境回退）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEasingCurve

from embeddebug.serial_station.ui.animations.reduced_motion import (
    ReducedMotionState,
    apply_reduced_duration,
    apply_reduced_easing,
    reduced_motion_enabled,
)


def test_current_returns_singleton():
    a = ReducedMotionState.current()
    b = ReducedMotionState.current()
    assert a is b


def test_default_disabled():
    state = ReducedMotionState.reset_for_tests()
    assert state.enabled is False
    assert reduced_motion_enabled() is False


def test_set_enabled_emits_signal(qtbot):
    state = ReducedMotionState.reset_for_tests()
    with qtbot.waitSignal(state.enabled_changed, timeout=500) as blocker:
        state.set_enabled(True)
    assert blocker.args == [True]
    assert state.enabled is True
    received = []
    state.enabled_changed.connect(lambda v: received.append(v))
    state.set_enabled(True)
    assert received == []


def test_toggle_returns_new_value():
    state = ReducedMotionState.reset_for_tests()
    assert state.toggle() is True
    assert state.enabled is True
    assert state.toggle() is False
    assert state.enabled is False


def test_apply_duration_disabled_passthrough():
    state = ReducedMotionState.reset_for_tests()
    assert state.apply_duration(240) == 240
    assert state.apply_duration(600) == 600


def test_apply_duration_enabled_caps_to_100():
    state = ReducedMotionState.reset_for_tests()
    state.set_enabled(True)
    assert state.apply_duration(240) == 100
    assert state.apply_duration(600) == 100
    assert state.apply_duration(50) == 50


def test_apply_easing_disabled_passthrough():
    state = ReducedMotionState.reset_for_tests()
    assert state.apply_easing(QEasingCurve.Type.OutBack) == QEasingCurve.Type.OutBack


def test_apply_easing_enabled_linear():
    state = ReducedMotionState.reset_for_tests()
    state.set_enabled(True)
    assert state.apply_easing(QEasingCurve.Type.OutBounce) == QEasingCurve.Type.Linear
    assert state.apply_easing(QEasingCurve.Type.OutElastic) == QEasingCurve.Type.Linear


def test_should_skip_path_motion():
    state = ReducedMotionState.reset_for_tests()
    assert state.should_skip_path_motion() is False
    state.set_enabled(True)
    assert state.should_skip_path_motion() is True


def test_should_skip_stagger():
    state = ReducedMotionState.reset_for_tests()
    assert state.should_skip_stagger() is False
    state.set_enabled(True)
    assert state.should_skip_stagger() is True


def test_scaled_stagger_step():
    state = ReducedMotionState.reset_for_tests()
    assert state.scaled_stagger_step(60) == 60
    state.set_enabled(True)
    assert state.scaled_stagger_step(60) == 0


def test_module_level_aliases():
    state = ReducedMotionState.reset_for_tests()
    state.set_enabled(True)
    assert reduced_motion_enabled() is True
    assert apply_reduced_duration(300) == 100
    assert apply_reduced_easing(QEasingCurve.Type.OutBack) == QEasingCurve.Type.Linear
    state.set_enabled(False)


def test_reset_for_tests_isolates_instances():
    first = ReducedMotionState.current()
    second = ReducedMotionState.reset_for_tests()
    assert first is not second
    assert ReducedMotionState.current() is second
    assert second.enabled is False
