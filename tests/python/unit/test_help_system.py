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
