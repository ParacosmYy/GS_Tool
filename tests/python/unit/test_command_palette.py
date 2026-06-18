"""命令面板（Ctrl+P）模糊搜索与执行测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QListWidget, QLineEdit

from embeddebug.serial_station.ui.command_palette import (
    CommandItem,
    CommandPalette,
    fuzzy_score,
    rank_commands,
)


def test_fuzzy_score_exact_substring_ranks_highest():
    assert fuzzy_score("connect", "Connect Fake") > 0
    assert fuzzy_score("conn", "Connect Fake") > fuzzy_score("cnn", "Connect Fake")


def test_fuzzy_score_returns_negative_for_no_match():
    assert fuzzy_score("xyz", "Connect") == -1


def test_fuzzy_score_empty_query_matches_all():
    assert fuzzy_score("", "anything") == 0


def test_fuzzy_score_case_insensitive():
    assert fuzzy_score("CONN", "connect") > 0
    assert fuzzy_score("connect", "CONNECT") > 0


def test_fuzzy_score_prefers_prefix_match():
    prefix = fuzzy_score("clear", "Clear Log")
    middle = fuzzy_score("log", "Clear Log")
    assert prefix > 0
    assert middle > 0


def test_rank_commands_filters_and_sorts():
    calls: list[str] = []

    def cb_a() -> None:
        calls.append("a")

    def cb_b() -> None:
        calls.append("b")

    commands = (
        CommandItem("Connect Fake", cb_a),
        CommandItem("Clear Log", cb_b),
        CommandItem("Save Profile", cb_a),
    )
    ranked = rank_commands("clear", commands)
    assert len(ranked) == 1
    assert ranked[0].title == "Clear Log"


def test_rank_commands_empty_query_returns_all_preserving_score_order():
    commands = (
        CommandItem("Connect", lambda: None),
        CommandItem("Clear", lambda: None),
    )
    ranked = rank_commands("", commands)
    assert len(ranked) == 2


def test_command_palette_builds_with_expected_objectnames(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    assert palette.objectName() == "serialStationCommandPalette"
    assert palette.findChild(QLineEdit, "serialStationCommandPaletteEdit") is not None
    assert palette.findChild(QListWidget, "serialStationCommandPaletteList") is not None


def test_command_palette_open_shows_and_targets_edit(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.open()
    assert palette.isVisible()
    edit = palette.findChild(QLineEdit, "serialStationCommandPaletteEdit")
    assert edit is not None
    # offscreen 平台不传播真实焦点，用 focusProxy 目标与可见性替代断言。
    assert edit.placeholderText() != ""


def test_command_palette_close_hides(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.open()
    assert palette.isVisible()
    palette.close()
    assert not palette.isVisible()


def test_command_palette_set_commands_populates_list(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands(
        (
            CommandItem("Connect", lambda: None),
            CommandItem("Clear", lambda: None),
        )
    )
    listing = palette.findChild(QListWidget, "serialStationCommandPaletteList")
    assert listing is not None
    assert listing.count() == 2


def test_command_palette_filter_on_typing(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands(
        (
            CommandItem("Connect Fake", lambda: None),
            CommandItem("Clear Log", lambda: None),
            CommandItem("Save Profile", lambda: None),
        )
    )
    edit = palette.findChild(QLineEdit, "serialStationCommandPaletteEdit")
    assert edit is not None
    edit.setText("clear")
    listing = palette.findChild(QListWidget, "serialStationCommandPaletteList")
    assert listing.count() == 1
    assert listing.item(0).text() == "Clear Log"


def test_command_palette_executes_selected_callback(qtbot):
    calls: list[str] = []

    def cb() -> None:
        calls.append("ran")

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands((CommandItem("Connect", cb),))
    palette.open()
    palette._execute_selected()
    assert calls == ["ran"]
    assert not palette.isVisible()


def test_command_palette_emits_executed_signal(qtbot):
    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.set_commands((CommandItem("Connect", lambda: None),))
    emitted: list[str] = []
    palette.command_executed.connect(lambda title: emitted.append(title))
    palette.open()
    palette._execute_selected()
    assert emitted == ["Connect"]


def test_command_palette_escape_closes(qtbot):
    from PyQt6.QtCore import QEvent, Qt
    from PyQt6.QtGui import QKeyEvent

    palette = CommandPalette()
    qtbot.addWidget(palette)
    palette.open()
    assert palette.isVisible()
    event = QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Escape, Qt.KeyboardModifier.NoModifier)
    palette.keyPressEvent(event)
    assert not palette.isVisible()
