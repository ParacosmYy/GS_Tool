"""Log display actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.controllers import SerialWorkbenchLogEntry
from embeddebug.serial_station.ui.log_entry_filter import log_entry_matches_filter
from embeddebug.serial_station.ui.log_view_content import clear_log_view
from embeddebug.serial_station.ui.status_messages import append_log_entry_line, set_log_stats_label


class LogActionHost(Protocol):
    """Minimal main-window surface needed by log display action handlers."""

    def tr(self, text: str) -> str: ...


def append_log_entry(host: LogActionHost, entry: SerialWorkbenchLogEntry) -> None:
    if not log_entry_visible(host, entry):
        update_log_stats(host)
        return
    append_log_line(host, entry)
    update_log_stats(host)


def append_log_line(host: LogActionHost, entry: SerialWorkbenchLogEntry) -> None:
    append_log_entry_line(host, direction=entry.direction, text=entry.text)


def render_log_entries(host: LogActionHost) -> None:
    clear_log_view(host._log_view)
    for entry in host._controller.entries:
        if log_entry_visible(host, entry):
            append_log_line(host, entry)
    update_log_stats(host)


def log_entry_visible(host: LogActionHost, entry: SerialWorkbenchLogEntry) -> bool:
    return log_entry_matches_filter(
        entry,
        selected_filter=host._log_filter_combo.currentText(),
        search_text=host._log_search_edit.text(),
        tx_text=host.tr("TX"),
        rx_text=host.tr("RX"),
    )


def update_log_stats(host: LogActionHost) -> None:
    entries = host._controller.entries
    total = len(entries)
    tx_count = sum(1 for entry in entries if entry.direction == "tx")
    rx_count = sum(1 for entry in entries if entry.direction == "rx")
    visible = sum(1 for entry in entries if log_entry_visible(host, entry))
    set_log_stats_label(host, visible=visible, total=total, tx=tx_count, rx=rx_count)
