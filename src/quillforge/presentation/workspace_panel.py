"""Qt projection for bounded workspace navigation."""

from pathlib import Path
from typing import Literal

from PyQt6.QtCore import QSize, Qt, pyqtSignal
from PyQt6.QtGui import QIcon, QKeyEvent, QPalette
from PyQt6.QtWidgets import (
    QAbstractItemView,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QTreeWidget,
    QTreeWidgetItem,
    QVBoxLayout,
    QWidget,
)

from ..domain.models import Locale, WorkspaceDirectory, WorkspaceEntry
from .feedback import FeedbackLevel, apply_feedback_state
from .i18n import localize_exception, localize_message, normalize_locale, tr
from .icons import IconKey, themed_icon

_PATH_ROLE = Qt.ItemDataRole.UserRole
_KIND_ROLE = Qt.ItemDataRole.UserRole + 1
_ERROR_ROLE = Qt.ItemDataRole.UserRole + 2


class _WorkspaceTree(QTreeWidget):
    """Keep keyboard activation separate from Qt's style-dependent itemActivated signal."""

    keyboard_activation_requested = pyqtSignal(object)

    def keyPressEvent(self, event: QKeyEvent) -> None:  # noqa: N802 - Qt override name
        if event.key() in (Qt.Key.Key_Return, Qt.Key.Key_Enter):
            item = self.currentItem()
            if item is not None:
                self.keyboard_activation_requested.emit(item)
                event.accept()
                return
        super().keyPressEvent(event)


