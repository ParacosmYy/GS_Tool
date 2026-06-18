"""ShortcutManager 行为域单测。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtWidgets import QMainWindow

from embeddebug.serial_station.shortcuts import DEFAULT_SHORTCUTS, ShortcutCategory, ShortcutDef, ShortcutManager


class _FakeHost(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.calls: list[str] = []

    def send(self) -> None:
        self.calls.append("send")

    def clear(self) -> None:
        self.calls.append("clear")


def test_default_shortcuts_count():
    assert len(DEFAULT_SHORTCUTS) == 15


def test_default_ids_unique():
    ids = [d.id for d in DEFAULT_SHORTCUTS]
    assert len(ids) == len(set(ids))


def test_grouped_by_category(manager_fixture):
    groups = manager_fixture.grouped_by_category()
    assert sum(len(v) for v in groups.values()) == len(DEFAULT_SHORTCUTS)


def test_register_installs_per_definition(manager_fixture):
    assert len(manager_fixture._shortcuts) == len(DEFAULT_SHORTCUTS)


def test_rebind_changes_key(manager_fixture):
    assert manager_fixture.key_sequence_for("send") == "Ctrl+Return"
    assert manager_fixture.rebind("send", "Ctrl+Enter") is True
    assert manager_fixture.key_sequence_for("send") == "Ctrl+Enter"


def test_rebind_unknown_returns_false(manager_fixture):
    assert manager_fixture.rebind("ghost", "Ctrl+X") is False


def test_shortcuts_reset_restores_defaults(manager_fixture):
    manager_fixture.rebind("send", "Ctrl+Enter")
    manager_fixture.reset_to_defaults()
    assert manager_fixture.overrides == {}
    assert manager_fixture.key_sequence_for("send") == "Ctrl+Return"


def test_to_dict_from_dict_round_trip(manager_fixture):
    manager_fixture.rebind("send", "Ctrl+Enter")
    data = manager_fixture.to_dict()
    restored = ShortcutManager.from_dict(data)
    assert restored.overrides == {"send": "Ctrl+Enter"}


def test_from_dict_drops_unknown():
    m = ShortcutManager.from_dict({"overrides": {"send": "Ctrl+X", "ghost": "Ctrl+Z"}})
    assert "ghost" not in m.overrides


def test_key_sequence_for_unknown_returns_none(manager_fixture):
    assert manager_fixture.key_sequence_for("ghost") is None


def test_shortcut_def_is_frozen():
    d = ShortcutDef("x", ShortcutCategory.EDIT, "Ctrl+X", "测试", "x")
    with pytest.raises(Exception):
        d.id = "y"


@pytest.fixture()
def manager_fixture(qtbot):
    host = _FakeHost()
    qtbot.addWidget(host)
    m = ShortcutManager()
    m.register(host)
    return m
