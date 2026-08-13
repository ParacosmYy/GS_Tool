"""Typed wiring for the command-batch presentation surface."""

from __future__ import annotations

from dataclasses import dataclass

from .action_surface import ActionRailButton, BusyActionButton
from .command_batch_empty_state import CommandBatchEmptyState
from .command_batch_surface import CommandBatchSurfaceLabel
from .qt import QComboBox, QTableWidget


@dataclass(frozen=True, slots=True)
class CommandBatchControlBindings:
    """Immutable Qt references for one composed command-batch surface.

    The bundle contains presentation widgets only. Batch definitions,
    execution snapshots, and application callbacks remain owned by the
    ViewModel and command controller.
    """

    combo: QComboBox
    new_button: ActionRailButton
    edit_button: ActionRailButton
    delete_button: ActionRailButton
    run_button: ActionRailButton
    stop_button: BusyActionButton
    status: CommandBatchSurfaceLabel
    results: QTableWidget
    empty_state: CommandBatchEmptyState

    @property
    def empty_action_button(self) -> ActionRailButton:
        """Expose the empty-state action without leaking its widget tree."""

        return self.empty_state.action_button


def command_batch_bindings_for(window: object) -> CommandBatchControlBindings | None:
    """Read the typed batch bundle without importing the MainWindow class."""

    bindings = getattr(window, "_command_batch_bindings", None)
    return (
        bindings
        if isinstance(bindings, CommandBatchControlBindings)
        else None
    )


__all__ = ["CommandBatchControlBindings", "command_batch_bindings_for"]
