from __future__ import annotations

from embeddebug.serial_station.ui.command_entry_text import apply_command_history_selection


class LineEdit:
    def __init__(self) -> None:
        self.text_value = ""

    def setText(self, text: str) -> None:
        self.text_value = text


def test_apply_command_history_selection_writes_non_empty_text():
    edit = LineEdit()

    apply_command_history_selection(edit, "ATI")

    assert edit.text_value == "ATI"


def test_apply_command_history_selection_ignores_empty_text():
    edit = LineEdit()
    edit.text_value = "AT"

    apply_command_history_selection(edit, "")

    assert edit.text_value == "AT"
