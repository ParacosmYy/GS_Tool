"""StaggerCoordinator 单元测试。

覆盖：
1. 默认步长 60ms、自定义步长 100ms。
2. add() 后 factory 不立即调用。
3. start() 后所有 factory 被调用。
4. start() 后全部动画完成时发出 finished（QPauseAnimation 模拟动画）。
5. cancel() 在 factory 启动前应停止所有未启动 factory。
6. GC 防护：start 后在 _active，finished 后从 _active 移除。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from PyQt6.QtCore import QPauseAnimation

from embeddebug.serial_station.ui.animations.stagger import StaggerCoordinator


@pytest.fixture(autouse=True)
def _clean_active():
    """每个测试前后清空类级 _active，避免跨测试干扰。"""
    StaggerCoordinator._active.clear()
    yield
    StaggerCoordinator._active.clear()


# ── 构造 ──────────────────────────────────────────────────────────
def test_stagger_construct_default_step():
    """默认构造 step_ms 应为 60（Linear 风格默认）。"""
    coord = StaggerCoordinator()
    assert coord._step_ms == 60


def test_stagger_custom_step():
    """自定义 step_ms=100。"""
    coord = StaggerCoordinator(step_ms=100)
    assert coord._step_ms == 100


# ── add 不立即调用 ────────────────────────────────────────────────
def test_stagger_add_factory_does_not_start():
    """add 后 factory 不应被立即调用。"""
    calls: list[int] = []

    def factory() -> QPauseAnimation:
        calls.append(1)
        return QPauseAnimation(0)

    coord = StaggerCoordinator(step_ms=1000)
    coord.add(factory)
    assert calls == []
    assert len(coord._factories) == 1


# ── start 调用所有 factory ────────────────────────────────────────
def test_stagger_start_calls_factories(qtbot):
    """start 后所有 3 个 factory 应被调用。"""
    coord = StaggerCoordinator(step_ms=10)
    counter = [0]

    def factory() -> QPauseAnimation:
        counter[0] += 1
        return QPauseAnimation(50)

    for _ in range(3):
        coord.add(factory)
    coord.start()
    qtbot.waitUntil(lambda: counter[0] == 3, timeout=1000)
    assert counter[0] == 3


# ── 全部完成发出 finished ─────────────────────────────────────────
def test_stagger_start_emits_finished_when_all_done(qtbot):
    """3 个 50ms 动画 + step=10ms，最后一个应在 ~70ms 完成，触发 finished。"""
    coord = StaggerCoordinator(step_ms=10)
    for _ in range(3):
        coord.add(lambda: QPauseAnimation(50))
    with qtbot.waitSignal(coord.finished, timeout=2000):
        coord.start()
    # 全部动画应已启动并被持有（等待结束后引用仍在）。
    assert len(coord._anims) == 3


# ── cancel 停止 pending ───────────────────────────────────────────
def test_stagger_cancel_stops_pending(qtbot):
    """step=1000ms 长，cancel() 在 factory 启动前应停止所有未启动 factory。"""
    coord = StaggerCoordinator(step_ms=1000)
    counter = [0]

    def factory() -> QPauseAnimation:
        counter[0] += 1
        return QPauseAnimation(0)

    for _ in range(3):
        coord.add(factory)
    coord.start()
    coord.cancel()  # 同步取消：所有 QTimer 已 stop，未触发的不再 fire
    qtbot.wait(300)  # 等过事件循环若干轮，确认确实无 factory 被调用
    assert counter[0] == 0


# ── GC 防护 ───────────────────────────────────────────────────────
def test_stagger_registered_for_gc(qtbot):
    """start 后在 _active；finished 后从 _active 移除。

    注意：finished 信号触发后，_discard 在 emit() 之后的同一 slot 中执行。
    使用 waitUntil 轮询而非断言瞬时状态，避免 waitSignal 返回时序的边缘竞争。
    """
    coord = StaggerCoordinator(step_ms=10)
    coord.add(lambda: QPauseAnimation(50))
    assert coord not in StaggerCoordinator._active  # start 前
    coord.start()
    assert coord in StaggerCoordinator._active  # 启动后立即注册
    qtbot.waitUntil(
        lambda: coord not in StaggerCoordinator._active,
        timeout=2000,
    )
