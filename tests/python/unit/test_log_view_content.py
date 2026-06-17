from __future__ import annotations

from embeddebug.serial_station.ui.log_view_content import clear_log_view


class LogView:
    def __init__(self) -> None:
        self.lines = ["TX AT", "RX OK"]
        self.clear_count = 0

    def clear(self) -> None:
        self.clear_count += 1
        self.lines.clear()


def test_clear_log_view_clears_existing_lines():
    view = LogView()

    clear_log_view(view)

    assert view.lines == []
    assert view.clear_count == 1
