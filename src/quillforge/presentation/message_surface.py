"""Presentation composition for common shell message dialogs."""

from __future__ import annotations

from typing import Literal

from PyQt6.QtWidgets import QMessageBox, QWidget

from ..domain.models import Locale
from .i18n import localize_exception, localize_message, normalize_locale, tr

SaveBeforeCloseDecision = Literal["save", "discard", "cancel"]


class MessageSurface:
    """Own common message-dialog composition without owning shell policy."""

    def __init__(self, parent: QWidget, *, locale: Locale) -> None:
        self._parent = parent
        self._locale = normalize_locale(locale)

    def set_locale(self, locale: Locale) -> None:
        """Set the locale used by the next message dialog."""
        self._locale = normalize_locale(locale)

    def ask_save_before_close(self, name: str) -> SaveBeforeCloseDecision:
        """Return the user's save, discard, or cancel decision."""
        box = self._new_message_box("messageDialog", QMessageBox.Icon.Question)
        box.setWindowTitle(tr("error.unsaved", self._locale))
        box.setText(tr("dialog.save_before_close", self._locale, name=name))
        box.setStandardButtons(
            QMessageBox.StandardButton.Save
            | QMessageBox.StandardButton.Discard
            | QMessageBox.StandardButton.Cancel
        )
        self._set_button_text(box, QMessageBox.StandardButton.Save, "dialog.button.save")
        self._set_button_text(box, QMessageBox.StandardButton.Discard, "dialog.button.discard")
        self._set_button_text(box, QMessageBox.StandardButton.Cancel, "dialog.button.cancel")
        self._set_button_role(box, QMessageBox.StandardButton.Save, "primaryAction")
        self._set_button_role(box, QMessageBox.StandardButton.Discard, "warningAction")
        self._set_button_role(box, QMessageBox.StandardButton.Cancel, "quietAction")
        box.setDefaultButton(QMessageBox.StandardButton.Save)
        choice = box.exec()
        if choice == QMessageBox.StandardButton.Save:
            return "save"
        if choice == QMessageBox.StandardButton.Discard:
            return "discard"
        return "cancel"

    def show_about(self) -> None:
        """Show the localized product information dialog."""
        box = self._new_message_box("aboutDialog", QMessageBox.Icon.Information)
        box.setWindowTitle(tr("about.title", self._locale))
        box.setText(tr("about.body", self._locale))
        box.setStandardButtons(QMessageBox.StandardButton.Ok)
        self._set_button_text(box, QMessageBox.StandardButton.Ok, "dialog.button.ok")
        self._set_button_role(box, QMessageBox.StandardButton.Ok, "quietAction")
        box.exec()

    def show_error(self, title: str, message: str | Exception) -> None:
        """Show one localized recoverable error message."""
        box = self._new_message_box("errorDialog", QMessageBox.Icon.Critical)
        box.setWindowTitle(localize_message(title, self._locale))
        box.setText(
            localize_exception(message, self._locale)
            if isinstance(message, Exception)
            else localize_message(message, self._locale)
        )
        box.setStandardButtons(QMessageBox.StandardButton.Ok)
        self._set_button_text(box, QMessageBox.StandardButton.Ok, "dialog.button.ok")
        self._set_button_role(box, QMessageBox.StandardButton.Ok, "quietAction")
        box.exec()

    def _new_message_box(self, object_name: str, icon: QMessageBox.Icon) -> QMessageBox:
        box = QMessageBox(self._parent)
        box.setObjectName(object_name)
        box.setIcon(icon)
        return box

    def _set_button_text(
        self,
        box: QMessageBox,
        standard_button: QMessageBox.StandardButton,
        key: str,
    ) -> None:
        button = box.button(standard_button)
        if button is not None:
            button.setText(tr(key, self._locale))

    @staticmethod
    def _set_button_role(
        box: QMessageBox,
        standard_button: QMessageBox.StandardButton,
        object_name: str,
    ) -> None:
        button = box.button(standard_button)
        if button is not None:
            button.setObjectName(object_name)
