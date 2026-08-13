"""Presentation composition for the command-palette modal flow."""

from __future__ import annotations

from collections.abc import Iterable

from PyQt6.QtWidgets import QDialog, QWidget

from ..application.commands import Command
from ..domain.models import Locale
from .command_palette import CommandPaletteDialog
from .i18n import normalize_locale


class CommandPaletteSurface:
    """Own command-palette modal composition without executing commands."""

    def __init__(self, parent: QWidget, *, locale: Locale) -> None:
        self._parent = parent
        self._locale = normalize_locale(locale)

    def set_locale(self, locale: Locale) -> None:
        """Set the locale used by the next command-palette invocation."""
        self._locale = normalize_locale(locale)

    def choose(self, commands: Iterable[Command]) -> str | None:
        """Return the selected stable command ID after explicit acceptance."""
        dialog = CommandPaletteDialog(commands, self._parent, locale=self._locale)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return None
        return dialog.selected_command_id()
