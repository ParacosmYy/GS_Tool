"""Qt projection for explicitly registered plugin lifecycle status."""

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

from ..application.plugin_runtime import PluginRuntimeStatus
from ..domain.models import Locale
from .i18n import localize_message, normalize_locale, plugin_display_name, tr


class PluginStatusDialog(QDialog):
    """Show runtime state and emit explicit lifecycle intents."""

    enable_requested = pyqtSignal(str)
    disable_requested = pyqtSignal(str)

    def __init__(
        self,
        statuses: tuple[PluginRuntimeStatus, ...],
        parent: QWidget | None = None,
        *,
        locale: Locale = "zh-CN",
    ) -> None:
        super().__init__(parent)
        self.setObjectName("pluginStatusDialog")
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("plugin.status.title", self._locale))
        self.resize(720, 420)
        self._statuses = statuses
        self._status_list = QListWidget(self)
        self._status_list.setObjectName("pluginStatusEntries")
        for index, status in enumerate(statuses):
            item = QListWidgetItem(_format_status(status, self._locale))
            item.setData(Qt.ItemDataRole.UserRole, index)
            item.setToolTip(_format_tooltip(status, self._locale))
            self._status_list.addItem(item)
        if not statuses:
            self._status_list.addItem(tr("plugin.status.empty", self._locale))

        self._enable_button = QPushButton(self)
        self._enable_button.setObjectName("primaryAction")
        self._disable_button = QPushButton(self)
        self._disable_button.setObjectName("warningAction")
        self._enable_button.clicked.connect(self._enable_current)
        self._disable_button.clicked.connect(self._disable_current)
        self._status_list.currentRowChanged.connect(self._update_actions)
        if statuses:
            self._status_list.setCurrentRow(0)
        else:
            self._update_actions(-1)

        summary = QLabel(
            tr("plugin.status.summary", self._locale),
            self,
        )
        summary.setObjectName("dialogSummary")
        summary.setWordWrap(True)
        actions = QWidget(self)
        actions.setObjectName("dialogActionRail")
        actions_layout = QHBoxLayout(actions)
        actions_layout.setContentsMargins(0, 8, 0, 0)
        actions_layout.setSpacing(8)
        actions_layout.addWidget(self._enable_button)
        actions_layout.addWidget(self._disable_button)
        actions_layout.addStretch(1)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(18, 18, 18, 18)
        layout.setSpacing(10)
        layout.addWidget(summary)
        layout.addWidget(self._status_list)
        layout.addLayout(actions)
        self._summary = summary
        self.set_locale(self._locale)

    def _current_status(self) -> PluginRuntimeStatus | None:
        index = self._status_list.currentRow()
        if index < 0 or index >= len(self._statuses):
            return None
        return self._statuses[index]

    def set_locale(self, locale: Locale) -> None:
        """Refresh static controls and re-render bounded status rows."""
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("plugin.status.title", self._locale))
        self._summary.setText(tr("plugin.status.summary", self._locale))
        self._enable_button.setText(tr("plugin.enable", self._locale))
        self._disable_button.setText(tr("plugin.disable", self._locale))
        for index, status in enumerate(self._statuses):
            item = self._status_list.item(index)
            if item is not None:
                item.setText(_format_status(status, self._locale))
                item.setToolTip(_format_tooltip(status, self._locale))

    def _update_actions(self, _row: int) -> None:
        status = self._current_status()
        can_toggle = status is not None and status.trusted
        self._enable_button.setEnabled(can_toggle and not status.enabled)
        self._disable_button.setEnabled(can_toggle and status.enabled)

    def _enable_current(self) -> None:
        status = self._current_status()
        if status is not None:
            self.enable_requested.emit(status.plugin_id)

    def _disable_current(self) -> None:
        status = self._current_status()
        if status is not None:
            self.disable_requested.emit(status.plugin_id)


def _format_status(status: PluginRuntimeStatus, locale: Locale = "zh-CN") -> str:
    trust = tr("plugin.trusted" if status.trusted else "plugin.untrusted", locale)
    enabled = tr("plugin.enabled" if status.enabled else "plugin.disabled", locale)
    active = tr("plugin.active" if status.active else "plugin.inactive", locale)
    name = plugin_display_name(status.plugin_id, status.name, locale)
    return f"[{active} · {enabled} · {trust}] {name} · {status.plugin_id} · {status.version}"


def _format_tooltip(status: PluginRuntimeStatus, locale: Locale = "zh-CN") -> str:
    trust_label = tr(
        "plugin.trusted" if status.trusted else "plugin.untrusted",
        locale,
    )
    permissions = ", ".join(status.permissions) or tr("plugin.none", locale)
    enabled_value = tr("plugin.value.true" if status.enabled else "plugin.value.false", locale)
    active_value = tr("plugin.value.true" if status.active else "plugin.value.false", locale)
    lines = [
        f"{tr('plugin.id', locale)}: {status.plugin_id}",
        f"{tr('plugin.version', locale)}: {status.version}",
        f"{tr('plugin.trust', locale)}: {trust_label}",
        f"{tr('plugin.enabled', locale)}: {enabled_value}",
        f"{tr('plugin.active', locale)}: {active_value}",
        f"{tr('plugin.permissions', locale)}: {permissions}",
    ]
    if status.error is not None:
        lines.append(f"{tr('plugin.error', locale)}: {localize_message(status.error, locale)}")
    return "\n".join(lines)
