"""Fake RX injection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.status_messages import translated_result_message
from embeddebug.shared.results import OperationResult


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
        _set_result_status(
            host,
            result,
            success_text="Received fake bytes",
            failure_prefix="Inject failed",
        )
        return
    _set_result_status(host, result, success_text="", failure_prefix="Inject failed")


def _set_result_status(
    host: InjectionActionHost,
    result: OperationResult[object],
    *,
    success_text: str,
    failure_prefix: str,
) -> None:
    host._status_label.setText(
        translated_result_message(
            host,
            result,
            success_text=success_text,
            failure_prefix=failure_prefix,
        )
    )
