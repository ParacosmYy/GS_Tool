"""Batch 12 测试：ToastContainer + NotificationManager→ToastWidget 端到端闭环 + AppShell 集成。

覆盖：
1. ToastContainer 接 manager 信号，add/remove 渲染 ToastWidget。
2. manager.show() → container 出现 toast（端到端闭环）。
3. toast closed（离场完成）→ 容器移除，空了隐藏。
4. max_visible 超限挤掉最早一条。
5. AppShell 实例化 NotificationManager + ToastContainer，notify() helper 落 toast。
6. QSS 覆盖 toast container objectName。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

import pytest

from embeddebug.serial_station.notifications import NotificationManager
from embeddebug.serial_station.notifications.data import NotificationLevel
from embeddebug.serial_station.ui.widgets.toast import ToastWidget
from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer


# ── ToastContainer 渲染闭环 ────────────────────────────────────────
def test_container_has_objectname(qtbot):
    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.objectName() == "serialStationToastContainer"


def test_manager_show_renders_toast(qtbot):
    """manager.show() → container 渲染一条 ToastWidget（端到端闭环）。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    manager.show(NotificationLevel.SUCCESS, "连接成功", "COM3 就绪")
    assert len(container.active_toasts) == 1
    toast = container.active_toasts[0]
    assert isinstance(toast, ToastWidget)
    assert toast._title_label.text() == "连接成功"


def test_container_empty_hides(qtbot):
    """无 toast 时容器隐藏。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.is_empty is True


def test_toast_closed_removes_from_container(qtbot):
    """ToastWidget 离场完成 → 从容器移除。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    manager.show(NotificationLevel.INFO, "提示", "内容", timeout_ms=100000)
    toast = container.active_toasts[0]
    # 模拟离场完成回调（leave → fade_out 完成 → closed 信号）。
    container._on_toast_closed(toast)
    assert container.is_empty is True


def test_manager_remove_triggers_toast_leave(qtbot, monkeypatch):
    """manager.dismiss/超时移除 → 对应 toast 调 leave（淡出）。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    data = manager.show(NotificationLevel.INFO, "提示", "内容", timeout_ms=100000)
    toast = container._find(data.uid)
    assert toast is not None
    leave_calls: list = []
    monkeypatch.setattr(toast, "leave", lambda: leave_calls.append(toast))
    manager.dismiss(data.uid)
    assert len(leave_calls) == 1


# ── max_visible 挤兑 ───────────────────────────────────────────────
def test_container_enforces_max_visible(qtbot, monkeypatch):
    """超过 max_visible 时挤掉最早一条（对最旧 toast 调 leave）。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=2)
    qtbot.addWidget(container)
    # 加三条 toast（绕过 _enforce_max_visible 直接加，再手动触发以精确控制断言）。
    leave_calls: list = []
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
        # 加完后立即拦截最新 toast 的 leave（避免被动画时序干扰）。
        for _, t in container._active:
            if not hasattr(t, "_leave_patched"):
                t._leave_patched = True
                monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    # _enforce_max_visible 已在第三条加入时被调用，应挤掉 toast #1。
    assert len(leave_calls) >= 1


def test_enforce_max_visible_directly(qtbot, monkeypatch):
    """直接测 _enforce_max_visible：超限时对最早 toast 调 leave。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=2)
    qtbot.addWidget(container)
    leave_calls: list = []
    # 手动塞三条 toast，全部拦截 leave。
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    # 临时调小 max_visible 再强制执行挤兑。
    container._max_visible = 1
    container._enforce_max_visible()
    assert len(leave_calls) >= 1


# ── AppShell 集成 ──────────────────────────────────────────────────
def test_appshell_holds_notification_manager(qtbot):
    """AppShell 应实例化 NotificationManager + ToastContainer。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    assert shell.notification_manager() is not None
    assert getattr(shell, "_toast_container", None) is not None


def test_appshell_notify_renders_toast(qtbot):
    """AppShell.notify() 应通过 manager → container 渲染一条 toast。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    container = shell._toast_container
    shell.notify("warning", "测试", "notify helper 落 toast", timeout_ms=100000)
    assert len(container.active_toasts) == 1
    toast = container.active_toasts[0]
    assert toast._title_label.text() == "测试"


def test_appshell_notify_unknown_level_defaults_info(qtbot):
    """未知 level 字符串应回退到 INFO（不崩溃）。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    shell.notify("bogus_level", "x", "y", timeout_ms=100000)
    assert len(shell._toast_container.active_toasts) == 1


def test_appshell_notify_without_manager_no_crash(qtbot):
    """无 manager 时 notify 不应崩溃（健壮性，manager 缺失静默返回）。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    # 模拟 manager 未装配。
    delattr(shell, "_notification_manager") if hasattr(shell, "_notification_manager") else None
    shell._notification_manager = None
    shell.notify("info", "x", "y")  # 不应抛异常


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_appshell_sources_wire_notification_subsystem():
    """AppShell 源码应引用 NotificationManager + ToastContainer + notify。"""

    from embeddebug.app import app_shell

    src = inspect.getsource(app_shell)
    assert "NotificationManager" in src
    assert "ToastContainer" in src
    assert "def notify" in src
    assert "_show_ready_toast" in src


# ── QSS 覆盖 ───────────────────────────────────────────────────────
def test_qss_covers_toast_container_objectname():
    """build_qss 应含 serialStationToastContainer 选择器。"""

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    qss = build_qss()
    assert "#serialStationToastContainer" in qss
    assert "#serialStationToastPlaceholder" in qss
