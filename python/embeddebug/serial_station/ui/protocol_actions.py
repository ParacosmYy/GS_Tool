"""Protocol selection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol


class ProtocolActionHost(Protocol):
    """Minimal main-window surface needed by protocol action handlers."""

    def tr(self, text: str) -> str: ...


def select_protocol(host: ProtocolActionHost, name: str) -> None:
    host._controller.set_protocol(name)
    host._status_label.setText(host.tr("Protocol: {name}").format(name=name))
