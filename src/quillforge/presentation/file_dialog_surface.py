"""Presentation composition for file and folder selection dialogs."""

from __future__ import annotations

from pathlib import Path

from PyQt6.QtWidgets import QFileDialog, QWidget

from ..domain.models import Locale
from .i18n import normalize_locale, tr


class FileDialogSurface:
    """Own native path-picking dialogs without owning path policy."""

    def __init__(self, parent: QWidget, *, locale: Locale) -> None:
        self._parent = parent
        self._locale = normalize_locale(locale)

    def set_locale(self, locale: Locale) -> None:
        """Set the locale used by the next native dialog."""
        self._locale = normalize_locale(locale)

    def choose_document(self) -> Path | None:
        """Choose one file to open; directories are handled by the workspace flow."""
        selected, _ = QFileDialog.getOpenFileName(
            self._parent,
            tr("dialog.open_document", self._locale),
            "",
            tr("dialog.text_filter", self._locale),
        )
        return Path(selected) if selected else None

    def choose_workspace(self) -> Path | None:
        """Choose one directory to use as the workspace root."""
        selected = QFileDialog.getExistingDirectory(
            self._parent,
            tr("dialog.open_workspace", self._locale),
        )
        return Path(selected) if selected else None

    def choose_save_path(self, current: Path | None) -> Path | None:
        """Choose a destination path while preserving the current default name."""
        selected, _ = QFileDialog.getSaveFileName(
            self._parent,
            tr("dialog.save_document", self._locale),
            (
                str(current)
                if current is not None
                else f"{tr('document.untitled', self._locale)}.txt"
            ),
            tr("dialog.text_filter", self._locale),
        )
        return Path(selected) if selected else None
