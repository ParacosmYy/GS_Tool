"""Command entry text helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class CommandTextEdit(Protocol):
    """Minimal text-edit surface needed by command entry helpers."""

    def setText(self, text: str) -> None: ...


def apply_command_history_selection(edit: CommandTextEdit, text: str) -> None:
    if text:
        edit.setText(text)
