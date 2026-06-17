"""Protocol selection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.status_messages import set_status_text


class ProtocolActionHost(Protocol):
    """Minimal main-window surface needed by protocol action handlers."""

    def tr(self, text: str) -> str: ...


def select_protocol(host: ProtocolActionHost, name: str) -> None:
    host._controller.set_protocol(name)
    set_status_text(host, "Protocol: {name}", name=name)
