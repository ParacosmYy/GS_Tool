from __future__ import annotations

from embeddebug.serial_station.ui.log_filter_options import (
    default_log_filter_text,
    log_filter_options,
)


def test_log_filter_options_return_default_visible_filters():
    assert log_filter_options() == ("All", "TX", "RX", "System", "Error")


def test_default_log_filter_text_matches_all_filter():
    assert default_log_filter_text() == "All"
