"""Theme bridge for Qt file-selection surfaces.

The helper keeps file-system navigation and acceptance semantics in Qt's
``QFileDialog`` while giving its non-native child surface the active
SerialForge theme.  It owns no paths, persistence, or application state.
"""

from __future__ import annotations

from .popup_surface import refresh_combo_popup_themes
from .qt import QDialog, QFileDialog, QWidget
from .theme import theme_key_for_widget
from .theme_stylesheet_runtime import render_file_dialog_stylesheet


def configure_file_dialog(dialog: QFileDialog, theme_key: str) -> None:
    """Apply the bounded theme bridge without changing file-dialog semantics."""

    dialog.setOption(QFileDialog.Option.DontUseNativeDialog, True)
    dialog.setStyleSheet(render_file_dialog_stylesheet(theme_key))
    refresh_combo_popup_themes(dialog, theme_key)


def _run_file_dialog(dialog: QFileDialog) -> tuple[str, str]:
    """Run file dialog."""
    if dialog.exec() != QDialog.DialogCode.Accepted:
        return "", ""
    selected_files = dialog.selectedFiles()
    selected_path = selected_files[0] if selected_files else ""
    return selected_path, dialog.selectedNameFilter()


def get_open_file_name(
    parent: QWidget,
    caption: str,
    directory: str,
    file_filter: str,
) -> tuple[str, str]:
    """Return one existing file using a themed Qt open dialog."""

    dialog = QFileDialog(parent, caption, directory, file_filter)
    dialog.setFileMode(QFileDialog.FileMode.ExistingFile)
    dialog.setAcceptMode(QFileDialog.AcceptMode.AcceptOpen)
    configure_file_dialog(dialog, theme_key_for_widget(parent))
    return _run_file_dialog(dialog)


def get_save_file_name(
    parent: QWidget,
    caption: str,
    directory: str,
    file_filter: str,
) -> tuple[str, str]:
    """Return a save target using a themed Qt save dialog."""

    dialog = QFileDialog(parent, caption, directory, file_filter)
    dialog.setFileMode(QFileDialog.FileMode.AnyFile)
    dialog.setAcceptMode(QFileDialog.AcceptMode.AcceptSave)
    configure_file_dialog(dialog, theme_key_for_widget(parent))
    return _run_file_dialog(dialog)


__all__ = [
    "configure_file_dialog",
    "get_open_file_name",
    "get_save_file_name",
]
