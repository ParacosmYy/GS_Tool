from __future__ import annotations

from embeddebug.serial_station.ui.profile_name_text import apply_profile_name_text


class LineEdit:
    def __init__(self) -> None:
        self.text_value = "old"

    def setText(self, text: str) -> None:
        self.text_value = text


def test_apply_profile_name_text_writes_loaded_profile_name():
    edit = LineEdit()

    apply_profile_name_text(edit, "factory-a")

    assert edit.text_value == "factory-a"


def test_apply_profile_name_text_allows_empty_name_to_clear_stale_text():
    edit = LineEdit()

    apply_profile_name_text(edit, "")

    assert edit.text_value == ""
