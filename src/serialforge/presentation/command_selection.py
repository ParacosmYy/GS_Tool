"""Typed command-form selectors shared by presentation controllers."""

from __future__ import annotations

from ..domain.commands import CommandBatch
from ..domain.models import CommandMode
from .command_bindings import command_batch_bindings_for
from .qt import Qt
from .terminal_bindings import terminal_bindings_for


def current_send_mode(window) -> CommandMode:
    """Read the bounded send-mode selector from the presentation form."""

    terminal = terminal_bindings_for(window)
    if terminal is None:
        return CommandMode.TEXT
    value = terminal.send_mode.currentData()
    return value if isinstance(value, CommandMode) else CommandMode(value)


def selected_command_batch(window) -> CommandBatch | None:
    """Read the selected immutable command batch from the presentation form."""

    bindings = command_batch_bindings_for(window)
    if bindings is None:
        return None
    value = bindings.combo.currentData(Qt.ItemDataRole.UserRole)
    return value if isinstance(value, CommandBatch) else None


__all__ = ["current_send_mode", "selected_command_batch"]
