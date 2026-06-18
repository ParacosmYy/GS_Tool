"""Batch 15 测试：toast 全局快捷键（Esc 关闭最早 / Ctrl+Shift+Esc 清空）。

覆盖：
1. app_notifications.handle_key_press：Esc → dismiss_oldest 返回 True。
2. Ctrl+Shift+Esc → clear_all。
3. 非快捷键（如 Enter）→ 返回 False（交给父类）。
4. 无 toast 时 Esc → 返回 False（无可关闭项）。
5. AppShell.keyPressEvent 端到端：Esc 关闭最早一条真实 toast。
6. settings_panel 文档化 toast 快捷键（Esc / Ctrl+Shift+Esc）。
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


# ── settings_panel 文档化 ──────────────────────────────────────────
def test_settings_panel_documents_toast_shortcuts():
    """settings_panel 快捷键表应包含 Esc 和 Ctrl+Shift+Esc（toast 文档化）。"""

    from embeddebug.serial_station.ui.panels import settings_panel

    src = inspect.getsource(settings_panel)
    assert "Esc" in src
    assert "Ctrl+Shift+Esc" in src


# ── app_notifications 模块存在性 ────────────────────────────────────
def test_app_notifications_module_exposes_helpers():
    """app_notifications 应暴露 build/show/handle_key_press/reposition。"""

    assert callable(app_notifications.build)
    assert callable(app_notifications.show)
    assert callable(app_notifications.handle_key_press)
    assert callable(app_notifications.reposition)
