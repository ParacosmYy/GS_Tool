from __future__ import annotations

from embeddebug.serial_station.controllers.command_history_state import remember_command, restore_command_history


def test_remember_command_moves_existing_command_to_latest_position():
    history = ["ping", "pong"]

    remember_command(history, "ping")

    assert history == ["pong", "ping"]


def test_restore_command_history_keeps_only_non_empty_strings_with_latest_order():
    history = ["stale"]

    restore_command_history(history, ["status?", "", 123, "reset", "status?"])

    assert history == ["reset", "status?"]
