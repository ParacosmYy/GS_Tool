"""Log filter option helpers for Serial Station widgets."""

from __future__ import annotations


_LOG_FILTER_OPTIONS = ("All", "TX", "RX", "System", "Error")


def log_filter_options() -> tuple[str, ...]:
    return _LOG_FILTER_OPTIONS


def default_log_filter_text() -> str:
    return _LOG_FILTER_OPTIONS[0]
