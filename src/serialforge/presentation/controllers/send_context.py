"""Projection controller for the native send form context surface."""

from __future__ import annotations

from ..command_selection import current_send_mode
from ..send_context_surface import project_send_form
from ..terminal_bindings import terminal_bindings_for


def refresh_send_context(window) -> None:
    """Project existing send controls without changing their submission path."""

    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    projection = project_send_form(
        terminal.send_input.text(),
        current_send_mode(window),
        terminal.newline_check.isChecked(),
    )
    terminal.send_context_surface.set_projection(projection)


__all__ = ["refresh_send_context"]
