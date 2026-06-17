from __future__ import annotations

from embeddebug.serial_station.ui.profile_combo_options import select_profile_combo_value


class Combo:
    def __init__(self, items: list[str]) -> None:
        self.items = items[:]
        self.current = ""

    def addItem(self, text: str) -> None:
        self.items.append(text)

    def findText(self, text: str) -> int:
        try:
            return self.items.index(text)
        except ValueError:
            return -1

    def setCurrentText(self, text: str) -> None:
        self.current = text


def test_select_profile_combo_value_selects_existing_option_without_duplicate():
    combo = Combo(["raw_data", "just_float"])

    select_profile_combo_value(combo, "just_float")

    assert combo.items == ["raw_data", "just_float"]
    assert combo.current == "just_float"


def test_select_profile_combo_value_adds_missing_option_before_selecting():
    combo = Combo(["COM1"])

    select_profile_combo_value(combo, "COM9")

    assert combo.items == ["COM1", "COM9"]
    assert combo.current == "COM9"
