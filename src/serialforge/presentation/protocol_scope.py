"""Pure presentation queries for protocol source applicability."""

from __future__ import annotations

from ..domain.models import TransportKind
from .connection_bindings import connection_shell_bindings_for


def parser_pipeline_supported(window) -> bool:
    """Return whether the selected source can feed the protocol parser."""

    if window._history_source_active:
        return True
    shell = connection_shell_bindings_for(window)
    if shell is None:
        return False
    try:
        transport = TransportKind(shell.transport_combo.currentData())
    except (AttributeError, TypeError, ValueError):
        return False
    return transport in {TransportKind.UART, TransportKind.TCP_STREAM}


def derived_source_supported(window) -> bool:
    """Return whether the current source can produce derived RX data."""

    if window._history_file_selected:
        return window._history_source_active
    return parser_pipeline_supported(window)


def derived_source_unavailable_text(window) -> str:
    """Explain why the current source cannot feed derived presentation surfaces."""

    if window._history_file_selected:
        return "当前历史文件没有可用 RX"
    return "当前实时传输 raw-only"
