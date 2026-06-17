"""Fake RX injection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


class InjectionActionHost(Protocol):
    """Minimal main-window surface needed by injection action handlers."""

    def tr(self, text: str) -> str: ...


def inject_received(host: InjectionActionHost) -> None:
    text = host._inject_edit.text()
    if not text:
        set_status_text(host, "RX text is empty")
        return
    result = host._controller.inject_received_text(text)
    if result.ok:
        set_result_status(
            host,
            result,
            success_text="Received fake bytes",
            failure_prefix="Inject failed",
        )
        return
    set_result_status(host, result, success_text="", failure_prefix="Inject failed")
