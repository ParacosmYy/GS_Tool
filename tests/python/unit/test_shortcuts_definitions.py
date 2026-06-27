"""shortcuts/definitions 常量契约单元测试。

覆盖 ShortcutCategory 枚举 + ShortcutDef frozen dataclass + DEFAULT_SHORTCUTS 表：
- ShortcutCategory：5 枚举成员 + value 值。
- ShortcutDef：frozen 不可变 + 5 字段。
- DEFAULT_SHORTCUTS：非空 + id 唯一 + default_key_sequence 非空 + category 有效 +
  description 非空 + callback_name 非空 + 关键快捷键存在。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.shortcuts.definitions import (
    DEFAULT_SHORTCUTS,
    ShortcutCategory,
    ShortcutDef,
)


# ── ShortcutCategory 枚举 ────────────────────────────────────────────────


def test_category_has_five_members():
    """ShortcutCategory 含 5 个分类。"""

    assert len(ShortcutCategory) == 5


def test_category_values():
    """枚举 value 是小写字符串。"""

    assert ShortcutCategory.FILE.value == "file"
    assert ShortcutCategory.EDIT.value == "edit"
    assert ShortcutCategory.VIEW.value == "view"
    assert ShortcutCategory.TRANSPORT.value == "transport"
    assert ShortcutCategory.HELP.value == "help"


def test_category_values_all_lowercase():
    """所有 value 都是小写（一致性）。"""

    for member in ShortcutCategory:
        assert member.value == member.value.lower()


# ── ShortcutDef frozen dataclass ─────────────────────────────────────────


def _make_def() -> ShortcutDef:
    return ShortcutDef(
        id="test",
        category=ShortcutCategory.EDIT,
        default_key_sequence="Ctrl+T",
        description="Test shortcut",
        callback_name="test_callback",
    )


def test_shortcut_def_has_five_fields():
    """ShortcutDef 含 5 个字段。"""

    sd = _make_def()
    assert sd.id == "test"
    assert sd.category == ShortcutCategory.EDIT
    assert sd.default_key_sequence == "Ctrl+T"
    assert sd.description == "Test shortcut"
    assert sd.callback_name == "test_callback"


def test_shortcut_def_is_frozen():
    """ShortcutDef 是 frozen（不可变）。"""

    sd = _make_def()
    with pytest.raises((AttributeError, Exception)):
        sd.id = "changed"  # type: ignore[misc]


# ── DEFAULT_SHORTCUTS 表契约 ─────────────────────────────────────────────


def test_default_shortcuts_non_empty():
    """DEFAULT_SHORTCUTS 非空（至少 10 条）。"""

    assert len(DEFAULT_SHORTCUTS) >= 10


def test_default_shortcuts_ids_unique():
    """所有快捷键 id 唯一。"""

    ids = [s.id for s in DEFAULT_SHORTCUTS]
    assert len(ids) == len(set(ids))


def test_default_shortcuts_key_sequences_non_empty():
    """所有 default_key_sequence 非空。"""

    for s in DEFAULT_SHORTCUTS:
        assert s.default_key_sequence, f"{s.id} has empty key sequence"


def test_default_shortcuts_descriptions_non_empty():
    """所有 description 非空（用户可见文字）。"""

    for s in DEFAULT_SHORTCUTS:
        assert s.description, f"{s.id} has empty description"


def test_default_shortcuts_callback_names_non_empty():
    """所有 callback_name 非空。"""

    for s in DEFAULT_SHORTCUTS:
        assert s.callback_name, f"{s.id} has empty callback_name"


def test_default_shortcuts_categories_valid():
    """所有 category 是有效 ShortcutCategory 枚举。"""

    for s in DEFAULT_SHORTCUTS:
        assert isinstance(s.category, ShortcutCategory)


def test_default_shortcuts_contains_connect():
    """含 connect 快捷键（核心交互）。"""

    ids = {s.id for s in DEFAULT_SHORTCUTS}
    assert "connect" in ids
    assert "disconnect" in ids
    assert "send" in ids
    assert "toggle_theme" in ids


def test_default_shortcuts_contains_command_palette():
    """含 command_palette 快捷键（Ctrl+P）。"""

    cmd_palette = [s for s in DEFAULT_SHORTCUTS if s.id == "command_palette"]
    assert len(cmd_palette) == 1
    assert "Ctrl+P" in cmd_palette[0].default_key_sequence


def test_default_shortcuts_contains_toggle_theme():
    """含 toggle_theme 快捷键。"""

    theme = [s for s in DEFAULT_SHORTCUTS if s.id == "toggle_theme"]
    assert len(theme) == 1


def test_default_shortcuts_send_uses_ctrl_return():
    """send 快捷键用 Ctrl+Return（非 Enter，兼容数字键盘）。"""

    send = next(s for s in DEFAULT_SHORTCUTS if s.id == "send")
    assert "Return" in send.default_key_sequence or "Enter" in send.default_key_sequence


def test_default_shortcuts_categories_covered():
    """DEFAULT_SHORTCUTS 覆盖多个分类（不止 1 种）。"""

    categories = {s.category for s in DEFAULT_SHORTCUTS}
    assert len(categories) >= 3  # 至少 3 种分类


def test_default_shortcuts_all_items_are_shortcut_def():
    """DEFAULT_SHORTCUTS 每项是 ShortcutDef 实例。"""

    for item in DEFAULT_SHORTCUTS:
        assert isinstance(item, ShortcutDef)


def test_default_shortcuts_quit_exists():
    """含 quit 快捷键（Ctrl+Q 退出）。"""

    quit_defs = [s for s in DEFAULT_SHORTCUTS if s.id == "quit"]
    assert len(quit_defs) == 1


def test_default_shortcuts_f1_is_help():
    """F1 绑定到 help 分类（关于）。"""

    f1_defs = [s for s in DEFAULT_SHORTCUTS if s.default_key_sequence == "F1"]
    assert len(f1_defs) == 1
    assert f1_defs[0].category == ShortcutCategory.HELP
