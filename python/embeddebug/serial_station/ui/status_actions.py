"""Status display actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol


class StatusActionHost(Protocol):
    """Minimal main-window surface needed by status action handlers."""

    def tr(self, text: str) -> str: ...


def show_error(host: StatusActionHost, message: str) -> None:
    host._status_label.setText(host.tr("Error: {message}").format(message=message))
