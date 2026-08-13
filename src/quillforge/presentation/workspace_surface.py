"""Presentation composition for the workspace dock and navigation panel."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QDockWidget, QMainWindow

from ..domain.models import Locale, WorkspaceDirectory
from .i18n import tr
from .workspace_panel import WorkspacePanel


@dataclass(frozen=True, slots=True)
class WorkspaceSurfaceCallbacks:
    """Semantic workspace intents routed back to the owning shell."""

    folder_requested: Callable[[], None]
    file_picker_requested: Callable[[], None]
    directory_requested: Callable[[object], None]
    file_requested: Callable[[object], None]
    back_requested: Callable[[], None]
    cancel_requested: Callable[[], None]


class WorkspaceSurface:
    """Own the workspace dock projection without owning workspace operations."""

    def __init__(
        self,
        parent: QMainWindow,
        *,
        locale: Locale,
        callbacks: WorkspaceSurfaceCallbacks,
    ) -> None:
        self._panel = WorkspacePanel(parent, locale=locale)
        self._panel.folder_requested.connect(callbacks.folder_requested)
        self._panel.file_picker_requested.connect(callbacks.file_picker_requested)
        self._panel.directory_requested.connect(callbacks.directory_requested)
        self._panel.file_requested.connect(callbacks.file_requested)
        self._panel.back_requested.connect(callbacks.back_requested)
        self._panel.cancel_requested.connect(callbacks.cancel_requested)

        self._dock = QDockWidget(tr("workspace.dock", locale), parent)
        self._dock.setObjectName("WorkspaceDock")
        self._dock.setWidget(self._panel)
        parent.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, self._dock)

    @property
    def current_path(self) -> Path | None:
        """Return the current directory without exposing the Qt widget."""
        return self._panel.current_path

    def set_loading(self, loading: bool) -> None:
        """Project an active workspace operation into navigation controls."""
        self._panel.set_loading(loading)

    def set_directory(self, directory: WorkspaceDirectory, root: Path) -> None:
        """Project one current, bounded directory page into the panel."""
        self._panel.set_directory(directory, root)

    def show_error(self, message: str | Exception) -> None:
        """Project a recoverable workspace error without clearing the page."""
        self._panel.show_error(message)

    def set_locale(self, locale: Locale) -> None:
        """Retranslate the dock and panel while retaining its visible page."""
        self._dock.setWindowTitle(tr("workspace.dock", locale))
        self._panel.set_locale(locale)

    def refresh_icons(self) -> None:
        """Refresh the panel's theme-tinted icons without changing its page."""
        self._panel.refresh_icons()
