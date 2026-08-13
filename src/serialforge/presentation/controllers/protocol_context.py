"""Projection controller for the protocol editor context surface."""

from __future__ import annotations

from ..protocol_config_context_surface import project_protocol_config


def refresh_protocol_config_context(window) -> None:
    """Project native protocol controls and the existing status state only."""

    surface = getattr(window, "_protocol_config_context", None)
    status = getattr(window, "_protocol_status", None)
    if surface is None or status is None:
        return
    surface.set_projection(
        project_protocol_config(
            _current_text(window._protocol_framing, "原始字节流"),
            _current_text(window._protocol_checksum, "无"),
            window._protocol_max_frame.value(),
            status.property("state"),
        )
    )


def _current_text(combo, fallback: str) -> str:
    """Current text."""
    text = combo.currentText()
    return text if isinstance(text, str) and text else fallback


__all__ = ["refresh_protocol_config_context"]
