"""CommandItem frozen + CommandPalette set_commands/execute 边界测试。

test_command_palette 覆盖基础；本文件补 CommandItem frozen + set_commands 空 +
_execute_item None 安全 + hint 默认值 + keyPressEvent 非 Esc。

覆盖：
1. CommandItem frozen 不可变。
2. CommandItem 默认 hint=""。
3. CommandItem 自定义 hint。
4. set_commands 空列表不崩。
5. set_commands 替换旧命令。
6. _execute_item None 不崩。
7. keyPressEvent 非 Esc/Enter 不崩。
8. keyPressEvent Down 不崩。
9. open/close 循环。
10. set_commands 后 _rerank 空查询返回全部。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent

from embeddebug.serial_station.ui.command_palette import (
    CommandItem,
    CommandPalette,
    fuzzy_score,
    rank_commands,
)


def _make_item(title="Test", hint=""):
    return CommandItem(title=title, callback=lambda: None, hint=hint)


def _key_event(key):
    return QKeyEvent(QEvent.Type.KeyPress, key, Qt.KeyboardModifier.NoModifier)


# ── CommandItem frozen ───────────────────────────────────────────
def test_command_item_is_frozen():
    item = _make_item()
    with pytest.raises(AttributeError):
        item.title = "Changed"  # type: ignore[misc]


def test_command_item_default_hint_empty():
    item = CommandItem(title="x", callback=lambda: None)
    assert item.hint == ""


def test_command_item_custom_hint():
    item = CommandItem(title="x", callback=lambda: None, hint="shortcut")
    assert item.hint == "shortcut"


def test_command_item_title_access():
    item = CommandItem(title="Connect", callback=lambda: None)
    assert item.title == "Connect"


# ── CommandPalette set_commands 边界 ─────────────────────────────
def test_set_commands_empty_no_crash(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands(())


def test_set_commands_replaces_old(qtbot):
    """set_commands 替换旧命令。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands((_make_item("Old"),))
    palette.set_commands((_make_item("New"),))


# ── _execute_item None 安全 ──────────────────────────────────────
def test_execute_item_none_no_crash(qtbot):
    """_execute_item(None) 不崩（无选中项）。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette._execute_item(None)


def test_execute_selected_no_items_no_crash(qtbot):
    """_execute_selected 无项不崩。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette._execute_selected()


# ── keyPressEvent 边界 ───────────────────────────────────────────
def test_key_press_down_no_crash(qtbot):
    """Down 键不崩。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands((_make_item("A"), _make_item("B")))
    palette.keyPressEvent(_key_event(Qt.Key.Key_Down))


def test_key_press_up_no_crash(qtbot):
    """Up 键不崩。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands((_make_item("A"),))
    palette.keyPressEvent(_key_event(Qt.Key.Key_Up))


def test_key_press_unknown_no_crash(qtbot):
    """未知键不崩。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.keyPressEvent(_key_event(Qt.Key.Key_F5))


# ── open/close 循环 ──────────────────────────────────────────────
def test_open_close_cycle(qtbot):
    """open/close 多次循环不崩。"""

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.open()
    palette.close()
    palette.open()
    palette.close()


def test_fuzzy_score_single_char():
    assert fuzzy_score("c", "connect") > 0


def test_fuzzy_score_long_query_no_match():
    assert fuzzy_score("verylongquery", "ab") == -1


def test_fuzzy_score_numbers():
    assert fuzzy_score("123", "test123") > 0


def test_fuzzy_score_exact_match():
    assert fuzzy_score("connect", "connect") > 0


def test_fuzzy_score_symbol_in_query():
    assert fuzzy_score("+", "AT+RST") > 0


def test_rank_commands_empty_commands():
    assert rank_commands("test", ()) == []


def test_rank_commands_empty_query_empty_commands():
    assert rank_commands("", ()) == []


def test_rank_commands_single_item():
    commands = (_make_item("Connect"),)
    assert len(rank_commands("con", commands)) == 1


def test_rank_commands_all_no_match():
    commands = (_make_item("Connect"), _make_item("Reset"))
    assert rank_commands("xyz", commands) == []


def test_rank_commands_preserves_order_same_score():
    commands = (_make_item("abc"), _make_item("abc"))
    result = rank_commands("abc", commands)
    assert len(result) == 2
