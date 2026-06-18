"""Command send and history actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.command_entry_text import apply_command_history_selection
from embeddebug.serial_station.ui.command_history_options import populate_command_history_options
from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


class CommandActionHost(Protocol):
    """Minimal main-window surface needed by command action handlers."""

    def tr(self, text: str) -> str: ...

    def _refresh_command_history(self) -> None: ...


def send_text(host: CommandActionHost) -> None:
    text = host._send_edit.text()
    if not text:
        set_status_text(host, "Command is empty")
        return
    result = host._controller.send_text_result(text)
    if result.ok:
        refresh_command_history(host)
        set_result_status(host, result, success_text="Command sent", failure_prefix="Send failed")
        return
    set_result_status(host, result, success_text="", failure_prefix="Send failed")


def refresh_command_history(host: CommandActionHost) -> None:
    history = host._controller.command_history
    populate_command_history_options(host._command_history_combo, history)


def select_command_history(host: CommandActionHost, text: str) -> None:
    apply_command_history_selection(host._send_edit, text)
