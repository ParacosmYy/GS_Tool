"""Log display actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.controllers import SerialWorkbenchLogEntry
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
    host._log_view.clear()
    for entry in host._controller.entries:
        if log_entry_visible(host, entry):
            append_log_line(host, entry)
    update_log_stats(host)


def log_entry_visible(host: LogActionHost, entry: SerialWorkbenchLogEntry) -> bool:
    selected = host._log_filter_combo.currentText()
    if selected == host.tr("TX"):
        direction_matches = entry.direction == "tx"
    elif selected == host.tr("RX"):
        direction_matches = entry.direction == "rx"
    else:
        direction_matches = True
    if not direction_matches:
        return False
    search_text = host._log_search_edit.text().strip().lower()
    if not search_text:
        return True
    prefix = "tx" if entry.direction == "tx" else "rx"
    return search_text in f"{prefix} {entry.text}".lower()


def update_log_stats(host: LogActionHost) -> None:
    entries = host._controller.entries
    total = len(entries)
    tx_count = sum(1 for entry in entries if entry.direction == "tx")
    rx_count = sum(1 for entry in entries if entry.direction == "rx")
    visible = sum(1 for entry in entries if log_entry_visible(host, entry))
    set_log_stats_label(host, visible=visible, total=total, tx=tx_count, rx=rx_count)
