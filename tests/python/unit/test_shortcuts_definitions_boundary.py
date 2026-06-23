"""shortcuts/definitions ShortcutCategory 枚举 + ShortcutDef 边界测试。

ShortcutCategory 枚举 + ShortcutDef frozen 此前经 test_shortcuts_definitions 间接测试。
本文件补枚举值 + frozen + 默认 shortcut help 条目。

覆盖：
1. ShortcutCategory 5 成员。
2. ShortcutCategory 值小写。
3. ShortcutDef frozen。
4. ShortcutDef 5 字段。
5. DEFAULT_SHORTCUTS 非空。
6. DEFAULT_SHORTCUTS id 唯一。
7. DEFAULT_SHORTCUTS 含 'send'。
8. DEFAULT_SHORTCUTS 含 'toggle_theme'。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.shortcuts.definitions import (
    DEFAULT_SHORTCUTS,
    ShortcutCategory,
    ShortcutDef,
)


def test_category_has_five_members():
    assert len(ShortcutCategory) == 5


def test_category_values_lowercase():
    for cat in ShortcutCategory:
        assert cat.value == cat.value.lower()


def test_shortcut_def_is_frozen():
    d = ShortcutDef(
        id="x", category=ShortcutCategory.FILE,
        default_key_sequence="Ctrl+X", description="test", callback_name="on_x",
    )
    with pytest.raises(AttributeError):
        d.id = "y"  # type: ignore[misc]


def test_shortcut_def_has_five_fields():
    d = ShortcutDef(
        id="send", category=ShortcutCategory.TRANSPORT,
        default_key_sequence="Ctrl+Return", description="Send command",
        callback_name="on_send",
    )
    assert d.id == "send"
    assert d.default_key_sequence == "Ctrl+Return"
    assert d.category == ShortcutCategory.TRANSPORT
    assert d.description == "Send command"
    assert d.callback_name == "on_send"


def test_default_shortcuts_non_empty():
    assert len(DEFAULT_SHORTCUTS) > 0


def test_default_shortcuts_ids_unique():
    ids = [s.id for s in DEFAULT_SHORTCUTS]
    assert len(ids) == len(set(ids))


def test_default_shortcuts_contains_send():
    ids = {s.id for s in DEFAULT_SHORTCUTS}
    assert "send" in ids


def test_default_shortcuts_contains_toggle_theme():
    ids = {s.id for s in DEFAULT_SHORTCUTS}
    assert "toggle_theme" in ids
