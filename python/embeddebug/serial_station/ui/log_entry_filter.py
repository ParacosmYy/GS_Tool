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
    system_text: str,
    error_text: str,
) -> bool:
    filtered_directions = {
        tx_text: "tx",
        rx_text: "rx",
        system_text: "system",
        error_text: "error",
    }
    expected_direction = filtered_directions.get(selected_filter)
    direction_matches = expected_direction is None or entry.direction == expected_direction
    if not direction_matches:
        return False

    normalized_search = search_text.strip().lower()
    if not normalized_search:
        return True
    return normalized_search in f"{entry.direction} {entry.text}".lower()
