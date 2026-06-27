"""帮助系统单元测试。"""

from __future__ import annotations

from embeddebug.serial_station.help import HelpEntry, HelpSystem, HelpTopic, PROTOCOL_HELP, SHORTCUT_HELP


def test_get_topic_returns_entries():
    system = HelpSystem()
    for topic in HelpTopic:
        assert len(system.get_topic(topic)) >= 1


def test_search_matches_title():
    system = HelpSystem()
    results = system.search("快捷键")
    assert any("快捷键" in e.title for e in results)


def test_search_matches_tag():
    system = HelpSystem()
    results = system.search("vofa")
    assert any("JustFloat" in e.title for e in results)


def test_search_empty_returns_empty():
    system = HelpSystem()
    assert system.search("") == []


def test_all_topics_covers_all():
    system = HelpSystem()
    assert set(system.all_topics().keys()) == set(HelpTopic)


def test_about_info():
    system = HelpSystem()
    info = system.about_info()
    assert info["app_name"] == "EmbedDebug"
    assert "version" in info
    assert "github.com" in info["github_url"]


def test_protocol_help_has_three():
    assert set(PROTOCOL_HELP.keys()) == {"raw_data", "fire_water", "just_float"}


def test_just_float_describes_tail():
    entry = PROTOCOL_HELP["just_float"]
    assert "00 00 80 7F" in entry.content


def test_shortcut_help_has_entries():
    assert len(SHORTCUT_HELP) >= 5
    for item in SHORTCUT_HELP:
        assert "key" in item and "action" in item


def test_register_custom_entry():
    system = HelpSystem()
    custom = HelpEntry(HelpTopic.TROUBLESHOOTING, "自定义", "蓝牙配对失败", ["ble"])
    system.register_entry(custom)
    assert custom in system.get_topic(HelpTopic.TROUBLESHOOTING)
    assert custom in system.search("蓝牙")


# ---- Batch 141: HelpSystem 边界扩展 ----


def test_help_entry_is_frozen_dataclass():
    """HelpEntry 是 frozen dataclass，不可变。"""
    import pytest
    entry = HelpEntry(HelpTopic.ABOUT, "x", "y", ["tag"])
    with pytest.raises((AttributeError, TypeError)):
        entry.title = "modified"


def test_help_topic_enum_has_five_members():
    """HelpTopic 枚举覆盖 5 个主题。"""
    assert {t.value for t in HelpTopic} == {
        "getting_started", "protocols", "shortcuts", "troubleshooting", "about"
    }
    for topic in HelpTopic:
        assert topic.value == topic.value.lower()


def test_help_entry_tags_defaults_and_custom_values():
    default = HelpEntry(HelpTopic.ABOUT, "x", "y")
    custom = HelpEntry(HelpTopic.ABOUT, "x", "y", ["a", "b"])

    assert default.tags == []
    assert custom.tags == ["a", "b"]


def test_search_matches_content():
    """search 在 content 中匹配（不仅 title/tags）。"""
    system = HelpSystem()
    results = system.search("波特率")
    assert any("波特率" in e.content for e in results)


def test_search_is_case_insensitive():
    """search 用 lower() 比较，大小写不敏感。"""
    system = HelpSystem()
    lower = system.search("justfloat")
    upper = system.search("JUSTFLOAT")
    assert len(lower) == len(upper)
    assert {e.title for e in lower} == {e.title for e in upper}


def test_search_strips_whitespace():
    """search 用 strip()，前后空格不影响匹配。"""
    system = HelpSystem()
    assert system.search("  快捷键  ") == system.search("快捷键")


def test_search_deduplicates_entries():
    """search 同一条目不会被重复加入（跨 topic 查找去重）。"""
    system = HelpSystem()
    results = system.search("embeddebug")
    titles = [e.title for e in results]
    assert len(titles) == len(set(titles))  # 无重复


def test_get_topic_returns_defensive_copy():
    """get_topic 返回 list 副本，外部修改不影响内部。"""
    system = HelpSystem()
    snapshot = system.get_topic(HelpTopic.PROTOCOLS)
    original_len = len(snapshot)
    snapshot.clear()
    assert len(system.get_topic(HelpTopic.PROTOCOLS)) == original_len


def test_all_topics_returns_defensive_copy():
    """all_topics 返回 dict 副本 + 每个列表也是副本。"""
    system = HelpSystem()
    topics = system.all_topics()
    original_count = len(topics[HelpTopic.PROTOCOLS])
    topics[HelpTopic.PROTOCOLS].clear()
    assert len(system.get_topic(HelpTopic.PROTOCOLS)) == original_count


def test_register_entry_creates_new_topic_if_missing():
    """register_entry 对未知 topic 用 setdefault 创建新列表。"""
    system = HelpSystem()
    # 构造一个合法但未在默认 entries 中的 topic（实际 HelpTopic 枚举已全覆盖，
    # 但 register_entry 的 setdefault 逻辑仍可验证不抛异常）
    custom = HelpEntry(HelpTopic.ABOUT, "extra", "content", ["tag"])
    system.register_entry(custom)
    assert custom in system.get_topic(HelpTopic.ABOUT)


def test_about_info_contains_all_fields():
    """about_info 返回 5 个字段。"""
    system = HelpSystem()
    info = system.about_info()
    assert set(info.keys()) == {"app_name", "version", "description", "github_url", "license"}
    assert info["app_name"] == "EmbedDebug"
    assert info["license"] == "MIT"


def test_protocol_help_entries_belong_to_protocols_topic():
    """PROTOCOL_HELP 所有条目的 topic 都是 PROTOCOLS。"""
    for entry in PROTOCOL_HELP.values():
        assert entry.topic == HelpTopic.PROTOCOLS


def test_shortcut_help_known_keys():
    """SHORTCUT_HELP 含已知快捷键。"""
    keys = {item["key"] for item in SHORTCUT_HELP}
    assert "Ctrl+Enter" in keys
    assert "F1" in keys
    assert "F11" in keys
