"""Status message helpers for Serial Station UI actions."""

from __future__ import annotations

import logging
from typing import Protocol

from embeddebug.shared.results import OperationResult

_log = logging.getLogger(__name__)


class StatusLabel(Protocol):
    """Minimal label surface needed by status message helpers."""

    def setText(self, text: str) -> None: ...


class StatusMessageHost(Protocol):
    """Minimal host surface needed for translated status messages."""

    _status_label: StatusLabel

    def tr(self, text: str) -> str: ...


class ProfileLabelHost(Protocol):
    """Minimal host surface needed for translated profile label messages."""

    _profile_label: StatusLabel

    def tr(self, text: str) -> str: ...


class LogStatsLabelHost(Protocol):
    """Minimal host surface needed for translated log statistics messages."""

    _log_stats_label: StatusLabel

    def tr(self, text: str) -> str: ...


class PlainTextLogView(Protocol):
    """Minimal plain text view surface needed by log display helpers."""

    def appendPlainText(self, text: str) -> None: ...


class LogEntryLineHost(Protocol):
    """Minimal host surface needed for translated log entry lines."""

    _log_view: PlainTextLogView

    def tr(self, text: str) -> str: ...


def result_message(result: OperationResult[object], *, success_text: str, failure_prefix: str) -> str:
    """Return a display message for a controller operation result."""

    if result.ok:
        return success_text
    return f"{failure_prefix}: {result.message}"


def translated_result_message(
    host: StatusMessageHost,
    result: OperationResult[object],
    *,
    success_text: str,
    failure_prefix: str,
    **format_values: object,
) -> str:
    """Return a translated and formatted status message."""

    message = result_message(result, success_text=success_text, failure_prefix=failure_prefix)
    return host.tr(message).format(**format_values)


def set_result_status(
    host: StatusMessageHost,
    result: OperationResult[object],
    *,
    success_text: str,
    failure_prefix: str,
    **format_values: object,
) -> None:
    """Write a translated controller result message to the host status label."""

    host._status_label.setText(
        translated_result_message(
            host,
            result,
            success_text=success_text,
            failure_prefix=failure_prefix,
            **format_values,
        )
    )


def set_status_text(host: StatusMessageHost, text: str, **format_values: object) -> None:
    """Write a translated plain status message to the host status label."""

    message = host.tr(text).format(**format_values)
    host._status_label.setText(message)
    try:
        from embeddebug.serial_station.ui.animations.typewriter import TypewriterAnimation
        TypewriterAnimation.run_with_label(host._status_label, message).start()
    except Exception:
        _log.debug("typewriter status animation failed", exc_info=True)


def set_profile_label(host: ProfileLabelHost, name: str) -> None:
    """Write a translated profile name to the host profile label."""

    host._profile_label.setText(host.tr("Profile: {name}").format(name=name))


def set_log_stats_label(
    host: LogStatsLabelHost,
    *,
    visible: int,
    total: int,
    tx: int,
    rx: int,
    system: int,
    error: int,
) -> None:
    """Write translated log statistics to the host log statistics label."""

    host._log_stats_label.setText(
        host.tr(
            "Visible {visible} / Total {total} | TX {tx} | RX {rx} | System {system} | Error {error}"
        ).format(
            visible=visible,
            total=total,
            tx=tx,
            rx=rx,
            system=system,
            error=error,
        )
    )


def append_log_entry_line(host: LogEntryLineHost, *, direction: str, text: str) -> None:
    """Append a translated log line to the host log view."""

    templates = {
        "tx": "TX {text}",
        "rx": "RX {text}",
        "system": "System {text}",
        "error": "Error {text}",
    }
    template = templates.get(direction, "System {text}")
    host._log_view.appendPlainText(host.tr(template).format(text=text))
