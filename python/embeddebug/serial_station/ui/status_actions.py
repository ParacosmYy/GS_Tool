"""Status display actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.status_messages import set_status_text


class StatusActionHost(Protocol):
    """Minimal main-window surface needed by status action handlers."""

    def tr(self, text: str) -> str: ...


def show_error(host: StatusActionHost, message: str) -> None:
    set_status_text(host, "Error: {message}", message=message)
