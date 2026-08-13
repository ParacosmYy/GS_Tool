"""Presentation projection and lookup for the document tab rail."""

from __future__ import annotations

from collections.abc import Callable
from pathlib import Path
from typing import Protocol, TypeVar

from PyQt6.QtCore import QSize, Qt
from PyQt6.QtGui import QPalette
from PyQt6.QtWidgets import QMainWindow, QTabWidget

from ..domain.models import DocumentState
from ..domain.path_identity import path_key
from .editor_widget import EditorWidget
from .icons import IconKey, themed_icon


class DocumentTabLike(Protocol):
    """Minimum tab record contract required by the presentation surface."""

    editor: EditorWidget
    state: DocumentState


TabT = TypeVar("TabT", bound=DocumentTabLike)


class DocumentTabSurface[TabT]:
    """Own the tab rail projection while leaving document behavior to the shell."""

    def __init__(
        self,
        parent: QMainWindow,
        *,
        close_requested: Callable[[int], None],
        current_changed: Callable[[int], None],
    ) -> None:
        self._widget = QTabWidget(parent)
        self._widget.setObjectName("documentTabs")
        self._widget.setDocumentMode(True)
        self._widget.setTabsClosable(True)
        self._widget.setIconSize(QSize(18, 18))
        tab_bar = self._widget.tabBar()
        tab_bar.setObjectName("documentTabBar")
        tab_bar.setElideMode(Qt.TextElideMode.ElideMiddle)
        tab_bar.setExpanding(False)
        self._widget.tabCloseRequested.connect(close_requested)
        self._widget.currentChanged.connect(current_changed)
        self._tabs: list[TabT] = []
        self._modified: list[bool] = []

    @property
    def widget(self) -> QTabWidget:
        """Return the Qt projection for composition into the desktop shell."""
        return self._widget

    @property
    def tabs(self) -> tuple[TabT, ...]:
        """Return a stable snapshot of the records projected into the tab rail."""
        return tuple(self._tabs)

    @property
    def count(self) -> int:
        return len(self._tabs)

    @property
    def current_index(self) -> int:
        return self._widget.currentIndex()

    def add_tab(self, tab: TabT, title: str, *, modified: bool = False) -> None:
        """Project one record and make it the current document."""
        if self._index_of(tab) is not None:
            return
        self._tabs.append(tab)
        self._modified.append(modified)
        index = self._widget.addTab(tab.editor, title)
        self._set_tab_icon(index, modified)
        self._widget.setCurrentIndex(index)

    def remove_tab(self, tab: TabT) -> bool:
        """Remove one record from the projection, preserving identity semantics."""
        index = self._index_of(tab)
        if index is None:
            return False
        self._tabs.pop(index)
        self._modified.pop(index)
        self._widget.removeTab(index)
        return True

    def active_tab(self) -> TabT | None:
        """Return the record at the current Qt tab index, if any."""
        index = self.current_index
        if index < 0 or index >= len(self._tabs):
            return None
        return self._tabs[index]

    def find_by_editor(self, editor: EditorWidget) -> TabT | None:
        """Resolve a record from an editor callback without exposing Qt indexes."""
        return next((tab for tab in self._tabs if tab.editor is editor), None)

    def find_by_path(self, path: Path | None, *, exclude: TabT | None = None) -> TabT | None:
        """Resolve one tab by normalized path identity within the full registry."""
        if path is None:
            return None
        identity = path_key(path)
        return next(
            (
                tab
                for tab in self._tabs
                if tab is not exclude
                and tab.state.path is not None
                and path_key(tab.state.path) == identity
            ),
            None,
        )

    def contains(self, tab: TabT) -> bool:
        return self._index_of(tab) is not None

    def index_of(self, tab: TabT) -> int | None:
        return self._index_of(tab)

    def set_current(self, tab: TabT) -> bool:
        """Select a projected record and report whether it belonged to the surface."""
        if not self.contains(tab):
            return False
        self._widget.setCurrentWidget(tab.editor)
        return True

    def set_title(self, tab: TabT, title: str) -> bool:
        """Synchronize one record's human-facing title with the Qt tab rail."""
        index = self._index_of(tab)
        if index is None:
            return False
        self._widget.setTabText(index, title)
        return True

    def set_modified(self, tab: TabT, modified: bool) -> bool:
        """Project the unsaved marker without owning document dirty policy."""
        index = self._index_of(tab)
        if index is None:
            return False
        self._modified[index] = modified
        self._set_tab_icon(index, modified)
        return True

    def refresh_icons(self) -> None:
        """Retint clean/modified tab markers after the application theme changes."""
        for index, modified in enumerate(self._modified):
            self._set_tab_icon(index, modified)

    def set_tab_bar_enabled(self, enabled: bool) -> None:
        self._widget.tabBar().setEnabled(enabled)

    def _set_tab_icon(self, index: int, modified: bool) -> None:
        palette = self._widget.palette()
        foreground = palette.color(QPalette.ColorRole.Text).name()
        accent = palette.color(QPalette.ColorRole.Link).name()
        disabled_foreground = palette.color(
            QPalette.ColorGroup.Disabled,
            QPalette.ColorRole.Text,
        ).name()
        self._widget.setTabIcon(
            index,
            themed_icon(
                IconKey.MODIFIED if modified else IconKey.DOCUMENT,
                foreground=foreground,
                accent=accent,
                disabled_foreground=disabled_foreground,
                disabled_accent=disabled_foreground,
            ),
        )

    def _index_of(self, target: TabT) -> int | None:
        return next((index for index, tab in enumerate(self._tabs) if tab is target), None)
