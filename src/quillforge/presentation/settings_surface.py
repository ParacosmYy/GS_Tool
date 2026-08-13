"""Presentation composition for modal settings editing."""

from __future__ import annotations

from PyQt6.QtWidgets import QDialog, QWidget

from ..domain.models import SettingsSnapshot
from .settings_dialog import SettingsDialog


class SettingsSurface:
    """Own the settings dialog boundary without owning persistence policy."""

    def __init__(self, parent: QWidget) -> None:
        self._parent = parent

    def edit(self, settings: SettingsSnapshot) -> SettingsSnapshot | None:
        """Edit one snapshot and return a candidate only after explicit Save."""
        dialog = SettingsDialog(settings, self._parent)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return None
        return dialog.settings_snapshot()
