from __future__ import annotations

from embeddebug.serial_station.ui.command_history_options import populate_command_history_options


class Combo:
    def __init__(self) -> None:
        self.enabled = False
        self.items: list[str] = []
        self.current = ""
        self.signal_blocks: list[bool] = []

    def addItems(self, items: list[str]) -> None:
        self.items.extend(items)

    def blockSignals(self, blocked: bool) -> None:
        self.signal_blocks.append(blocked)

    def clear(self) -> None:
        self.items.clear()
        self.current = ""

    def setCurrentText(self, text: str) -> None:
        self.current = text

    def setEnabled(self, enabled: bool) -> None:
        self.enabled = enabled


def test_populate_command_history_options_selects_latest_and_unblocks_signals():
    combo = Combo()

    populate_command_history_options(combo, ["AT", "ATI"])

    assert combo.items == ["AT", "ATI"]
    assert combo.current == "ATI"
    assert combo.enabled
    assert combo.signal_blocks == [True, False]


def test_populate_command_history_options_disables_empty_history():
    combo = Combo()
    combo.enabled = True
    combo.current = "AT"
    combo.items = ["AT"]

    populate_command_history_options(combo, [])

    assert combo.items == []
    assert combo.current == ""
    assert not combo.enabled
    assert combo.signal_blocks == [True, False]
