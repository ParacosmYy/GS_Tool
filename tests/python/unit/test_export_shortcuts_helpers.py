"""export _channel_indices + shortcuts _effective_key/manager 边界单元测试。

补强 test_export.py / test_shortcuts_manager.py 未直接断言的边角：
- DataExporter._channel_indices：None=全选 / 已知名顺序 / 未知名 KeyError / 空 names。
- _effective_key：默认值 / override 覆盖 / 未知 id。
- ShortcutManager：list_shortcuts / key_sequence_for 默认/override/unknown /
  to_dict 版本 + from_dict 非 dict / None 值过滤 / grouped_by_category 分组数。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.export.exporter import DataExporter
from embeddebug.serial_station.shortcuts.definitions import ShortcutCategory
from embeddebug.serial_station.shortcuts.manager import (
    ShortcutManager,
    _effective_key,
)


# ── DataExporter._channel_indices 静态方法 ──────────────────────────────


def test_channel_indices_none_returns_all():
    """channels=None → 全部索引 [0, 1, 2, ...]。"""

    assert DataExporter._channel_indices(["a", "b", "c"], None) == [0, 1, 2]


def test_channel_indices_named_order():
    """显式通道名 → 按名称顺序返回索引。"""

    names = ["temp", "volt", "curr"]
    assert DataExporter._channel_indices(names, ["volt", "temp"]) == [1, 0]


def test_channel_indices_empty_names_none():
    """空 names + None → 空列表。"""

    assert DataExporter._channel_indices([], None) == []


def test_channel_indices_unknown_raises():
    """未知名 → KeyError。"""

    with pytest.raises(KeyError, match="未找到"):
        DataExporter._channel_indices(["a"], ["ghost"])


def test_channel_indices_duplicate_names():
    """重复名取最后一个出现的索引（dict 覆盖语义）。"""

    names = ["ch", "ch"]
    assert DataExporter._channel_indices(names, ["ch"]) == [1]


def test_channel_indices_subset():
    """子集选择。"""

    names = ["a", "b", "c", "d"]
    assert DataExporter._channel_indices(names, ["b", "d"]) == [1, 3]


# ── _effective_key 纯函数 ────────────────────────────────────────────────


def _make_def(shortcut_id: str = "send", key: str = "Ctrl+Return"):
    from embeddebug.serial_station.shortcuts.definitions import ShortcutDef

    return ShortcutDef(
        id=shortcut_id,
        category=ShortcutCategory.TRANSPORT,
        default_key_sequence=key,
        description="test",
        callback_name="send",
    )


def test_effective_key_no_override_returns_default():
    """无 override → 返回 default_key_sequence。"""

    def_ = _make_def(key="Ctrl+Return")
    assert _effective_key(def_, {}) == "Ctrl+Return"


def test_effective_key_override_returns_new():
    """有 override → 返回 override 值。"""

    def_ = _make_def(key="Ctrl+Return")
    assert _effective_key(def_, {"send": "Ctrl+Enter"}) == "Ctrl+Enter"


def test_effective_key_override_different_id_ignored():
    """override 含其他 id → 不影响本 def。"""

    def_ = _make_def(key="Ctrl+Return")
    assert _effective_key(def_, {"other": "Ctrl+X"}) == "Ctrl+Return"


# ── ShortcutManager 边角 ────────────────────────────────────────────────


def test_list_shortcuts_returns_all_defaults():
    """list_shortcuts 返回 DEFAULT_SHORTCUTS 全部。"""

    mgr = ShortcutManager()
    shortcuts = mgr.list_shortcuts()
    assert len(shortcuts) >= 10


def test_key_sequence_for_default():
    """key_sequence_for 默认值。"""

    mgr = ShortcutManager()
    assert mgr.key_sequence_for("send") == "Ctrl+Return"


def test_key_sequence_for_override():
    """key_sequence_for override 值。"""

    mgr = ShortcutManager(overrides={"send": "Ctrl+Enter"})
    assert mgr.key_sequence_for("send") == "Ctrl+Enter"


def test_to_dict_contains_version():
    """to_dict 含 version=1。"""

    mgr = ShortcutManager()
    d = mgr.to_dict()
    assert d["version"] == 1
    assert "overrides" in d


def test_from_dict_non_dict_returns_empty_overrides():
    """from_dict 非 dict（如 list）→ 空 overrides。"""

    mgr = ShortcutManager.from_dict(["not", "a", "dict"])  # type: ignore[arg-type]
    assert mgr.overrides == {}


def test_from_dict_filters_none_values():
    """from_dict 过滤 None 值。"""

    mgr = ShortcutManager.from_dict({
        "overrides": {"send": None, "clear": "Ctrl+Shift+L"},
    })
    assert "send" not in mgr.overrides
    assert mgr.overrides["clear"] == "Ctrl+Shift+L"


def test_from_dict_filters_unknown_ids():
    """from_dict 过滤未知 id。"""

    mgr = ShortcutManager.from_dict({
        "overrides": {"send": "Ctrl+X", "ghost": "Ctrl+Z"},
    })
    assert "send" in mgr.overrides
    assert "ghost" not in mgr.overrides


def test_grouped_by_category_has_multiple_groups():
    """grouped_by_category 返回多组（≥3 分类）。"""

    mgr = ShortcutManager()
    groups = mgr.grouped_by_category()
    assert len(groups) >= 3


def test_grouped_by_category_all_items_are_shortcut_def():
    """grouped_by_category 每项是 ShortcutDef。"""

    from embeddebug.serial_station.shortcuts.definitions import ShortcutDef

    mgr = ShortcutManager()
    groups = mgr.grouped_by_category()
    for cat, defs in groups.items():
        assert isinstance(cat, ShortcutCategory)
        for d in defs:
            assert isinstance(d, ShortcutDef)


def test_grouped_by_category_keys_are_valid_categories():
    """分组键都是有效 ShortcutCategory。"""

    mgr = ShortcutManager()
    groups = mgr.grouped_by_category()
    for key in groups:
        assert isinstance(key, ShortcutCategory)


def test_reset_to_defaults_clears_overrides():
    """reset_to_defaults 清空 overrides（无 host 不重建）。"""

    mgr = ShortcutManager(overrides={"send": "Ctrl+Enter"})
    mgr.reset_to_defaults()
    assert mgr.overrides == {}
    assert mgr.key_sequence_for("send") == "Ctrl+Return"


def test_find_def_known_returns_def():
    """_find_def 已知 id → 返回 ShortcutDef。"""

    mgr = ShortcutManager()
    def_ = mgr._find_def("send")
    assert def_ is not None
    assert def_.id == "send"


def test_find_def_unknown_returns_none():
    """_find_def 未知 id → None。"""

    mgr = ShortcutManager()
    assert mgr._find_def("ghost") is None
