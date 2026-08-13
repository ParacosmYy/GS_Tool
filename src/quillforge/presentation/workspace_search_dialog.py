"""Qt projection for bounded workspace search results."""

from __future__ import annotations

from pathlib import Path
from typing import Literal

from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QCloseEvent
from PyQt6.QtWidgets import (
    QCheckBox,
    QDialog,
    QDialogButtonBox,
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QListWidget,
    QListWidgetItem,
    QPushButton,
    QStackedLayout,
    QToolButton,
    QVBoxLayout,
    QWidget,
)

from ..application.workspace_search import WorkspaceSearchIssue, WorkspaceSearchResult
from ..domain.models import Locale
from .feedback import FeedbackLevel, apply_feedback_state
from .i18n import localize_message, normalize_locale, tr, workspace_search_summary

_PATH_ROLE = Qt.ItemDataRole.UserRole
_LINE_ROLE = Qt.ItemDataRole.UserRole + 1
_StatusKind = Literal["catalog", "error", "summary"]


class WorkspaceSearchDialog(QDialog):
    """Non-modal search surface that owns no filesystem or editor behavior."""

    search_requested = pyqtSignal(str, bool)
    cancel_requested = pyqtSignal()
    file_requested = pyqtSignal(object, int)

    def __init__(
        self,
        root: Path,
        parent: QWidget | None = None,
        *,
        locale: Locale = "zh-CN",
    ) -> None:
        super().__init__(parent)
        self.setObjectName("workspaceSearchDialog")
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("search.title", self._locale))
        self.setMinimumSize(760, 460)
        self.resize(920, 560)
        self._root = root
        self._diagnostic_count = 0
        self._diagnostics_truncated = False
        self._status_level: FeedbackLevel = "info"
        self._status_kind: _StatusKind = "catalog"
        self._status_key = "search.initial"
        self._status_error_message = ""
        self._status_result: WorkspaceSearchResult | None = None
        self._diagnostic_result: WorkspaceSearchResult | None = None
        self._root_label = QLabel(self)
        self._root_label.setObjectName("workspaceSearchRoot")
        self._root_label.setWordWrap(True)
        self._query = QLineEdit(self)
        self._query.setObjectName("workspaceSearchQuery")
        self._query.setClearButtonEnabled(True)
        self._case_sensitive = QCheckBox(self)
        self._search_button = QPushButton(self)
        self._search_button.setObjectName("primaryAction")
        self._cancel_button = QPushButton(self)
        self._cancel_button.hide()
        self._status = QLabel(self)
        self._status.setObjectName("workspaceSearchStatus")
        self._status.setWordWrap(True)
        self._results = QListWidget(self)
        self._results.setObjectName("workspaceSearchResults")
        self._results.setAlternatingRowColors(True)
        self._empty_message_key = "search.empty.initial"
        self._empty = QLabel(self)
        self._empty.setObjectName("workspaceSearchEmpty")
        self._empty.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._empty.setWordWrap(True)
        self._empty.setMinimumHeight(120)
        self._empty.setAccessibleName(tr("search.results", self._locale))
        self._results_stage = QWidget(self)
        self._results_stage.setObjectName("workspaceSearchResultsStage")
        results_stage_layout = QStackedLayout(self._results_stage)
        results_stage_layout.setContentsMargins(0, 0, 0, 0)
        results_stage_layout.addWidget(self._results)
        results_stage_layout.addWidget(self._empty)
        self._results_stage_layout = results_stage_layout
        self._diagnostic_toggle = QToolButton(self)
        self._diagnostic_toggle.setObjectName("workspaceSearchDiagnosticsToggle")
        self._diagnostic_toggle.setCheckable(True)
        self._diagnostic_toggle.setAutoRaise(True)
        self._diagnostic_toggle.setArrowType(Qt.ArrowType.RightArrow)
        self._diagnostic_toggle.toggled.connect(self._set_diagnostics_expanded)
        self._diagnostic_toggle.hide()
        self._diagnostics = QListWidget(self)
        self._diagnostics.setObjectName("workspaceSearchDiagnostics")
        self._diagnostics.setWordWrap(True)
        self._diagnostics.setMaximumHeight(160)
        self._diagnostics.hide()
        self._results.itemDoubleClicked.connect(self._on_item_activated)
        self._query.returnPressed.connect(self._emit_search)
        self._search_button.clicked.connect(self._emit_search)
        self._cancel_button.clicked.connect(self.cancel_requested)

        query_card = QFrame(self)
        query_card.setObjectName("workspaceSearchQueryCard")
        query_row = QHBoxLayout(query_card)
        query_row.setContentsMargins(8, 8, 8, 8)
        query_row.setSpacing(8)
        self._find_label = QLabel(self)
        query_row.addWidget(self._find_label)
        query_row.addWidget(self._query, 1)
        query_row.addWidget(self._case_sensitive)
        query_row.addWidget(self._search_button)
        query_row.addWidget(self._cancel_button)
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Close, parent=self)
        buttons.setObjectName("dialogActions")
        close_button = buttons.button(QDialogButtonBox.StandardButton.Close)
        if close_button is not None:
            close_button.setObjectName("quietAction")
        buttons.rejected.connect(self.close)
        self._buttons = buttons

        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)
        layout.addWidget(self._root_label)
        layout.addWidget(query_card)
        layout.addWidget(self._results_stage, 1)
        layout.addWidget(self._diagnostic_toggle)
        layout.addWidget(self._diagnostics)
        layout.addWidget(self._status)
        layout.addWidget(buttons)
        self.set_locale(self._locale)
        self.set_root(root)

    @property
    def root(self) -> Path:
        """Return the root currently shown to the user."""
        return self._root

    def set_root(self, root: Path) -> None:
        """Update the explicit search scope and clear results from the old root."""
        self._root = root.expanduser().resolve()
        self._diagnostic_result = None
        self._root_label.setText(tr("search.root", self._locale, root=self._root))
        self._results.clear()
        self._empty_message_key = "search.empty.initial"
        self._sync_results_stage()
        self._clear_diagnostics()
        self._set_catalog_status("search.initial", "info")

    def set_busy(self, busy: bool) -> None:
        """Keep one dialog request stable while its worker is scanning."""
        for control in (self._query, self._case_sensitive, self._search_button):
            control.setEnabled(not busy)
        self._diagnostic_toggle.setEnabled(not busy)
        if busy:
            self._set_diagnostics_expanded(False)
        self._cancel_button.setVisible(busy)
        self._cancel_button.setEnabled(busy)
        if busy and self._results.count() == 0:
            self._empty_message_key = "search.empty.loading"
            self._sync_results_stage()
        if busy:
            self._set_catalog_status("search.loading", "working")

    def set_cancel_requested(self) -> None:
        """Give immediate feedback while the worker observes the cancellation token."""
        self._cancel_button.setEnabled(False)
        self._set_catalog_status("search.cancelling", "warning")

    def present_result(self, result: WorkspaceSearchResult) -> None:
        """Render only immutable application data from the current search."""
        self._results.clear()
        for match in result.matches:
            try:
                relative_path = match.path.relative_to(self._root)
            except ValueError:
                relative_path = match.path
            item = QListWidgetItem(
                f"{relative_path}:{match.line}:{match.column}    {match.preview}",
                self._results,
            )
            item.setData(_PATH_ROLE, str(match.path))
            item.setData(_LINE_ROLE, match.line)
            item.setToolTip(str(match.path))
        if not result.matches:
            self._empty_message_key = (
                "search.empty.cancelled" if result.cancelled else "search.empty.no_matches"
            )
        self._sync_results_stage()
        self._diagnostic_count = len(result.issues)
        self._diagnostics_truncated = result.issues_truncated
        self._diagnostic_result = result
        self._present_diagnostics(result)
        self._set_summary_status(
            result,
            (
                "warning"
                if result.cancelled or result.truncated or result.issues or result.issues_truncated
                else "success"
            ),
        )
        self.set_busy(False)

    def present_error(self, message: str) -> None:
        """Show a recoverable operation error without discarding the root."""
        self.set_busy(False)
        if self._results.count() == 0:
            self._empty_message_key = "search.empty.error"
            self._sync_results_stage()
        self._set_error_status(message)

    def present_cancelled(self) -> None:
        """Clear the busy state after a stale or explicitly invalidated request."""
        self.set_busy(False)
        if self._results.count() == 0:
            self._empty_message_key = "search.empty.cancelled"
            self._sync_results_stage()
        self._set_catalog_status("search.cancelled", "warning")

    def set_locale(self, locale: Locale) -> None:
        """Refresh the search surface while retaining query and results."""
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("search.title", self._locale))
        self._empty.setAccessibleName(tr("search.results", self._locale))
        self._find_label.setText(tr("find.find", self._locale))
        self._query.setPlaceholderText(tr("search.placeholder", self._locale))
        self._case_sensitive.setText(tr("search.case", self._locale))
        self._search_button.setText(tr("search.search", self._locale))
        self._cancel_button.setText(tr("search.cancel", self._locale))
        self._buttons.button(QDialogButtonBox.StandardButton.Close).setText(
            tr("search.close", self._locale)
        )
        self._root_label.setText(tr("search.root", self._locale, root=self._root))
        if self._diagnostics_truncated:
            self._diagnostic_toggle.setText(
                tr("search.diagnostics_truncated", self._locale, count=self._diagnostic_count)
            )
        elif self._diagnostic_count:
            self._diagnostic_toggle.setText(
                tr("search.diagnostics_count", self._locale, count=self._diagnostic_count)
            )
        else:
            self._diagnostic_toggle.setText(tr("search.diagnostics", self._locale))
        self._sync_results_stage()
        self._refresh_diagnostic_items()
        self._status.setText(self._localized_status_text())
        apply_feedback_state(self._status, self._status_level)

    def _emit_search(self) -> None:
        query = self._query.text()
        if not query:
            self._set_catalog_status("search.enter", "warning")
            return
        if self._results.count() == 0:
            self._empty_message_key = "search.empty.loading"
            self._sync_results_stage()
        self.search_requested.emit(query, self._case_sensitive.isChecked())

    def _sync_results_stage(self) -> None:
        """Keep the result viewport explicit when there are no rows to show."""
        self._empty.setText(tr(self._empty_message_key, self._locale))
        level: FeedbackLevel = "info"
        if self._empty_message_key == "search.empty.loading":
            level = "working"
        elif self._empty_message_key == "search.empty.cancelled":
            level = "warning"
        elif self._empty_message_key == "search.empty.error":
            level = "error"
        apply_feedback_state(self._empty, level)
        self._results_stage_layout.setCurrentWidget(
            self._results if self._results.count() else self._empty
        )

    def _set_catalog_status(self, key: str, level: FeedbackLevel) -> None:
        self._status_kind = "catalog"
        self._status_key = key
        self._status_error_message = ""
        self._status_result = None
        self._set_status(tr(key, self._locale), level)

    def _set_error_status(self, message: str) -> None:
        self._status_kind = "error"
        self._status_key = "search.failed"
        self._status_error_message = message
        self._status_result = None
        self._set_status(self._localized_status_text(), "error")

    def _set_summary_status(
        self,
        result: WorkspaceSearchResult,
        level: FeedbackLevel,
    ) -> None:
        self._status_kind = "summary"
        self._status_key = ""
        self._status_error_message = ""
        self._status_result = result
        self._set_status(self._localized_status_text(), level)

    def _localized_status_text(self) -> str:
        """Rebuild the status from its source instead of caching translated text."""
        if self._status_kind == "error":
            return tr(
                "search.failed",
                self._locale,
                message=localize_message(self._status_error_message, self._locale),
            )
        if self._status_kind == "summary" and self._status_result is not None:
            result = self._status_result
            return workspace_search_summary(
                cancelled=result.cancelled,
                truncated=result.truncated,
                limit_reason=result.limit_reason,
                matches=len(result.matches),
                files_scanned=result.files_scanned,
                bytes_scanned=result.bytes_scanned,
                diagnostic_count=len(result.issues),
                diagnostics_truncated=result.issues_truncated,
                locale=self._locale,
            )
        return tr(self._status_key, self._locale)

    def _set_status(self, text: str, level: FeedbackLevel) -> None:
        self._status_level = level
        self._status.setText(text)
        apply_feedback_state(self._status, level)

    def _on_item_activated(self, item: QListWidgetItem) -> None:
        path_value = item.data(_PATH_ROLE)
        line_value = item.data(_LINE_ROLE)
        if not isinstance(path_value, str) or type(line_value) is not int:
            return
        self.file_requested.emit(Path(path_value), line_value)

    def _present_diagnostics(self, result: WorkspaceSearchResult) -> None:
        """Project bounded issue records without exposing absolute local paths."""
        self._diagnostics.clear()
        for issue in result.issues:
            text = self._diagnostic_text(issue)
            item = QListWidgetItem(text, self._diagnostics)
            item.setToolTip(text)
        if result.issues_truncated and not result.issues:
            QListWidgetItem(
                tr("search.more_diagnostics", self._locale),
                self._diagnostics,
            )
        has_diagnostics = bool(result.issues) or result.issues_truncated
        if result.issues_truncated:
            self._diagnostic_toggle.setText(
                tr(
                    "search.diagnostics_truncated",
                    self._locale,
                    count=len(result.issues),
                )
            )
        else:
            self._diagnostic_toggle.setText(
                tr("search.diagnostics_count", self._locale, count=len(result.issues))
            )
        self._diagnostic_toggle.setVisible(has_diagnostics)
        self._diagnostic_toggle.setEnabled(True)
        self._diagnostic_toggle.setChecked(False)
        self._set_diagnostics_expanded(False)

    def _clear_diagnostics(self) -> None:
        self._diagnostic_result = None
        self._diagnostics.clear()
        self._diagnostic_count = 0
        self._diagnostics_truncated = False
        self._diagnostic_toggle.setChecked(False)
        self._diagnostic_toggle.setText(tr("search.diagnostics", self._locale))
        self._diagnostic_toggle.hide()
        self._set_diagnostics_expanded(False)

    def _refresh_diagnostic_items(self) -> None:
        """Reproject existing diagnostic rows without changing expansion state."""
        result = self._diagnostic_result
        if result is None:
            return
        for index, issue in enumerate(result.issues):
            item = self._diagnostics.item(index)
            if item is None:
                continue
            text = self._diagnostic_text(issue)
            item.setText(text)
            item.setToolTip(text)
        if result.issues_truncated and not result.issues:
            marker = self._diagnostics.item(0)
            if marker is not None:
                marker.setText(tr("search.more_diagnostics", self._locale))

    def _diagnostic_text(self, issue: WorkspaceSearchIssue) -> str:
        """Format one bounded issue with a locale-aware reason projection."""
        return f"{self._relative_path(issue.path)} — {localize_message(issue.reason, self._locale)}"

    def _set_diagnostics_expanded(self, expanded: bool) -> None:
        """Toggle the bounded read-only diagnostic list."""
        self._diagnostic_toggle.setArrowType(
            Qt.ArrowType.DownArrow if expanded else Qt.ArrowType.RightArrow
        )
        self._diagnostics.setVisible(expanded and self._diagnostic_toggle.isVisible())

    def _relative_path(self, path: Path) -> str:
        try:
            return str(path.relative_to(self._root))
        except ValueError:
            return tr("search.outside_workspace", self._locale)

    def closeEvent(self, event: QCloseEvent) -> None:  # noqa: N802 - Qt override name
        """Treat closing the surface as a cooperative cancellation request."""
        self.cancel_requested.emit()
        super().closeEvent(event)
