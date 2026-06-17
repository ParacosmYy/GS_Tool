"""Command history combo option helpers for Serial Station UI actions."""

from __future__ import annotations

from typing import Protocol


class CommandHistoryCombo(Protocol):
    """Minimal combo surface needed by command history option helpers."""

    def addItems(self, items: list[str]) -> None: ...

    def blockSignals(self, blocked: bool) -> None: ...

    def clear(self) -> None: ...

    def setCurrentText(self, text: str) -> None: ...

    def setEnabled(self, enabled: bool) -> None: ...


def populate_command_history_options(combo: CommandHistoryCombo, history: list[str]) -> None:
    """Populate command history options while avoiding selection signals."""

    combo.blockSignals(True)
    combo.clear()
    combo.addItems(history)
    if history:
        combo.setCurrentText(history[-1])
    combo.setEnabled(bool(history))
    combo.blockSignals(False)
