"""Log entry filter helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class LogEntry(Protocol):
    """Minimal log entry surface needed by filter helpers."""

    direction: str
    text: str


def log_entry_matches_filter(
    entry: LogEntry,
    *,
    selected_filter: str,
    search_text: str,
    tx_text: str,
    rx_text: str,
) -> bool:
    if selected_filter == tx_text:
        direction_matches = entry.direction == "tx"
    elif selected_filter == rx_text:
        direction_matches = entry.direction == "rx"
    else:
        direction_matches = True
    if not direction_matches:
        return False

    normalized_search = search_text.strip().lower()
    if not normalized_search:
        return True
    prefix = "tx" if entry.direction == "tx" else "rx"
    return normalized_search in f"{prefix} {entry.text}".lower()
