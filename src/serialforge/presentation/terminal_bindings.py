"""Typed presentation wiring for terminal, send, and history surfaces."""

from __future__ import annotations

from dataclasses import dataclass

from .action_surface import ActionRailButton, BusyActionButton
from .data_activity_surface import DataActivitySurface
from .qt import QCheckBox, QComboBox, QFrame, QLabel, QMenu, QToolButton
from .send_context_surface import SendContextSurface
from .send_input_surface import SendInputSurface
from .send_state_surface import SendStateSurface
from .terminal_surface import TerminalEmptyState, TerminalViewport


@dataclass(frozen=True, slots=True)
class TerminalControlBindings:
    """Immutable Qt references for the composed terminal workspace."""

    live_observation_band: QFrame
    terminal_surface: QFrame
    terminal: TerminalViewport
    terminal_empty_state: TerminalEmptyState
    display_mode: QComboBox
    pause_check: QCheckBox
    paused_label: QLabel
    clear_terminal_button: ActionRailButton
    record_button: BusyActionButton
    record_label: QLabel
    data_activity_label: DataActivitySurface
    send_control_band: QFrame
    send_mode: QComboBox
    send_input: SendInputSurface
    newline_check: QCheckBox
    send_button: ActionRailButton
    send_state_label: SendStateSurface
    send_context_surface: SendContextSurface
    quick_button: QToolButton
    quick_menu: QMenu
    save_quick_button: ActionRailButton
    history_combo: QComboBox
    clear_history_button: ActionRailButton


def terminal_bindings_for(window: object) -> TerminalControlBindings | None:
    """Read the typed terminal bundle without importing the MainWindow class."""

    bindings = getattr(window, "_terminal_bindings", None)
    return bindings if isinstance(bindings, TerminalControlBindings) else None


__all__ = ["TerminalControlBindings", "terminal_bindings_for"]
