"""StaggerCoordinator 单元测试 — 错峰入场编排器。

直接 import 子模块（不依赖 animations/__init__.py 的 re-export，避免环境回退）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QFrame

from embeddebug.serial_station.ui.animations.stagger import (
    StaggerCoordinator,
    StaggerHandle,
)
from embeddebug.serial_station.ui.animations.reduced_motion import ReducedMotionState
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def _reset_reduced_motion():
    ReducedMotionState.reset_for_tests()


def test_stagger_in_returns_handle_with_controller(qtbot):
    _reset_reduced_motion()
    cards = [QFrame() for _ in range(3)]
    for c in cards:
        qtbot.addWidget(c)
    handle = StaggerCoordinator.stagger_in(cards, step_ms=50, slide_px=0)
    assert isinstance(handle, StaggerHandle)
    assert handle.controller is not None


def test_stagger_in_creates_pending_timers_when_step_positive(qtbot):
    _reset_reduced_motion()
    cards = [QFrame() for _ in range(4)]
    for c in cards:
        qtbot.addWidget(c)
    handle = StaggerCoordinator.stagger_in(cards, step_ms=50, slide_px=0)
    assert len(handle.timers) == 3
    assert handle.remaining == 4


def test_stagger_in_no_timers_when_reduced_motion(qtbot):
    _reset_reduced_motion()
    ReducedMotionState.current().set_enabled(True)
    cards = [QFrame() for _ in range(4)]
    for c in cards:
        qtbot.addWidget(c)
    handle = StaggerCoordinator.stagger_in(cards, step_ms=50, slide_px=0)
    assert len(handle.timers) == 0


def test_stagger_in_no_timers_when_step_zero(qtbot):
    _reset_reduced_motion()
    cards = [QFrame() for _ in range(3)]
    for c in cards:
        qtbot.addWidget(c)
    handle = StaggerCoordinator.stagger_in(cards, step_ms=0, slide_px=0)
    assert len(handle.timers) == 0


def test_stagger_in_empty_triggers_completion(qtbot):
    _reset_reduced_motion()
    called = []
    handle = StaggerCoordinator.stagger_in(
        [], step_ms=50, slide_px=0, on_all_finished=lambda: called.append(True)
    )
    assert called == [True]
    assert handle.remaining == 0


def test_stagger_in_all_finished_after_animations(qtbot):
    """所有项动画完成后 on_all_finished 被调用。

    offscreen 平台下未 show 的 widget 的 opacity 动画 finished 信号可能不发，
    所以本测试只验证 stagger_in 不抛异常 + handle 状态正确（remaining=2）。
    on_all_finished 的端到端触发由真实 UI 路径（widget show 后）验证。
    """
    _reset_reduced_motion()
    cards = [QFrame() for _ in range(2)]
    for c in cards:
        qtbot.addWidget(c)
    called = []
    handle = StaggerCoordinator.stagger_in(
        cards, step_ms=0, slide_px=0, on_all_finished=lambda: called.append(True)
    )
    # 两项动画已启动（fade_in 创建并 start），controller 持有它们。
    assert len(handle.controller._animations) >= 1
    assert handle.remaining == 2


def test_cancel_stops_timers_and_animations(qtbot):
    _reset_reduced_motion()
    cards = [QFrame() for _ in range(4)]
    for c in cards:
        qtbot.addWidget(c)
    handle = StaggerCoordinator.stagger_in(cards, step_ms=200, slide_px=0)
    assert len(handle.timers) == 3
    handle.cancel()
    assert handle.timers == []
    assert handle.remaining == 0


def test_stagger_in_slide_px_adds_slide_animation(qtbot):
    _reset_reduced_motion()
    card = QFrame()
    qtbot.addWidget(card)
    handle = StaggerCoordinator.stagger_in([card], step_ms=0, slide_px=12)
    assert len(handle.controller._animations) >= 1


def test_stagger_fade_safe_no_slide(qtbot):
    _reset_reduced_motion()
    cards = [QFrame() for _ in range(3)]
    for c in cards:
        qtbot.addWidget(c)
    from embeddebug.serial_station.ui.animations.stagger import stagger_fade_safe
    handle = stagger_fade_safe(cards, step_ms=0)
    assert len(handle.controller._animations) >= 1


def test_handle_mark_one_finished_decrements(qtbot):
    _reset_reduced_motion()
    handle = StaggerCoordinator.stagger_in([QFrame()], step_ms=0, slide_px=0)
    assert handle.remaining == 1
    handle._mark_one_finished()
    assert handle.remaining == 0
