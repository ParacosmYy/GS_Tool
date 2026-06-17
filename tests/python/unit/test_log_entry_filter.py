from __future__ import annotations

from dataclasses import dataclass

from embeddebug.serial_station.ui.log_entry_filter import log_entry_matches_filter


@dataclass(frozen=True)
class Entry:
    direction: str
    text: str


def test_log_entry_matches_filter_accepts_all_direction_without_search():
    assert log_entry_matches_filter(
        Entry("rx", "OK"), selected_filter="All", search_text="", tx_text="TX", rx_text="RX"
    )


def test_log_entry_matches_filter_rejects_non_matching_direction():
    assert not log_entry_matches_filter(
        Entry("rx", "OK"), selected_filter="TX", search_text="", tx_text="TX", rx_text="RX"
    )


def test_log_entry_matches_filter_searches_direction_prefix_and_text():
    assert log_entry_matches_filter(
        Entry("tx", "AT+RST"), selected_filter="All", search_text="tx at", tx_text="TX", rx_text="RX"
    )
    assert not log_entry_matches_filter(
        Entry("rx", "OK"), selected_filter="All", search_text="tx at", tx_text="TX", rx_text="RX"
    )
