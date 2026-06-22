"""command_history_state 单元测试 — remember + restore。"""

from __future__ import annotations

from embeddebug.serial_station.controllers.command_history_state import (
    remember_command,
    restore_command_history,
)


def test_remember_new_command():
    h = []
    remember_command(h, "AT")
    assert h == ["AT"]


def test_remember_moves_to_end():
    h = ["AT", "ATZ"]
    remember_command(h, "AT")
    assert h == ["ATZ", "AT"]


def test_remember_multiple_unique():
    h = []
    remember_command(h, "a")
    remember_command(h, "b")
    remember_command(h, "c")
    assert h == ["a", "b", "c"]


def test_remember_duplicate_only_once():
    h = []
    remember_command(h, "x")
    remember_command(h, "x")
    assert h == ["x"]


def test_restore_from_list():
    h = ["old"]
    restore_command_history(h, ["a", "b"])
    assert h == ["a", "b"]


def test_restore_clears_existing():
    h = ["old1", "old2"]
    restore_command_history(h, [])
    assert h == []


def test_restore_non_list_ignored():
    h = ["keep"]
    restore_command_history(h, "not a list")
    assert h == []


def test_restore_filters_non_string():
    h = []
    restore_command_history(h, ["valid", 123, "", "also_valid"])
    assert h == ["valid", "also_valid"]
