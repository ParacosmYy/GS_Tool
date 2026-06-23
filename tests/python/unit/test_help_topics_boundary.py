"""help/topics HelpTopic 枚举 + HelpEntry 边界测试。

HelpTopic 枚举 + HelpEntry frozen/tags 此前经 test_help_system 间接测试。
本文件覆盖枚举值 + frozen + tags 默认 + PROTOCOL_HELP keys。

覆盖：
1. HelpTopic 5 成员。
2. HelpTopic 值小写。
3. HelpEntry frozen。
4. HelpEntry 默认 tags 空 list。
5. HelpEntry 自定义 tags。
6. PROTOCOL_HELP 含 3 个协议。
7. SHORTCUT_HELP 含 Ctrl+Enter。
8. SHORTCUT_HELP 含 F1。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.help.topics import (
    PROTOCOL_HELP,
    SHORTCUT_HELP,
    HelpEntry,
    HelpTopic,
)


def test_help_topic_has_five_members():
    assert len(HelpTopic) == 5


def test_help_topic_values_lowercase():
    for topic in HelpTopic:
        assert topic.value == topic.value.lower()


def test_help_entry_is_frozen():
    entry = HelpEntry(topic=HelpTopic.ABOUT, title="x", content="y")
    with pytest.raises(AttributeError):
        entry.title = "z"  # type: ignore[misc]


def test_help_entry_default_tags_empty():
    entry = HelpEntry(topic=HelpTopic.ABOUT, title="x", content="y")
    assert entry.tags == []


def test_help_entry_custom_tags():
    entry = HelpEntry(topic=HelpTopic.ABOUT, title="x", content="y", tags=["a", "b"])
    assert entry.tags == ["a", "b"]


def test_protocol_help_has_three():
    assert len(PROTOCOL_HELP) == 3
    assert set(PROTOCOL_HELP.keys()) == {"raw_data", "fire_water", "just_float"}


def test_shortcut_help_contains_ctrl_enter():
    keys = {item["key"] for item in SHORTCUT_HELP}
    assert "Ctrl+Enter" in keys


def test_shortcut_help_contains_f1():
    keys = {item["key"] for item in SHORTCUT_HELP}
    assert "F1" in keys
