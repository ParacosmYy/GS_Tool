"""Command history state helpers for the Serial Station controller."""

from __future__ import annotations


def remember_command(history: list[str], text: str) -> None:
    if text in history:
        history.remove(text)
    history.append(text)


def restore_command_history(history: list[str], values: object) -> None:
    history.clear()
    if not isinstance(values, list):
        return
    for value in values:
        if isinstance(value, str) and value:
            remember_command(history, value)
