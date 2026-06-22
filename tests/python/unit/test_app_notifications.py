"""app_notifications 通知子系统 helper 边界测试。

模块此前无直接测试覆盖（grep 0 命中）。本文件覆盖：

1. _LEVEL_MAP 常量契约（4 级 → NotificationLevel）。
2. build 装配（NotificationManager + ToastContainer + 340 宽 + 初始 hide）。
3. show level 解析（已知 4 级 + 未知回退 INFO）+ 无 manager 安全跳过。
4. reposition 无 container 安全跳过 + 有 container 对齐右上角。
5. handle_key_press Esc 关闭最早 + Ctrl+Shift+Esc 清空 + 非 Esc 返回 False +
   无 container 返回 False + event 无 key 方法返回 False。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent
from PyQt6.QtWidgets import QMainWindow

from embeddebug.app.app_notifications import (
    _LEVEL_MAP,
    build,
    handle_key_press,
    reposition,
    show,
)
from embeddebug.serial_station.notifications.data import NotificationLevel


# ── _LEVEL_MAP 常量 ──────────────────────────────────────────────
def test_level_map_has_four_entries():
    assert len(_LEVEL_MAP) == 4


def test_level_map_maps_correctly():
    assert _LEVEL_MAP["info"] == NotificationLevel.INFO
    assert _LEVEL_MAP["success"] == NotificationLevel.SUCCESS
    assert _LEVEL_MAP["warning"] == NotificationLevel.WARNING
    assert _LEVEL_MAP["error"] == NotificationLevel.ERROR


# ── build 装配 ────────────────────────────────────────────────────
def test_build_creates_manager_and_container(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    assert shell._notification_manager is not None
    assert shell._toast_container is not None
    assert shell._toast_container.width() == 340
    assert shell._toast_container.isHidden()  # 初始 hide


# ── show level 解析 ───────────────────────────────────────────────
def test_show_with_manager_calls_manager_show(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    calls = []
    shell._notification_manager.show = lambda lvl, t, m, timeout_ms=3000: calls.append((lvl, t, m))
    show(shell, "error", "标题", "内容")
    assert len(calls) == 1
    assert calls[0][0] == NotificationLevel.ERROR
    assert calls[0][1] == "标题"


def test_show_unknown_level_falls_back_to_info(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    captured = []
    shell._notification_manager.show = lambda lvl, t, m, timeout_ms=3000: captured.append(lvl)
    show(shell, "critical", "x", "y")  # 未知 level
    assert captured == [NotificationLevel.INFO]


def test_show_without_manager_skips():
    """无 _notification_manager → 安全跳过（不抛）。"""

    shell = SimpleNamespace()
    show(shell, "info", "x", "y")  # 不抛


# ── reposition ────────────────────────────────────────────────────
def test_reposition_without_container_skips():
    """无 _toast_container → 安全跳过。"""

    shell = SimpleNamespace()
    reposition(shell)  # 不抛


def test_reposition_moves_container(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    shell.resize(800, 600)
    reposition(shell)
    # 容器应在右上角（x = 800 - 340 - 16 = 444，y = 16）。
    pos = shell._toast_container.pos()
    assert pos.x() == 800 - 340 - 16
    assert pos.y() == 16


# ── handle_key_press ─────────────────────────────────────────────
def test_handle_key_escape_dismiss_oldest(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    shell._toast_container.dismiss_oldest = MagicMock(return_value=True)
    event = QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Escape, Qt.KeyboardModifier.NoModifier)
    assert handle_key_press(shell, event) is True
    shell._toast_container.dismiss_oldest.assert_called_once()


def test_handle_key_ctrl_shift_escape_clears_all(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    shell._toast_container.clear_all = MagicMock()
    mods = Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier
    event = QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Escape, mods)
    assert handle_key_press(shell, event) is True
    shell._toast_container.clear_all.assert_called_once()


def test_handle_key_non_escape_returns_false(qtbot):
    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    event = QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Return, Qt.KeyboardModifier.NoModifier)
    assert handle_key_press(shell, event) is False


def test_handle_key_without_container_returns_false():
    """无 _toast_container → 返回 False。"""

    shell = SimpleNamespace()
    event = SimpleNamespace(key=lambda: Qt.Key.Key_Escape, modifiers=lambda: Qt.KeyboardModifier.NoModifier)
    assert handle_key_press(shell, event) is False


def test_handle_key_event_without_key_method_returns_false(qtbot):
    """event 无 key 方法 → 返回 False（防御）。"""

    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    event = SimpleNamespace()  # 无 key 属性
    assert handle_key_press(shell, event) is False


def test_handle_key_escape_dismiss_returns_false_when_no_toast(qtbot):
    """Esc 但 dismiss_oldest 返回 False（无 toast）→ handle 返回 False。"""

    shell = QMainWindow()
    qtbot.addWidget(shell)
    build(shell)
    shell._toast_container.dismiss_oldest = MagicMock(return_value=False)
    event = QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Escape, Qt.KeyboardModifier.NoModifier)
    assert handle_key_press(shell, event) is False
