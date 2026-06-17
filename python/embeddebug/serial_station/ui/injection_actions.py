"""Fake RX injection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol


class InjectionActionHost(Protocol):
    """Minimal main-window surface needed by injection action handlers."""

    def tr(self, text: str) -> str: ...


def inject_received(host: InjectionActionHost) -> None:
    text = host._inject_edit.text()
    if not text:
        host._status_label.setText(host.tr("RX text is empty"))
        return
    result = host._controller.inject_received_text(text)
    if result.ok:
        host._status_label.setText(host.tr("Received fake bytes"))
        return
    host._status_label.setText(host.tr("Inject failed: {message}").format(message=result.message))
