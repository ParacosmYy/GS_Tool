"""Presentation composition for the recoverable-snapshot prompt."""

from __future__ import annotations

from datetime import datetime
from pathlib import Path
from typing import Literal

from PyQt6.QtWidgets import QMessageBox, QWidget

from ..domain.models import Locale
from .i18n import normalize_locale, tr

RecoveryPromptDecision = Literal["restore", "discard", "later"]


class RecoveryPromptSurface:
    """Render one recovery choice without owning recovery service policy."""

    def __init__(self, parent: QWidget, *, locale: Locale) -> None:
        self._parent = parent
        self._locale = normalize_locale(locale)

    def set_locale(self, locale: Locale) -> None:
        """Set the locale used by the next prompt."""
        self._locale = normalize_locale(locale)

    def choose(
        self,
        *,
        path: Path | None,
        created_at_ns: int,
        source_status: str,
    ) -> RecoveryPromptDecision:
        """Return the user's restore, discard, or defer decision."""
        path_text = str(path) if path is not None else tr("recovery.untitled", self._locale)
        created_text = _format_recovery_time(created_at_ns)
        source_key = {
            "untitled": "recovery.source.untitled",
            "unchanged": "recovery.source.unchanged",
            "changed": "recovery.source.changed",
            "missing": "recovery.source.missing",
            "unavailable": "recovery.source.unavailable",
        }.get(source_status, "recovery.source.unknown")
        source_text = tr(source_key, self._locale)
        box = QMessageBox(self._parent)
        box.setObjectName("recoveryPrompt")
        box.setWindowTitle(tr("recovery.title", self._locale))
        box.setText(tr("recovery.available", self._locale, path=path_text))
        box.setInformativeText(
            tr("recovery.details", self._locale, created=created_text, source=source_text)
        )
        restore = box.addButton(
            tr("recovery.restore", self._locale), QMessageBox.ButtonRole.AcceptRole
        )
        restore.setObjectName("primaryAction")
        discard = box.addButton(
            tr("recovery.discard", self._locale), QMessageBox.ButtonRole.DestructiveRole
        )
        discard.setObjectName("warningAction")
        later = box.addButton(tr("recovery.later", self._locale), QMessageBox.ButtonRole.RejectRole)
        later.setObjectName("quietAction")
        box.exec()
        clicked = box.clickedButton()
        if clicked is restore:
            return "restore"
        if clicked is discard:
            return "discard"
        return "later"


def _format_recovery_time(created_at_ns: int) -> str:
    return (
        datetime.fromtimestamp(created_at_ns / 1_000_000_000)
        .astimezone()
        .strftime("%Y-%m-%d %H:%M:%S")
    )
