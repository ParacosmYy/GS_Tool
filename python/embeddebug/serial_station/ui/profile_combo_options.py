"""Profile combo option helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class ProfileCombo(Protocol):
    """Minimal combo surface needed by profile option helpers."""

    def addItem(self, text: str) -> None: ...

    def findText(self, text: str) -> int: ...

    def setCurrentText(self, text: str) -> None: ...


def select_profile_combo_value(combo: ProfileCombo, value: str) -> None:
    if combo.findText(value) < 0:
        combo.addItem(value)
    combo.setCurrentText(value)
