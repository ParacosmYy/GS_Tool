"""Read-only Qt projection for extension catalog diagnostics."""

from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QDialog,
    QHBoxLayout,
    QLabel,
    QListWidget,
    QListWidgetItem,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from ..application.plugin_catalog import PluginCatalogSnapshot
from ..domain.models import Locale
from ..plugins.catalog import PluginCatalogEntry
from .i18n import catalog_value, localize_message, normalize_locale, tr


class PluginCatalogDialog(QDialog):
    """Show extension metadata and trust state without exposing execution controls."""

    approve_requested = pyqtSignal(object)
    revoke_requested = pyqtSignal(object)

    def __init__(
        self,
        snapshot: PluginCatalogSnapshot,
        parent: QWidget | None = None,
        *,
        locale: Locale = "zh-CN",
    ) -> None:
        super().__init__(parent)
        self.setObjectName("pluginCatalogDialog")
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("plugin.catalog.title", self._locale))
        self.resize(720, 420)
        self._entries = tuple(snapshot.entries)
        self._governance_actions_enabled = True

        self._summary_source = snapshot.summary()
        summary = QLabel(localize_message(self._summary_source, self._locale), self)
        summary.setObjectName("dialogSummary")
        summary.setWordWrap(True)
        entries = QListWidget(self)
        entries.setObjectName("catalogEntries")
        if self._entries:
            for index, entry in enumerate(self._entries):
                item = QListWidgetItem(_format_entry(entry, self._locale))
                item.setData(Qt.ItemDataRole.UserRole, index)
                item.setToolTip(_format_tooltip(entry, self._locale))
                entries.addItem(item)
        else:
            entries.addItem(tr("plugin.catalog.empty", self._locale))
        hint = QLabel(tr("plugin.catalog.hint", self._locale), self)
        hint.setObjectName("dialogHint")
        hint.setWordWrap(True)
        self._entries_widget = entries
        self._approve_button = QPushButton(self)
        self._approve_button.setObjectName("primaryAction")
        self._revoke_button = QPushButton(self)
        self._revoke_button.setObjectName("warningAction")
        self._approve_button.clicked.connect(self._approve_current)
        self._revoke_button.clicked.connect(self._revoke_current)
        entries.currentRowChanged.connect(self._update_actions)
        if self._entries:
            entries.setCurrentRow(0)
        else:
            self._update_actions(-1)

        actions = QWidget(self)
        actions.setObjectName("dialogActionRail")
        actions_layout = QHBoxLayout(actions)
        actions_layout.setContentsMargins(0, 8, 0, 0)
        actions_layout.setSpacing(8)
        actions_layout.addWidget(self._approve_button)
        actions_layout.addWidget(self._revoke_button)
        actions_layout.addStretch(1)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(18, 18, 18, 18)
        layout.setSpacing(10)
        layout.addWidget(summary)
        layout.addWidget(entries)
        layout.addLayout(actions)
        layout.addWidget(hint)
        self._hint = hint
        self._summary = summary
        self.set_locale(self._locale)

    def set_governance_actions_enabled(self, enabled: bool) -> None:
        """Disable mutation actions while the application owns a worker task."""
        self._governance_actions_enabled = enabled
        self._update_actions(self._entries_widget.currentRow())

    def set_locale(self, locale: Locale) -> None:
        """Refresh stable catalog labels without changing entry state."""
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("plugin.catalog.title", self._locale))
        self._summary.setText(localize_message(self._summary_source, self._locale))
        self._hint.setText(tr("plugin.catalog.hint", self._locale))
        self._approve_button.setText(tr("plugin.catalog.approve", self._locale))
        self._revoke_button.setText(tr("plugin.catalog.revoke", self._locale))
        for index, entry in enumerate(self._entries):
            item = self._entries_widget.item(index)
            if item is not None:
                item.setText(_format_entry(entry, self._locale))
                item.setToolTip(_format_tooltip(entry, self._locale))
        if not self._entries:
            empty_item = self._entries_widget.item(0)
            if empty_item is not None:
                empty_item.setText(tr("plugin.catalog.empty", self._locale))

    def _current_entry(self) -> PluginCatalogEntry | None:
        index = self._entries_widget.currentRow()
        if index < 0 or index >= len(self._entries):
            return None
        return self._entries[index]

    def _update_actions(self, _row: int) -> None:
        entry = self._current_entry()
        can_approve = (
            self._governance_actions_enabled
            and entry is not None
            and entry.status == "valid"
            and entry.approval_state != "approved"
        )
        can_revoke = (
            self._governance_actions_enabled
            and entry is not None
            and entry.plugin_id is not None
            and entry.approval_state in {"approved", "stale"}
        )
        self._approve_button.setEnabled(can_approve)
        self._revoke_button.setEnabled(can_revoke)

    def _approve_current(self) -> None:
        entry = self._current_entry()
        if entry is not None:
            self.approve_requested.emit(entry)

    def _revoke_current(self) -> None:
        entry = self._current_entry()
        if entry is not None:
            self.revoke_requested.emit(entry)


def _format_entry(entry: PluginCatalogEntry, locale: Locale) -> str:
    status = catalog_value("status", entry.status, locale)
    if entry.manifest is None:
        return f"[{status}] {entry.source_path.name}"
    manifest = entry.manifest
    approval = catalog_value("approval", entry.approval_state, locale)
    execution = catalog_value("execution", entry.execution_state, locale)
    execution_label = catalog_value("field", "execution", locale)
    return (
        f"[{status} · {approval} · {execution_label}:{execution}] "
        f"{manifest.name} · {manifest.plugin_id} · {manifest.version}"
    )


def _format_tooltip(entry: PluginCatalogEntry, locale: Locale) -> str:
    reason = catalog_value("reason", entry.execution_reason, locale)
    requirements = ", ".join(
        catalog_value("reason", value, locale) for value in entry.execution_requirements
    )
    lines = [
        f"{_catalog_field('source', locale)}: {entry.source_path.name}",
        (f"{_catalog_field('status', locale)}: {catalog_value('status', entry.status, locale)}"),
        (f"{_catalog_field('trust', locale)}: {catalog_value('trust', entry.trust_state, locale)}"),
        (
            f"{_catalog_field('approval', locale)}: "
            f"{catalog_value('approval', entry.approval_state, locale)}"
        ),
        (
            f"{_catalog_field('loadable', locale)}: "
            f"{catalog_value('value', 'yes' if entry.loadable else 'no', locale)}"
        ),
        (
            f"{_catalog_field('execution', locale)}: "
            f"{catalog_value('execution', entry.execution_state, locale)}"
        ),
        f"{_catalog_field('execution_reason', locale)}: {reason}",
    ]
    if entry.execution_requirements:
        lines.append(f"{_catalog_field('execution_requirements', locale)}: {requirements}")
    if entry.manifest is not None:
        permissions = ", ".join(entry.manifest.permissions) or catalog_value(
            "value", "none", locale
        )
        lines.extend(
            [
                f"{_catalog_field('api', locale)}: {entry.manifest.api_version}",
                f"{_catalog_field('permissions', locale)}: {permissions}",
                f"{_catalog_field('entrypoint', locale)}: {entry.manifest.entrypoint}",
            ]
        )
    if entry.descriptor_sha256 is not None:
        lines.append(f"{_catalog_field('descriptor', locale)}: {entry.descriptor_sha256}")
    if entry.reason is not None:
        lines.append(
            f"{_catalog_field('reason', locale)}: {localize_message(entry.reason, locale)}"
        )
    return "\n".join(lines)


def _catalog_field(name: str, locale: Locale) -> str:
    return catalog_value("field", name, locale)
