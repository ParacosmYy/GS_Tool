"""Log view content helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class LogView(Protocol):
    """Minimal log view surface needed by content helpers."""

    def clear(self) -> None: ...


def clear_log_view(view: LogView) -> None:
    view.clear()
