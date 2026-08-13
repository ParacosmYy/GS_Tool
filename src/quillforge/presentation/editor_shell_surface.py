"""Presentation composition for the central editor shell."""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtWidgets import QMainWindow, QVBoxLayout, QWidget

from ..domain.models import Locale
from .document_tab_surface import DocumentTabLike, DocumentTabSurface
from .find_surface import FindSurface, FindSurfaceCallbacks


class EditorShellSurface[TabT: DocumentTabLike]:
    """Own the central editor layout while leaving document policy in MainWindow."""

    def __init__(
        self,
        parent: QMainWindow,
        *,
        locale: Locale,
        close_requested: Callable[[int], None],
        current_changed: Callable[[int], None],
        find_callbacks: FindSurfaceCallbacks,
    ) -> None:
        self._shell = QWidget(parent)
        self._shell.setObjectName("editorShell")
        shell_layout = QVBoxLayout(self._shell)
        shell_layout.setContentsMargins(10, 8, 10, 8)
        shell_layout.setSpacing(8)
        self._tabs = DocumentTabSurface[TabT](
            parent,
            close_requested=close_requested,
            current_changed=current_changed,
        )
        shell_layout.addWidget(self._tabs.widget)
        self._find = FindSurface(
            self._shell,
            locale=locale,
            callbacks=find_callbacks,
        )
        self._find.hide()
        shell_layout.addWidget(self._find.widget)

    @property
    def widget(self) -> QWidget:
        """Return the central shell widget for QMainWindow composition."""
        return self._shell

    @property
    def tabs(self) -> DocumentTabSurface[TabT]:
        """Return the semantic document-tab surface."""
        return self._tabs

    @property
    def find(self) -> FindSurface:
        """Return the semantic FindBar surface."""
        return self._find

    def set_locale(self, locale: Locale) -> None:
        """Refresh the FindBar locale within the central shell."""
        self._find.set_locale(locale)
