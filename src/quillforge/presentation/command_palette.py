"""Qt command palette projection over the stable command registry."""

from collections.abc import Iterable

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QDialog,
    QLabel,
    QLineEdit,
    QListWidget,
    QListWidgetItem,
    QStackedLayout,
    QVBoxLayout,
    QWidget,
)

from ..application.commands import Command
from ..domain.models import Locale
from .i18n import command_title, normalize_locale, tr

_COMMAND_ROLE = Qt.ItemDataRole.UserRole


class CommandPaletteDialog(QDialog):
    """Search current commands and return one stable command for execution."""

    def __init__(
        self,
        commands: Iterable[Command],
        parent: QWidget | None = None,
        *,
        locale: Locale = "zh-CN",
    ) -> None:
        super().__init__(parent)
        self.setObjectName("commandPalette")
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("command_palette.title", self._locale))
        self.resize(620, 420)
        self._commands = tuple(commands)
        self._selected_command_id: str | None = None
        self._query = QLineEdit(self)
        self._query.setObjectName("commandPaletteQuery")
        self._query.setPlaceholderText(tr("command_palette.placeholder", self._locale))
        self._query.textChanged.connect(self._render)
        self._query.returnPressed.connect(self._accept_current)
        self._list = QListWidget(self)
        self._list.setObjectName("commandPaletteList")
        self._list.setAlternatingRowColors(True)
        self._list.itemActivated.connect(self._accept_item)
        self._empty_message_key = "command_palette.empty.initial"
        self._empty = QLabel(self)
        self._empty.setObjectName("commandPaletteEmpty")
        self._empty.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._empty.setWordWrap(True)
        self._empty.setMinimumHeight(100)
        self._empty.setAccessibleName(tr("command_palette.results", self._locale))
        self._results_stage = QWidget(self)
        self._results_stage.setObjectName("commandPaletteResultsStage")
        results_stage_layout = QStackedLayout(self._results_stage)
        results_stage_layout.setContentsMargins(0, 0, 0, 0)
        results_stage_layout.addWidget(self._list)
        results_stage_layout.addWidget(self._empty)
        self._results_stage_layout = results_stage_layout
        hint = QLabel(tr("command_palette.hint", self._locale), self)
        hint.setObjectName("commandPaletteHint")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(18, 18, 18, 18)
        layout.setSpacing(10)
        layout.addWidget(self._query)
        layout.addWidget(self._results_stage, 1)
        layout.addWidget(hint)
        self._render("")
        self._query.setFocus()

    def selected_command_id(self) -> str | None:
        """Return only the stable ID selected when the dialog was accepted."""
        return self._selected_command_id

    def _render(self, query: str) -> None:
        needle = query.casefold().strip()
        self._list.clear()
        for command in self._commands:
            title = command_title(command.command_id, command.title, self._locale)
            searchable = f"{title} {command.title} {command.command_id}".casefold()
            if needle and needle not in searchable:
                continue
            item = QListWidgetItem(f"{title}  [{command.command_id}]")
            item.setData(_COMMAND_ROLE, command.command_id)
            self._list.addItem(item)
        if self._list.count() > 0:
            self._list.setCurrentRow(0)
        else:
            self._empty_message_key = (
                "command_palette.empty.no_matches" if needle else "command_palette.empty.initial"
            )
        self._sync_results_stage()

    def _sync_results_stage(self) -> None:
        """Keep an empty command result explicit without changing execution."""
        self._empty.setText(tr(self._empty_message_key, self._locale))
        self._results_stage_layout.setCurrentWidget(
            self._list if self._list.count() else self._empty
        )

    def _accept_current(self) -> None:
        item = self._list.currentItem()
        if item is not None:
            self._accept_item(item)

    def _accept_item(self, item: QListWidgetItem) -> None:
        command_id = item.data(_COMMAND_ROLE)
        if isinstance(command_id, str):
            self._selected_command_id = command_id
            self.accept()
