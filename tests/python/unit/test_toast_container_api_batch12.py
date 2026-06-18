"""Batch 12 测试（分体 2）：ToastContainer 管理 API。

从 test_toast_container_batch12.py 拆出，满足 250 行测试文件预算。
覆盖 count/is_empty/clear_all/dismiss_oldest + manager.clear 端到端传播。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.notifications import NotificationManager
from embeddebug.serial_station.notifications.data import NotificationLevel
from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer


def test_container_count_and_is_empty(qtbot):
    """count / is_empty 反映活跃 toast 数。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.count == 0
    assert container.is_empty is True
    manager.show(NotificationLevel.INFO, "1", "", timeout_ms=100000)
    manager.show(NotificationLevel.INFO, "2", "", timeout_ms=100000)
    assert container.count == 2
    assert container.is_empty is False


def test_clear_all_leaves_every_toast(qtbot, monkeypatch):
    """clear_all 应对每条活跃 toast 调 leave（淡出）。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=10)
    qtbot.addWidget(container)
    leave_calls: list = []
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    container.clear_all()
    assert len(leave_calls) == 3


def test_dismiss_oldest_returns_false_when_empty(qtbot):
    """无活跃 toast 时 dismiss_oldest 返回 False。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.dismiss_oldest() is False


def test_dismiss_oldest_leaves_first(qtbot, monkeypatch):
    """dismiss_oldest 应对最早一条调 leave 并返回 True。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=10)
    qtbot.addWidget(container)
    leave_calls: list = []
    for i in range(2):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    assert container.dismiss_oldest() is True
    assert len(leave_calls) == 1  # 只 dismiss 一条


def test_manager_clear_propagates_to_container(qtbot, monkeypatch):
    """manager.clear() → 每条通知 removed 信号 → 对应 toast leave。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=10)
    qtbot.addWidget(container)
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    leave_calls: list = []
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    manager.clear()
    assert len(leave_calls) == 3
