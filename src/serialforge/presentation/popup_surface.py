"""Theme bridge for native combo-box popup window surfaces."""

from __future__ import annotations

from .property_refresh import refresh_dynamic_property
from .qt import QComboBox, QFrame, QWidget
from .theme_stylesheet_runtime import render_combo_popup_stylesheet


def refresh_combo_popup_theme(combo: QComboBox, theme_key: str) -> None:
    """Theme the popup frame while retaining Qt's native list-view behavior."""

    popup = combo.view().window()
    if not isinstance(popup, QFrame):
        return
    stylesheet = render_combo_popup_stylesheet(theme_key)
    if popup.styleSheet() != stylesheet:
        popup.setStyleSheet(stylesheet)
    refresh_dynamic_property(popup, "surfaceRole", "comboPopup")


def refresh_combo_popup_themes(root: QWidget, theme_key: str) -> None:
    """Refresh every combo popup below a composed presentation root."""

    combos = root.findChildren(QComboBox)
    if isinstance(root, QComboBox):
        combos = [root, *combos]
    for combo in combos:
        refresh_combo_popup_theme(combo, theme_key)


__all__ = ["refresh_combo_popup_theme", "refresh_combo_popup_themes"]
