"""Ctrl+Shift+T 主题快捷键接线 + theme_actions 测试。

覆盖：
- shortcuts.handle_key_press 对 Ctrl+Shift+T 分发到 owner._toggle_theme。
- 单独 Ctrl+T（无 Shift）不触发（防浏览器 reopen-tab 误触）。
- 非 Ctrl 修饰不触发。
- MainWindow._toggle_theme 委托 theme_actions.toggle_theme（守架构：不内联逻辑）。
- theme_actions.toggle_theme 真正切换主题（dark→light）。
- definitions.toggle_theme 定义存在（callback_name 一致）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent

from embeddebug.serial_station.shortcuts.definitions import DEFAULT_SHORTCUTS
from embeddebug.serial_station.ui import shortcuts, theme_actions


class _FakeHost:
    """最小宿主，记录 _toggle_theme 调用以验证分发。"""

    def __init__(self) -> None:
        self.toggled = 0

    def _send_text(self) -> None: ...  # noqa: D401
    def _clear_log(self) -> None: ...  # noqa: D401
    def _refresh_serial_ports(self) -> None: ...  # noqa: D401
    def _open_command_palette(self) -> None: ...  # noqa: D401

    def _toggle_theme(self) -> None:
        self.toggled += 1


def _key(key: Qt.Key, modifiers: Qt.KeyboardModifier) -> QKeyEvent:
    return QKeyEvent(QEvent.Type.KeyPress, int(key), modifiers)


# ── 快捷键定义 ───────────────────────────────────────────────────
def test_toggle_theme_definition_exists():
    defs = {d.id: d for d in DEFAULT_SHORTCUTS}
    assert "toggle_theme" in defs
    assert defs["toggle_theme"].callback_name == "toggle_theme"
    assert defs["toggle_theme"].default_key_sequence == "Ctrl+Shift+T"


# ── handle_key_press 分发 ────────────────────────────────────────
def test_ctrl_shift_t_dispatches_to_toggle_theme():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier)
    assert shortcuts.handle_key_press(host, event) is True
    assert host.toggled == 1
    assert event.isAccepted()


def test_ctrl_t_without_shift_does_not_toggle():
    """单独 Ctrl+T（无 Shift）不触发主题切换。"""

    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.ControlModifier)
    # Ctrl+T 不是已知快捷键，handle_key_press 返回 False（未消费）。
    assert shortcuts.handle_key_press(host, event) is False
    assert host.toggled == 0


def test_shift_t_without_ctrl_does_not_toggle():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.ShiftModifier)
    assert shortcuts.handle_key_press(host, event) is False
    assert host.toggled == 0


def test_plain_t_does_not_toggle():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.NoModifier)
    assert shortcuts.handle_key_press(host, event) is False
    assert host.toggled == 0


def test_other_ctrl_keys_unaffected_by_toggle_branch():
    """Ctrl+L 仍走 clear，不被 toggle 分支误吞。"""

    host = _FakeHost()
    event = _key(Qt.Key.Key_L, Qt.KeyboardModifier.ControlModifier)
    # _FakeHost._clear_log 是空实现，只验证返回 True（被消费）且没触发 toggle。
    assert shortcuts.handle_key_press(host, event) is True
    assert host.toggled == 0


# ── MainWindow 委托（架构：不内联逻辑） ──────────────────────────
def test_main_window_toggle_theme_delegates_to_theme_actions():
    import inspect

    from embeddebug.serial_station.ui import main_window

    source = inspect.getsource(main_window.SerialStationMainWindow._toggle_theme)
    assert "theme_actions.toggle_theme(self)" in source


# ── theme_actions.toggle_theme 真正切换 ───────────────────────────
def test_theme_actions_toggle_switches_theme(qapp, monkeypatch, tmp_path):
    # 隔离 theme_store 落点，避免污染开发机。
    from embeddebug.serial_station.ui.theme import theme_store

    prefs = tmp_path / "embeddebug" / theme_store.PREFS_FILENAME
    legacy = tmp_path / "embeddebug" / "accent.json"
    monkeypatch.setattr(theme_store, "prefs_path", lambda: prefs)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy)

    from embeddebug.serial_station.ui.theme.manager import ThemeManager

    # 起始：深色。
    ThemeManager()._current_theme = "serial_station_dark"
    # 泵事件让过渡动画跑完（apply_fn 在谷值点执行 toggle）。
    from PyQt6.QtCore import QEventLoop, QTimer

    theme_actions.toggle_theme(qapp)
    loop = QEventLoop()
    QTimer.singleShot(600, loop.quit)
    loop.exec()
    # toggle 后应为浅色。
    assert ThemeManager().current_theme == "serial_station_light"
    assert theme_store.load_theme_id() == "serial_station_light"
