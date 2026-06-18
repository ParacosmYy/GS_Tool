"""Toast 通知子系统全局快捷键 + AppShell 装配集成测试。

覆盖 toast 的全局交互与装配接线：
1. AppShell 集成：NotificationManager 装配 / notify helper / 未知级别回退 /
   无 manager 健壮性 / app_notifications 装配委托。
2. 全局快捷键：app_notifications.handle_key_press（Esc 关闭最早 /
   Ctrl+Shift+Esc 清空 / 非快捷键放行 / Ctrl+Esc 不误触发）。
3. AppShell.keyPressEvent 端到端：真实按键关闭/清空 toast。
4. app_notifications 模块 helper 暴露 + settings_panel 文档化断言。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent

from embeddebug.app import app_notifications


def _make_key_event(key, modifiers=Qt.KeyboardModifier.NoModifier) -> QKeyEvent:
    """构造一个 QKeyEvent（模拟按键）。"""

    return QKeyEvent(QEvent.Type.KeyPress, key, modifiers)


def _make_shell_with_toasts(qtbot, count=3):
    """构造一个 AppShell 并塞入 count 条 toast（超长 timeout 不自动消失）。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    for i in range(count):
        shell.notify("info", f"t-{i}", f"m-{i}", timeout_ms=100000)
    return shell


# ── AppShell 装配 + notify helper ──────────────────────────────────
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


def test_appshell_sources_wire_notification_subsystem():
    """AppShell 应委托 app_notifications 装配，并保留 notify/_show_ready_toast 入口。

    Batch 15 重构后 NotificationManager/ToastContainer 实例化移到 app_notifications.py
    （守 300 行门禁），AppShell 通过 _app_notifications 委托。
    """

    from embeddebug.app import app_notifications, app_shell

    shell_src = inspect.getsource(app_shell)
    helper_src = inspect.getsource(app_notifications)
    # AppShell 委托 helper + 保留公开入口。
    assert "app_notifications" in shell_src
    assert "def notify" in shell_src
    assert "_show_ready_toast" in shell_src
    # helper 才是真正引用 NotificationManager/ToastContainer 的地方。
    assert "NotificationManager" in helper_src
    assert "ToastContainer" in helper_src


# ── app_notifications.handle_key_press ─────────────────────────────
def test_esc_dismiss_oldest_returns_true(qtbot):
    """有 toast 时 Esc → handle_key_press 返回 True 并 dismiss 一条。"""

    shell = _make_shell_with_toasts(qtbot, count=3)
    container = shell._toast_container
    before = container.count
    handled = app_notifications.handle_key_press(shell, _make_key_event(Qt.Key.Key_Escape))
    assert handled is True
    # dismiss_oldest 触发 leave（淡出），count 暂不变但 is_leaving 置位最早一条。
    assert container.active_toasts[0].is_leaving is True
    assert container.count == before  # 未离场完成前仍在 active


def test_esc_no_toasts_returns_false(qtbot):
    """无 toast 时 Esc → 返回 False（无可关闭项，交给父类）。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    handled = app_notifications.handle_key_press(shell, _make_key_event(Qt.Key.Key_Escape))
    assert handled is False


def test_ctrl_shift_esc_clears_all(qtbot):
    """Ctrl+Shift+Esc → clear_all（所有 toast 调 leave）。"""

    shell = _make_shell_with_toasts(qtbot, count=4)
    container = shell._toast_container
    handled = app_notifications.handle_key_press(
        shell,
        _make_key_event(
            Qt.Key.Key_Escape,
            Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier,
        ),
    )
    assert handled is True
    # 所有活跃 toast 应进入 leaving。
    assert all(t.is_leaving for t in container.active_toasts)


def test_non_escape_key_returns_false(qtbot):
    """非 Esc 键（如 Enter）→ 返回 False（交给父类默认处理）。"""

    shell = _make_shell_with_toasts(qtbot, count=2)
    handled = app_notifications.handle_key_press(
        shell, _make_key_event(Qt.Key.Key_Return)
    )
    assert handled is False


def test_esc_with_ctrl_only_not_treated_as_dismiss(qtbot):
    """Ctrl+Esc（非 Ctrl+Shift+Esc）不应触发 dismiss/clear（避免与系统快捷键冲突）。"""

    shell = _make_shell_with_toasts(qtbot, count=2)
    handled = app_notifications.handle_key_press(
        shell,
        _make_key_event(Qt.Key.Key_Escape, Qt.KeyboardModifier.ControlModifier),
    )
    # Ctrl+Esc 不匹配任一 toast 快捷键规则 → 返回 False。
    assert handled is False


# ── AppShell.keyPressEvent 端到端 ──────────────────────────────────
def test_appshell_esc_dismisses_oldest_toast(qtbot):
    """AppShell.keyPressEvent(Esc) 应关闭最早一条真实 toast（端到端）。"""

    shell = _make_shell_with_toasts(qtbot, count=2)
    container = shell._toast_container
    shell.keyPressEvent(_make_key_event(Qt.Key.Key_Escape))
    assert container.active_toasts[0].is_leaving is True


def test_appshell_ctrl_shift_esc_clears_all(qtbot):
    """AppShell.keyPressEvent(Ctrl+Shift+Esc) 应清空所有 toast。"""

    shell = _make_shell_with_toasts(qtbot, count=3)
    container = shell._toast_container
    shell.keyPressEvent(
        _make_key_event(
            Qt.Key.Key_Escape,
            Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier,
        )
    )
    assert all(t.is_leaving for t in container.active_toasts)


# ── app_notifications 模块存在性 ────────────────────────────────────
def test_app_notifications_module_exposes_helpers():
    """app_notifications 应暴露 build/show/handle_key_press/reposition。"""

    assert callable(app_notifications.build)
    assert callable(app_notifications.show)
    assert callable(app_notifications.handle_key_press)
    assert callable(app_notifications.reposition)


# ── settings_panel 文档化 ──────────────────────────────────────────
def test_settings_panel_documents_toast_shortcuts():
    """settings_panel 快捷键表应包含 Esc 和 Ctrl+Shift+Esc（toast 文档化）。"""

    from embeddebug.serial_station.ui.panels import settings_panel

    src = inspect.getsource(settings_panel)
    assert "Esc" in src
    assert "Ctrl+Shift+Esc" in src
