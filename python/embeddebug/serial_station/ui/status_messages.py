"""Status message helpers for Serial Station UI actions."""

from __future__ import annotations

from typing import Protocol

from embeddebug.shared.results import OperationResult


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

    host._status_label.setText(host.tr(text).format(**format_values))


def set_profile_label(host: ProfileLabelHost, name: str) -> None:
    """Write a translated profile name to the host profile label."""

    host._profile_label.setText(host.tr("Profile: {name}").format(name=name))
