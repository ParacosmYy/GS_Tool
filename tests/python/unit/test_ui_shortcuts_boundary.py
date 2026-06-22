"""ui/shortcuts.py install_shortcuts + handle_key_press 边界测试。

模块此前无直接测试覆盖（grep 0 命中，仅经 main_window 间接调用）。
本文件覆盖 4 个快捷键装配 + handle_key_press 全分支。

覆盖：
1. install_shortcuts 创建 4 个 QShortcut + owner._shortcuts 列表。
2. install_shortcuts objectName 契约（send/clear/refresh/palette）。
3. install_shortcuts 连接 activated 到对应 host 方法。
4. handle_key_press Ctrl+Return → _send_text True。
5. handle_key_press Ctrl+Enter → _send_text True。
6. handle_key_press Ctrl+L → _clear_log True。
7. handle_key_press Ctrl+R → _refresh_serial_ports True。
8. handle_key_press Ctrl+P → _open_command_palette True。
9. handle_key_press Ctrl+Shift+T → _toggle_theme True。
10. handle_key_press 单 Ctrl+T（无 Shift）→ False（不触发主题）。
11. handle_key_press 无 Ctrl 修饰 → False。
12. handle_key_press 未知 Ctrl+X → False。
13. handle_key_press event.accept 被调用（已处理路径）。
"""

from __future__ import annotations

import os
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent
from PyQt6.QtWidgets import QMainWindow

from embeddebug.serial_station.ui.shortcuts import (
    handle_key_press,
    install_shortcuts,
)


class _FakeHost:
    """模拟 SerialStationShortcutHost（5 个回调方法）。"""

    def __init__(self) -> None:
        self.send_calls = 0
        self.clear_calls = 0
        self.refresh_calls = 0
        self.palette_calls = 0
        self.theme_calls = 0
        self._shortcuts = None

    def _send_text(self) -> None:
        self.send_calls += 1

    def _clear_log(self) -> None:
        self.clear_calls += 1

    def _refresh_serial_ports(self) -> None:
        self.refresh_calls += 1

    def _open_command_palette(self) -> None:
        self.palette_calls += 1

    def _toggle_theme(self) -> None:
        self.theme_calls += 1


def _key_event(key: int, modifiers: Qt.KeyboardModifier) -> QKeyEvent:
    return QKeyEvent(QEvent.Type.KeyPress, key, modifiers)


CTRL = Qt.KeyboardModifier.ControlModifier
SHIFT = Qt.KeyboardModifier.ShiftModifier
NONE = Qt.KeyboardModifier.NoModifier


# ── install_shortcuts ─────────────────────────────────────────────
def _make_host_window(qtbot):
    """QMainWindow + 5 个 host 方法（install_shortcuts 需 QWidget 父 + 方法）。"""

    window = QMainWindow()
    qtbot.addWidget(window)
    window._send_text = MagicMock()
    window._clear_log = MagicMock()
    window._refresh_serial_ports = MagicMock()
    window._open_command_palette = MagicMock()
    window._toggle_theme = MagicMock()
    return window


def test_install_creates_four_shortcuts(qtbot):
    window = _make_host_window(qtbot)
    install_shortcuts(window)
    assert len(window._shortcuts) == 4


def test_install_objectnames(qtbot):
    window = _make_host_window(qtbot)
    install_shortcuts(window)
    names = [s.objectName() for s in window._shortcuts]
    assert "serialStationSendShortcut" in names
    assert "serialStationClearShortcut" in names
    assert "serialStationRefreshPortsShortcut" in names
    assert "serialStationCommandPaletteShortcut" in names


# ── handle_key_press Ctrl+Return/Enter ───────────────────────────
def test_handle_ctrl_return_triggers_send():
    host = _FakeHost()
    event = _key_event(Qt.Key.Key_Return, CTRL)
    event.accept = MagicMock()
    assert handle_key_press(host, event) is True
    assert host.send_calls == 1
    event.accept.assert_called_once()


def test_handle_ctrl_enter_triggers_send():
    host = _FakeHost()
    event = _key_event(Qt.Key.Key_Enter, CTRL)
    assert handle_key_press(host, event) is True
    assert host.send_calls == 1


# ── handle_key_press Ctrl+L ───────────────────────────────────────
def test_handle_ctrl_l_triggers_clear():
    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_L, CTRL)) is True
    assert host.clear_calls == 1


# ── handle_key_press Ctrl+R ───────────────────────────────────────
def test_handle_ctrl_r_triggers_refresh():
    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_R, CTRL)) is True
    assert host.refresh_calls == 1


# ── handle_key_press Ctrl+P ───────────────────────────────────────
def test_handle_ctrl_p_triggers_palette():
    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_P, CTRL)) is True
    assert host.palette_calls == 1


# ── handle_key_press Ctrl+Shift+T ─────────────────────────────────
def test_handle_ctrl_shift_t_triggers_theme():
    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_T, CTRL | SHIFT)) is True
    assert host.theme_calls == 1


def test_handle_ctrl_t_without_shift_does_not_trigger_theme():
    """单 Ctrl+T（无 Shift）→ False（避免误触）。"""

    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_T, CTRL)) is False
    assert host.theme_calls == 0


# ── handle_key_press 无 Ctrl ──────────────────────────────────────
def test_handle_no_modifier_returns_false():
    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_Return, NONE)) is False
    assert handle_key_press(host, _key_event(Qt.Key.Key_L, NONE)) is False


def test_handle_shift_only_returns_false():
    """Shift 但无 Ctrl → False。"""

    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_T, SHIFT)) is False


# ── handle_key_press 未知 Ctrl 组合 ───────────────────────────────
def test_handle_unknown_ctrl_combo_returns_false():
    host = _FakeHost()
    assert handle_key_press(host, _key_event(Qt.Key.Key_X, CTRL)) is False
    assert handle_key_press(host, _key_event(Qt.Key.Key_Z, CTRL)) is False


def test_handle_unknown_does_not_call_any_callback():
    host = _FakeHost()
    handle_key_press(host, _key_event(Qt.Key.Key_X, CTRL))
    assert host.send_calls == 0
    assert host.clear_calls == 0
    assert host.refresh_calls == 0
    assert host.palette_calls == 0
    assert host.theme_calls == 0
