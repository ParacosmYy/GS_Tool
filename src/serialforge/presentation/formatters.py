"""Bounded presentation labels shared by independent workspace controllers."""

from __future__ import annotations

from ..domain.commands import CommandEntry
from ..domain.models import CommandMode, Endpoint


def endpoint_label(endpoint: Endpoint) -> str:
    """Format an endpoint for user-facing selectors without changing its identity."""

    fields = [endpoint.address]
    if endpoint.label and endpoint.label != endpoint.address:
        fields.append(endpoint.label)
    metadata = dict(endpoint.metadata)
    for key in ("serial_number", "vid", "pid", "location"):
        value = metadata.get(key)
        if value:
            fields.append(f"{key}={value}")
    return " · ".join(fields)


def history_label(entry: CommandEntry) -> str:
    """Format a bounded command-history preview for the terminal selector."""

    preview = (
        entry.payload.hex(" ").upper()
        if entry.mode is CommandMode.HEX
        else entry.payload.decode("utf-8", errors="replace")
    )
    suffix = " + CRLF" if entry.append_newline else ""
    return f"{preview[:100]}{suffix}"


__all__ = ["endpoint_label", "history_label"]
