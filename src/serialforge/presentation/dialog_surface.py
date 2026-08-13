"""Presentation-only configuration for high-risk confirmation dialogs."""

from __future__ import annotations

from .property_refresh import refresh_dynamic_property
from .qt import QMessageBox


def configure_confirmation_dialog(
    dialog: QMessageBox,
    *,
    accessible_name: str,
    accessible_description: str,
    confirm_text: str,
    confirm_description: str,
    cancel_text: str,
    cancel_description: str,
) -> None:
    """Apply shared visual and assistive semantics without owning dialog flow."""

    refresh_dynamic_property(dialog, "surfaceRole", "confirmation")
    dialog.setAccessibleName(accessible_name)
    dialog.setAccessibleDescription(accessible_description)

    confirm_button = dialog.button(QMessageBox.StandardButton.Ok)
    if confirm_button is not None:
        confirm_button.setObjectName("dangerButton")
        confirm_button.setText(confirm_text)
        confirm_button.setAccessibleName(confirm_text)
        confirm_button.setAccessibleDescription(confirm_description)
        confirm_button.setToolTip(confirm_description)

    cancel_button = dialog.button(QMessageBox.StandardButton.Cancel)
    if cancel_button is not None:
        cancel_button.setText(cancel_text)
        cancel_button.setAccessibleName(cancel_text)
        cancel_button.setAccessibleDescription(cancel_description)
        cancel_button.setToolTip(cancel_description)


__all__ = ["configure_confirmation_dialog"]