class WorkspacePanel(QWidget):
    """Display one bounded directory page and emit semantic navigation intents."""

    folder_requested = pyqtSignal()
    file_picker_requested = pyqtSignal()
    directory_requested = pyqtSignal(object)
    file_requested = pyqtSignal(object)
    back_requested = pyqtSignal()
    cancel_requested = pyqtSignal()

    def __init__(self, parent: QWidget | None = None, *, locale: Locale = "zh-CN") -> None:
        super().__init__(parent)
        self.setObjectName("workspacePanel")
        self._locale = normalize_locale(locale)
        self._root: Path | None = None
        self._current_path: Path | None = None
        self._directory_count = 0
        self._directory_truncated = False
        self._loading = False
        self._status_level: FeedbackLevel = "info"
        self._workspace_error: str | Exception | None = None
        self._tree = _WorkspaceTree(self)
        self._tree.setObjectName("workspaceTree")
        self._tree.setHeaderLabels(["Workspace"])
        self._tree.setHeaderHidden(True)
        self._tree.setIconSize(QSize(18, 18))
        self._tree.setIndentation(16)
        self._tree.setRootIsDecorated(False)
        self._tree.setAnimated(False)
        self._tree.setAlternatingRowColors(True)
        self._tree.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        self._tree.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self._tree.setUniformRowHeights(True)
        self._tree.setTextElideMode(Qt.TextElideMode.ElideMiddle)
        self._tree.itemClicked.connect(self._on_item_clicked)
        self._tree.itemDoubleClicked.connect(self._on_item_double_clicked)
        self._tree.keyboard_activation_requested.connect(self._on_item_activated)
        self._path_label = QLabel(self)
        self._path_label.setObjectName("workspacePath")
        self._path_label.setWordWrap(True)
        self._empty_label = QLabel(self)
        self._empty_label.setObjectName("workspaceEmpty")
        self._empty_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._empty_label.setWordWrap(True)
        self._empty_label.setMinimumHeight(72)
        self._empty_label.setVisible(True)
        self._tree.setVisible(False)
        self._status_label = QLabel(self)
        self._status_label.setObjectName("workspaceStatus")
        self._status_label.setWordWrap(True)
        self._open_button = QPushButton(self)
        self._open_button.setObjectName("primaryAction")
        self._open_button.setProperty("workspaceRole", "folderPicker")
        self._open_button.clicked.connect(self.folder_requested)
        self._file_button = QPushButton(self)
        self._file_button.setObjectName("workspaceFileAction")
        self._file_button.setProperty("workspaceRole", "documentPicker")
        self._file_button.clicked.connect(self.file_picker_requested)
        self._back_button = QPushButton(self)
        self._back_button.setObjectName("workspaceBack")
        self._back_button.setProperty("workspaceRole", "parentNavigation")
        self._back_button.setEnabled(False)
        self._back_button.clicked.connect(self.back_requested)
        self._cancel_button = QPushButton(self)
        self._cancel_button.setObjectName("workspaceCancel")
        self._cancel_button.setProperty("workspaceRole", "cancelOperation")
        self._cancel_button.setVisible(False)
        self._cancel_button.clicked.connect(self.cancel_requested)

        self._eyebrow = QLabel(self)
        self._eyebrow.setObjectName("workspaceEyebrow")
        toolbar = QHBoxLayout()
        toolbar.setSpacing(6)
        toolbar.addWidget(self._open_button)
        toolbar.addWidget(self._file_button)
        toolbar.addWidget(self._back_button)
        toolbar.addWidget(self._cancel_button)
        toolbar.addStretch(1)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(10)
        layout.addWidget(self._eyebrow)
        layout.addLayout(toolbar)
        layout.addWidget(self._path_label)
        layout.addWidget(self._empty_label)
        layout.addWidget(self._tree)
        layout.addWidget(self._status_label)
        self.set_locale(self._locale)
        self.refresh_icons()

    def set_directory(self, directory: WorkspaceDirectory, root: Path) -> None:
        """Replace the visible page only after the application result is current."""
        self._workspace_error = None
        self._root = root
        self._current_path = directory.path
        self._directory_count = len(directory.entries)
        self._directory_truncated = directory.truncated
        self._loading = False
        self._path_label.setText(str(directory.path))
        self._tree.clear()
        for entry in directory.entries:
            self._tree.addTopLevelItem(self._item_for(entry))
        if directory.truncated:
            truncated = QTreeWidgetItem([tr("workspace.more_entries", self._locale)])
            truncated.setDisabled(True)
            self._tree.addTopLevelItem(truncated)
        self._back_button.setEnabled(directory.path != root)
        self._sync_content_state()
        self._set_status_text(
            self._entries_status(),
            "warning" if directory.truncated else "success",
        )

    @property
    def current_path(self) -> Path | None:
        """Return the page currently projected by the panel."""
        return self._current_path

    def set_loading(self, loading: bool) -> None:
        """Disable navigation controls while the bounded provider call is in flight."""
        self._loading = loading
        if loading:
            self._workspace_error = None
        self._open_button.setEnabled(not loading)
        self._file_button.setEnabled(not loading)
        self._tree.setEnabled(not loading)
        self._cancel_button.setVisible(loading)
        self._cancel_button.setEnabled(loading)
        self._sync_content_state()
        if loading:
            self._set_status_text(tr("workspace.loading", self._locale), "working")

    def show_error(self, message: str | Exception) -> None:
        """Render a recoverable workspace error without clearing the last good page."""
        self._workspace_error = message
        self._set_status_text(self._localized_workspace_error(), "error")

    def set_locale(self, locale: Locale) -> None:
        """Refresh stable labels and retain the current workspace page."""
        self._locale = normalize_locale(locale)
        self._eyebrow.setText(tr("workspace.eyebrow", self._locale))
        self._tree.setHeaderLabel(tr("workspace.tree", self._locale))
        self._tree.setAccessibleName(tr("workspace.tree", self._locale))
        self._path_label.setAccessibleName(tr("workspace.path", self._locale))
        self._empty_label.setAccessibleName(tr("workspace.empty", self._locale))
        self._path_label.setText(
            str(self._current_path)
            if self._current_path is not None
            else tr("workspace.no_selection", self._locale)
        )
        self._open_button.setText(tr("workspace.open_folder", self._locale))
        self._file_button.setText(tr("workspace.open_file", self._locale))
        self._back_button.setText(tr("workspace.up", self._locale))
        self._cancel_button.setText(tr("workspace.cancel", self._locale))
        self._set_action_accessibility(
            self._open_button,
            "workspace.open_folder",
            "workspace.open_folder_hint",
        )
        self._set_action_accessibility(
            self._file_button,
            "workspace.open_file",
            "workspace.open_file_hint",
        )
        self._set_action_accessibility(
            self._back_button,
            "workspace.up",
            "workspace.up_hint",
        )
        self._set_action_accessibility(
            self._cancel_button,
            "workspace.cancel",
            "workspace.cancel_hint",
        )
        if self._directory_truncated and self._tree.topLevelItemCount():
            marker = self._tree.topLevelItem(self._tree.topLevelItemCount() - 1)
            if marker is not None:
                marker.setText(0, tr("workspace.more_entries", self._locale))
        for index in range(self._tree.topLevelItemCount()):
            item = self._tree.topLevelItem(index)
            if item is None:
                continue
            error = item.data(0, _ERROR_ROLE)
            if isinstance(error, str) and error:
                item.setToolTip(0, localize_message(error, self._locale))
                continue
            if item.isDisabled():
                continue
            hint = self._entry_hint(item.data(0, _KIND_ROLE))
            if hint:
                item.setToolTip(0, hint)
        self._sync_content_state()
        if self._workspace_error is not None:
            self._set_status_text(self._localized_workspace_error(), "error")
        elif self._loading:
            self._set_status_text(tr("workspace.loading", self._locale), "working")
        elif self._current_path is None:
            self._set_status_text(tr("workspace.choose", self._locale), "info")
        else:
            self._set_status_text(
                self._entries_status(),
                "warning" if self._directory_truncated else "success",
            )

    def _set_action_accessibility(
        self,
        button: QPushButton,
        label_key: str,
        hint_key: str,
    ) -> None:
        """Keep action labels, tooltips, and screen-reader descriptions aligned."""
        label = tr(label_key, self._locale)
        hint = tr(hint_key, self._locale)
        button.setAccessibleName(label)
        button.setAccessibleDescription(hint)
        button.setToolTip(hint)

    def _set_status_text(self, text: str, level: FeedbackLevel) -> None:
        self._status_level = level
        self._status_label.setText(text)
        apply_feedback_state(self._status_label, level)

    def _localized_workspace_error(self) -> str:
        """Project the retained error source in the currently selected locale."""
        message = self._workspace_error
        if message is None:
            return ""
        if isinstance(message, Exception):
            return localize_exception(message, self._locale)
        return localize_message(message, self._locale)

    def _sync_content_state(self) -> None:
        """Keep the tree and its localized empty state mutually exclusive."""
        if self._loading and self._current_path is None:
            self._empty_label.setText(tr("workspace.loading", self._locale))
            self._empty_label.setVisible(True)
            self._tree.setVisible(False)
            return
        if self._current_path is None:
            self._empty_label.setText(tr("workspace.empty", self._locale))
            self._empty_label.setVisible(True)
            self._tree.setVisible(False)
            return
        has_rows = self._tree.topLevelItemCount() > 0
        self._empty_label.setText(tr("workspace.empty_directory", self._locale))
        self._empty_label.setVisible(not has_rows)
        self._tree.setVisible(has_rows)

    def refresh_icons(self) -> None:
        """Retint authored controls and visible entries after a theme change."""
        palette = self.palette()
        foreground = palette.color(QPalette.ColorRole.Text).name()
        accent = palette.color(QPalette.ColorRole.Link).name()
        disabled_foreground = palette.color(
            QPalette.ColorGroup.Disabled,
            QPalette.ColorRole.Text,
        ).name()
        self._open_button.setIcon(
            themed_icon(
                IconKey.FOLDER_OPEN,
                foreground=palette.color(QPalette.ColorRole.BrightText).name(),
                accent=accent,
                disabled_foreground=disabled_foreground,
                disabled_accent=disabled_foreground,
            )
        )
        self._file_button.setIcon(
            themed_icon(
                IconKey.DOCUMENT,
                foreground=foreground,
                accent=accent,
                disabled_foreground=disabled_foreground,
                disabled_accent=disabled_foreground,
            )
        )
        self._back_button.setIcon(
            themed_icon(
                IconKey.ARROW_UP,
                foreground=foreground,
                accent=accent,
                disabled_foreground=disabled_foreground,
                disabled_accent=disabled_foreground,
            )
        )
        self._cancel_button.setIcon(
            themed_icon(
                IconKey.CLOSE,
                foreground=foreground,
                accent=accent,
                disabled_foreground=disabled_foreground,
                disabled_accent=disabled_foreground,
            )
        )
        for index in range(self._tree.topLevelItemCount()):
            item = self._tree.topLevelItem(index)
            if item is None:
                continue
            kind = item.data(0, _KIND_ROLE)
            if self._icon_key_for_kind(kind) is not None:
                item.setIcon(0, self._entry_icon(kind))

    def _entries_status(self) -> str:
        suffix = tr("workspace.truncated_suffix", self._locale) if self._directory_truncated else ""
        return tr(
            "workspace.entries",
            self._locale,
            count=self._directory_count,
            suffix=suffix,
        )

    def _item_for(self, entry: WorkspaceEntry) -> QTreeWidgetItem:
        item = QTreeWidgetItem([entry.name])
        item.setIcon(0, self._entry_icon(entry.kind))
        item.setData(0, _PATH_ROLE, str(entry.path))
        item.setData(0, _KIND_ROLE, entry.kind)
        item.setData(0, _ERROR_ROLE, entry.error)
        if entry.error:
            item.setToolTip(0, localize_message(entry.error, self._locale))
            item.setDisabled(True)
        else:
            item.setToolTip(0, self._entry_hint(entry.kind))
        return item

    def _entry_icon(self, kind: object) -> QIcon:
        key = self._icon_key_for_kind(kind) or IconKey.DOCUMENT
        palette = self.palette()
        foreground = palette.color(QPalette.ColorRole.Text).name()
        accent = palette.color(QPalette.ColorRole.Link).name()
        if kind == "file" and palette.color(QPalette.ColorRole.Base).lightnessF() < 0.65:
            # Files use the link role for an immediate cue on dark canvases;
            # light canvases keep the primary text outline so pressed rows stay
            # readable, while folders retain the link-colored filled detail.
            foreground = accent
        return themed_icon(
            key,
            foreground=foreground,
            accent=accent,
            disabled_foreground=palette.color(
                QPalette.ColorGroup.Disabled,
                QPalette.ColorRole.Text,
            ).name(),
            disabled_accent=palette.color(
                QPalette.ColorGroup.Disabled,
                QPalette.ColorRole.Text,
            ).name(),
        )

    def _entry_hint(self, kind: object) -> str:
        key = self._entry_hint_key(kind)
        return tr(key, self._locale) if key is not None else ""

    @staticmethod
    def _entry_hint_key(kind: object) -> str | None:
        if kind == "file":
            return "workspace.entry_file_hint"
        if kind == "directory":
            return "workspace.entry_directory_hint"
        if kind == "inaccessible":
            return "workspace.entry_inaccessible_hint"
        return None

    @staticmethod
    def _icon_key_for_kind(kind: object) -> IconKey | None:
        if kind == "directory":
            return IconKey.FOLDER
        if kind == "inaccessible":
            return IconKey.WARNING
        if kind == "file":
            return IconKey.DOCUMENT
        return None

    def _on_item_clicked(self, item: QTreeWidgetItem, _column: int) -> None:
        """Open a file on the first click while leaving folders navigable."""
        self._emit_item_intent(item, expected_kind="file")

    def _on_item_double_clicked(self, item: QTreeWidgetItem, _column: int) -> None:
        """Activate either entry kind on double-click for an unsurprising tree UX."""
        self._emit_item_intent(item)

    def _on_item_activated(self, item: QTreeWidgetItem) -> None:
        """Route Enter/Return activation to the entry's semantic workspace intent."""
        self._emit_item_intent(item)

    def _emit_item_intent(
        self,
        item: QTreeWidgetItem | None,
        *,
        expected_kind: Literal["file", "directory"] | None = None,
    ) -> None:
        """Emit only the semantic callback that matches the visible entry kind."""
        path, kind = self._item_path_and_kind(item)
        if path is None or (expected_kind is not None and kind != expected_kind):
            return
        if kind == "file":
            self.file_requested.emit(path)
        elif kind == "directory":
            self.directory_requested.emit(path)

    def _item_path_and_kind(self, item: QTreeWidgetItem | None) -> tuple[Path | None, object]:
        if item is None:
            return None, None
        path_value = item.data(0, _PATH_ROLE)
        kind = item.data(0, _KIND_ROLE)
        if not isinstance(path_value, str):
            return None, kind
        return Path(path_value), kind
