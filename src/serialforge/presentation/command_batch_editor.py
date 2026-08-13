"""Small, local editor for declarative command batches."""

from __future__ import annotations

import time
from dataclasses import dataclass

from ..domain.commands import (
    MAX_COMMAND_BATCH_STEP_BYTES,
    MAX_COMMAND_BATCH_STEP_DELAY_MS,
    CommandBatch,
    CommandBatchStep,
)
from ..domain.errors import ConfigurationError
from ..domain.models import CommandEntry, CommandMode
from .bounded_value_combo import COMMAND_DELAY_OPTION_VALUES, BoundedIntCombo
from .dialog_transition import start_dialog_transition, stop_dialog_transition
from .popup_surface import refresh_combo_popup_themes
from .qt import (
    QCheckBox,
    QComboBox,
    QDialog,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QLineEdit,
    QPlainTextEdit,
    QPushButton,
    Qt,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)
from .theme import apply_theme, theme_key_for_widget


def _field_label(text: str) -> QLabel:
    """Create a secondary editor label with theme-aware contrast."""

    label = QLabel(text)
    label.setProperty("role", "muted")
    return label


@dataclass
class _DraftStep:
    mode: CommandMode = CommandMode.TEXT
    content: str = ""
    append_newline: bool = False
    delay_after_ms: int = 0


class _BoundedCommandTextEdit(QPlainTextEdit):
    """Keep pasted/typed editor text bounded before draft snapshots are rebuilt."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._max_bytes = MAX_COMMAND_BATCH_STEP_BYTES * 4
        self._limiting = False
        self._was_trimmed = False

    def set_max_bytes(self, maximum: int) -> None:
        """Set max bytes."""
        self._max_bytes = max(1, int(maximum))
        self._trim_current()

    def consume_trimmed(self) -> bool:
        """Consume trimmed."""
        trimmed = self._was_trimmed
        self._was_trimmed = False
        return trimmed

    def setPlainText(self, text: str) -> None:
        """Setplaintext."""
        bounded = _truncate_utf8(text, self._max_bytes)
        if bounded != text:
            self._was_trimmed = True
        super().setPlainText(bounded)

    def insertFromMimeData(self, source: object) -> None:
        """Insertfrommimedata."""
        if not hasattr(source, "hasText") or not source.hasText():
            super().insertFromMimeData(source)  # type: ignore[arg-type]
            return
        text = source.text()
        cursor = self.textCursor()
        selected = cursor.selectedText().replace("\u2029", "\n")
        current = self.toPlainText()
        available = self._max_bytes - (len(current.encode("utf-8")) - len(selected.encode("utf-8")))
        if available <= 0:
            self._was_trimmed = True
            return
        bounded = _truncate_utf8(text, available)
        if bounded != text:
            self._was_trimmed = True
        cursor.insertText(bounded)

    def keyPressEvent(self, event: object) -> None:
        """Keypressevent."""
        super().keyPressEvent(event)  # type: ignore[arg-type]
        self._trim_current()

    def _trim_current(self) -> None:
        """Trim current."""
        if self._limiting:
            return
        text = self.toPlainText()
        bounded = _truncate_utf8(text, self._max_bytes)
        if bounded == text:
            return
        position = min(self.textCursor().position(), len(bounded))
        self._was_trimmed = True
        self._limiting = True
        try:
            super().setPlainText(bounded)
            cursor = self.textCursor()
            cursor.setPosition(position)
            self.setTextCursor(cursor)
        finally:
            self._limiting = False


def _truncate_utf8(value: str, maximum: int) -> str:
    """Truncate utf8."""
    encoded = value.encode("utf-8")
    if len(encoded) <= maximum:
        return value
    return encoded[:maximum].decode("utf-8", errors="ignore")


class CommandBatchEditorDialog(QDialog):
    """Edit finite steps while keeping parsing and domain validation local."""

    def __init__(
        self,
        *,
        batch: CommandBatch | None = None,
        quick_entries: tuple[CommandEntry, ...] = (),
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        apply_theme(self, theme_key_for_widget(parent))
        self._batch = batch
        self._result: CommandBatch | None = None
        self._drafts = self._from_batch(batch) if batch is not None else [_DraftStep()]
        self._loading = False

        self.setWindowTitle("编辑批量命令" if batch is not None else "新建批量命令")
        self.resize(760, 560)
        root = QVBoxLayout(self)

        name_row = QHBoxLayout()
        name_row.addWidget(_field_label("名称"))
        self._name = QLineEdit(batch.name if batch is not None else "")
        self._name.setAccessibleName("批量命令名称")
        self._name.setPlaceholderText("例如：设备初始化")
        self._name.setMaxLength(128)
        name_row.addWidget(self._name, stretch=1)
        root.addLayout(name_row)

        quick_row = QHBoxLayout()
        quick_row.addWidget(_field_label("快捷命令"))
        self._quick_combo = QComboBox()
        self._quick_combo.setAccessibleName("可加入的快捷命令")
        self._quick_combo.setEditable(False)
        self._quick_combo.setToolTip("选择已有快捷命令并加入当前批量步骤；不会立即发送。")
        self._quick_combo.setAccessibleDescription(
            "选择一个已有快捷命令加入当前批量步骤；加入步骤不会立即发送数据。"
        )
        self._quick_combo.addItem("选择后加入步骤…")
        for entry in quick_entries:
            self._quick_combo.addItem(entry.name, entry)
        quick_row.addWidget(self._quick_combo, stretch=1)
        add_quick = QPushButton("加入")
        add_quick.clicked.connect(self._add_quick)
        add_quick.setAccessibleName("加入快捷命令步骤")
        self._add_quick_button = add_quick
        self._quick_combo.currentIndexChanged.connect(self._on_quick_selection_changed)
        quick_row.addWidget(add_quick)
        quick_row.addWidget(
            _field_label("固定顺序 · 无循环/脚本/广播；宏只在当前运行中保存")
        )
        root.addLayout(quick_row)

        self._table = QTableWidget(0, 4)
        self._table.setHorizontalHeaderLabels(("步骤", "内容摘要", "格式", "下一步前等待"))
        self._table.setAccessibleName("批量命令步骤表")
        self._table.setAccessibleDescription(
            "只读显示当前批量命令的固定步骤；使用下方编辑区修改当前步骤。"
        )
        self._table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        self._table.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
        self._table.setSelectionMode(QTableWidget.SelectionMode.SingleSelection)
        self._table.verticalHeader().setVisible(False)
        header = self._table.horizontalHeader()
        header.setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(1, QHeaderView.ResizeMode.Stretch)
        header.setSectionResizeMode(2, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(3, QHeaderView.ResizeMode.ResizeToContents)
        self._table.currentCellChanged.connect(self._on_current_row_changed)
        root.addWidget(self._table, stretch=1)
        self._table_empty = QLabel("暂无步骤 · 点击“添加步骤”继续。")
        self._table_empty.setObjectName("emptyState")
        self._table_empty.setProperty("role", "subtle")
        self._table_empty.setAccessibleName("批量命令步骤空态")
        root.addWidget(self._table_empty)

        edit_row = QHBoxLayout()
        edit_row.addWidget(_field_label("当前步骤"))
        self._mode = QComboBox()
        self._mode.setAccessibleName("当前步骤格式")
        self._mode.setEditable(False)
        self._mode.setToolTip("选择当前批量步骤按文本或 Hex（十六进制）编辑。")
        self._mode.setAccessibleDescription(
            "选择当前批量步骤的编辑格式：文本或十六进制；不会绕过批量发送限制。"
        )
        self._mode.addItem("文本", CommandMode.TEXT)
        self._mode.addItem("Hex", CommandMode.HEX)
        self._mode.currentIndexChanged.connect(self._on_draft_changed)
        edit_row.addWidget(self._mode)
        self._newline = QCheckBox("CRLF")
        self._newline.setAccessibleName("当前步骤追加 CRLF")
        self._newline.setToolTip("当前步骤发送时追加 CRLF 换行字节；只改变步骤 payload。")
        self._newline.setAccessibleDescription(
            "控制当前批量步骤是否追加 CRLF 换行字节；不会单独发送或执行步骤。"
        )
        self._newline.toggled.connect(self._on_draft_changed)
        edit_row.addWidget(self._newline)
        edit_row.addWidget(_field_label("延时"))
        self._delay = BoundedIntCombo(
            0,
            MAX_COMMAND_BATCH_STEP_DELAY_MS,
            values=COMMAND_DELAY_OPTION_VALUES,
            suffix=" ms",
        )
        self._delay.setAccessibleName("当前步骤延时")
        self._delay.currentIndexChanged.connect(self._on_draft_changed)
        edit_row.addWidget(self._delay)
        root.addLayout(edit_row)

        self._content = _BoundedCommandTextEdit()
        self._content.setAccessibleName("当前步骤内容")
        self._content.setAccessibleDescription(
            "文本步骤最多 512 字节；Hex 步骤输入会在保存时按 wire payload 上限校验。"
        )
        self._content.setPlaceholderText("输入当前步骤文本，或输入 Hex：AA 55 01")
        self._content.setMaximumHeight(100)
        self._content.textChanged.connect(self._on_draft_changed)
        root.addWidget(self._content)

        operations = QHBoxLayout()
        self._add_step_button = QPushButton("添加步骤")
        self._add_step_button.setAccessibleName("添加批量命令步骤")
        self._add_step_button.clicked.connect(self._add_step)
        operations.addWidget(self._add_step_button)
        self._copy_step_button = QPushButton("复制步骤")
        self._copy_step_button.setAccessibleName("复制当前批量命令步骤")
        self._copy_step_button.clicked.connect(self._copy_step)
        operations.addWidget(self._copy_step_button)
        self._move_up_button = QPushButton("上移")
        self._move_up_button.setAccessibleName("上移当前批量命令步骤")
        self._move_up_button.clicked.connect(lambda: self._move_step(-1))
        operations.addWidget(self._move_up_button)
        self._move_down_button = QPushButton("下移")
        self._move_down_button.setAccessibleName("下移当前批量命令步骤")
        self._move_down_button.clicked.connect(lambda: self._move_step(1))
        operations.addWidget(self._move_down_button)
        self._remove_step_button = QPushButton("删除步骤")
        self._remove_step_button.setAccessibleName("删除当前批量命令步骤")
        self._remove_step_button.setObjectName("dangerButton")
        self._remove_step_button.clicked.connect(self._remove_step)
        operations.addWidget(self._remove_step_button)
        operations.addStretch()
        root.addLayout(operations)

        self._error = QLabel()
        self._error.setProperty("role", "error")
        self._error.setAccessibleName("批量命令编辑错误")
        self._error.setAccessibleDescription("")
        self._error.setWordWrap(True)
        root.addWidget(self._error)

        buttons = QHBoxLayout()
        buttons.addStretch()
        cancel = QPushButton("取消")
        cancel.setAccessibleName("取消批量命令编辑")
        cancel.setToolTip("取消批量命令编辑；不会保存当前修改。")
        cancel.setAccessibleDescription("取消批量命令编辑；不会保存当前修改。")
        cancel.clicked.connect(self.reject)
        self._cancel_button = cancel
        buttons.addWidget(cancel)
        save = QPushButton("保存")
        save.setAccessibleName("保存批量命令")
        save.setToolTip("保存当前批量命令；只保存编辑内容，不会自动执行或发送。")
        save.setAccessibleDescription("保存当前批量命令；只保存编辑内容，不会自动执行或发送。")
        save.setObjectName("primaryButton")
        save.setDefault(True)
        save.clicked.connect(self._save)
        self._save_button = save
        buttons.addWidget(save)
        root.addLayout(buttons)

        self._render_table(selected=0 if self._drafts else None)
        self._install_tab_order()
        refresh_combo_popup_themes(self, theme_key_for_widget(self))

    def showEvent(self, event: object) -> None:
        """Showevent."""
        super().showEvent(event)  # type: ignore[arg-type]
        start_dialog_transition(self)

    def hideEvent(self, event: object) -> None:
        """Hideevent."""
        stop_dialog_transition(self)
        super().hideEvent(event)  # type: ignore[arg-type]

    @property
    def result_batch(self) -> CommandBatch | None:
        """Return the immutable result after the dialog was accepted."""

        return self._result

    def _from_batch(self, batch: CommandBatch) -> list[_DraftStep]:
        """From batch."""
        drafts: list[_DraftStep] = []
        for step in batch.steps:
            content = (
                step.entry.payload.hex(" ").upper()
                if step.entry.mode is CommandMode.HEX
                else step.entry.payload.decode("utf-8", errors="replace")
            )
            drafts.append(
                _DraftStep(
                    mode=step.entry.mode,
                    content=content,
                    append_newline=step.entry.append_newline,
                    delay_after_ms=step.delay_after_ms,
                )
            )
        return drafts

    def _current_row(self) -> int:
        """Current row."""
        row = self._table.currentRow()
        return row if 0 <= row < len(self._drafts) else -1

    def _render_table(self, *, selected: int | None = None) -> None:
        """Render table."""
        self._table.blockSignals(True)
        self._table.setRowCount(len(self._drafts))
        for row, draft in enumerate(self._drafts):
            self._set_table_row(row, draft)
        self._table.blockSignals(False)
        self._table_empty.setVisible(not self._drafts)
        has_draft = bool(self._drafts)
        for widget in (self._mode, self._newline, self._delay, self._content):
            widget.setEnabled(has_draft)
        if selected is not None and self._drafts:
            selected = min(max(selected, 0), len(self._drafts) - 1)
            self._table.setCurrentCell(selected, 0)
            self._load_row(selected)
        self._update_operation_buttons()

    def _install_tab_order(self) -> None:
        """Install tab order."""
        order = (
            self._name,
            self._quick_combo,
            self._add_quick_button,
            self._table,
            self._mode,
            self._newline,
            self._delay,
            self._content,
            self._add_step_button,
            self._copy_step_button,
            self._move_up_button,
            self._move_down_button,
            self._remove_step_button,
            self._cancel_button,
            self._save_button,
        )
        for first, second in zip(order, order[1:], strict=False):
            self.setTabOrder(first, second)

    def _update_operation_buttons(self) -> None:
        """Update operation buttons."""
        row = self._current_row()
        has_row = row >= 0
        at_capacity = len(self._drafts) >= 32
        self._add_step_button.setEnabled(not at_capacity)
        self._add_step_button.setToolTip(
            "最多添加 32 个步骤。" if at_capacity else "在当前批量命令末尾添加步骤。"
        )
        self._copy_step_button.setEnabled(has_row and not at_capacity)
        self._copy_step_button.setToolTip(
            "选择一个步骤后才能复制。"
            if not has_row
            else ("最多添加 32 个步骤。" if at_capacity else "复制当前步骤并插入到下一行。")
        )
        self._move_up_button.setEnabled(has_row and row > 0)
        self._move_up_button.setToolTip(
            "选择步骤后才能上移。"
            if not has_row
            else ("当前步骤已在第一行。" if row == 0 else "将当前步骤上移。")
        )
        self._move_down_button.setEnabled(has_row and row < len(self._drafts) - 1)
        self._move_down_button.setToolTip(
            "选择步骤后才能下移。"
            if not has_row
            else ("当前步骤已在最后一行。" if row == len(self._drafts) - 1 else "将当前步骤下移。")
        )
        self._remove_step_button.setEnabled(has_row)
        self._remove_step_button.setToolTip(
            "选择步骤后才能删除。" if not has_row else "删除当前步骤。"
        )
        quick_available = isinstance(self._quick_combo.currentData(), CommandEntry)
        self._add_quick_button.setEnabled(quick_available and not at_capacity)
        if not quick_available:
            quick_hint = "选择一个快捷命令后才能加入。"
        elif at_capacity:
            quick_hint = "最多添加 32 个步骤。"
        else:
            quick_hint = "将所选快捷命令加入为新步骤。"
        self._add_quick_button.setToolTip(quick_hint)
        self._add_quick_button.setAccessibleDescription(quick_hint)
        for button in (
            self._add_step_button,
            self._copy_step_button,
            self._move_up_button,
            self._move_down_button,
            self._remove_step_button,
        ):
            button.setAccessibleDescription(button.toolTip())

    def _set_table_row(self, row: int, draft: _DraftStep) -> None:
        """Set table row."""
        mode = "Hex" if draft.mode is CommandMode.HEX else "文本"
        summary = draft.content.replace("\n", "↵")[:64] or "（空）"
        step_item = QTableWidgetItem(str(row + 1))
        step_item.setData(Qt.ItemDataRole.AccessibleTextRole, f"步骤 {row + 1}")
        self._table.setItem(row, 0, step_item)
        summary_item = QTableWidgetItem(summary)
        summary_item.setToolTip(draft.content or "（空）")
        summary_item.setData(Qt.ItemDataRole.AccessibleTextRole, draft.content or "（空）")
        self._table.setItem(row, 1, summary_item)
        mode_item = QTableWidgetItem(mode)
        mode_item.setData(Qt.ItemDataRole.AccessibleTextRole, f"格式：{mode}")
        self._table.setItem(row, 2, mode_item)
        wait = "—" if row == len(self._drafts) - 1 else f"{draft.delay_after_ms} ms"
        wait_item = QTableWidgetItem(wait)
        wait_item.setData(Qt.ItemDataRole.AccessibleTextRole, f"下一步前等待：{wait}")
        self._table.setItem(row, 3, wait_item)

    def _load_row(self, row: int) -> None:
        """Load row."""
        if not 0 <= row < len(self._drafts):
            return
        draft = self._drafts[row]
        self._loading = True
        self._mode.setCurrentIndex(self._mode.findData(draft.mode))
        self._content.set_max_bytes(self._content_limit_for(draft.mode, draft.append_newline))
        self._content.setPlainText(draft.content)
        self._newline.setChecked(draft.append_newline)
        self._delay.setValue(draft.delay_after_ms)
        self._content.consume_trimmed()
        self._loading = False

    def _on_current_row_changed(self, row: int, *_args: object) -> None:
        """On current row changed."""
        self._load_row(row)
        self._update_operation_buttons()

    def _on_quick_selection_changed(self, _index: int = -1) -> None:
        """On quick selection changed."""
        self._update_operation_buttons()

    def _on_draft_changed(self, *_args: object) -> None:
        """On draft changed."""
        if self._loading:
            return
        self._error.clear()
        self._error.setAccessibleDescription("")
        row = self._current_row()
        if row < 0:
            return
        mode = self._mode.currentData()
        if not isinstance(mode, CommandMode):
            mode = CommandMode(mode)
        self._content.set_max_bytes(self._content_limit_for(mode, self._newline.isChecked()))
        trimmed = self._content.consume_trimmed()
        self._drafts[row] = _DraftStep(
            mode=mode,
            content=self._content.toPlainText(),
            append_newline=self._newline.isChecked(),
            delay_after_ms=self._delay.value(),
        )
        self._set_table_row(row, self._drafts[row])
        if trimmed:
            message = "当前步骤输入已按编辑器上限截断；保存时仍会校验 wire payload。"
            self._error.setText(message)
            self._error.setAccessibleDescription(message)
        elif mode is CommandMode.HEX:
            try:
                payload_size = len(bytes.fromhex("".join(self._content.toPlainText().split())))
            except ValueError:
                payload_size = 0
            wire_size = payload_size + (2 if self._newline.isChecked() else 0)
            if wire_size > MAX_COMMAND_BATCH_STEP_BYTES:
                message = (
                    f"当前步骤 wire payload 为 {wire_size} B，"
                    f"超过单步上限 {MAX_COMMAND_BATCH_STEP_BYTES} B；保存前请缩短内容。"
                )
                self._error.setText(message)
                self._error.setAccessibleDescription(message)

    @staticmethod
    def _content_limit_for(mode: CommandMode, append_newline: bool) -> int:
        """Content limit for."""
        if mode is CommandMode.HEX:
            return MAX_COMMAND_BATCH_STEP_BYTES * 4
        return MAX_COMMAND_BATCH_STEP_BYTES - (2 if append_newline else 0)

    def _add_step(self) -> None:
        """Add step."""
        if len(self._drafts) >= 32:
            self._error.setText("最多添加 32 个步骤。")
            self._error.setAccessibleDescription("最多添加 32 个步骤。")
            return
        self._drafts.append(_DraftStep())
        self._render_table(selected=len(self._drafts) - 1)

    def _add_quick(self) -> None:
        """Add quick."""
        entry = self._quick_combo.currentData()
        if not isinstance(entry, CommandEntry):
            self._error.setText("请先选择一个快捷命令。")
            self._error.setAccessibleDescription("请先选择一个快捷命令。")
            return
        if len(self._drafts) >= 32:
            self._error.setText("最多添加 32 个步骤。")
            self._error.setAccessibleDescription("最多添加 32 个步骤。")
            return
        content = (
            entry.payload.hex(" ").upper()
            if entry.mode is CommandMode.HEX
            else entry.payload.decode("utf-8", errors="replace")
        )
        wire_size = len(entry.payload) + (2 if entry.append_newline else 0)
        if wire_size > MAX_COMMAND_BATCH_STEP_BYTES:
            crlf_note = "含 CRLF 的 " if entry.append_newline else ""
            message = (
                f"快捷命令“{entry.name}”加入失败："
                f"{crlf_note}wire payload 为 {wire_size} B，"
                f"超过单步上限 {MAX_COMMAND_BATCH_STEP_BYTES} B。"
            )
            self._error.setText(message)
            self._error.setAccessibleDescription(message)
            return
        self._drafts.append(
            _DraftStep(
                mode=entry.mode,
                content=content,
                append_newline=entry.append_newline,
            )
        )
        self._render_table(selected=len(self._drafts) - 1)

    def _copy_step(self) -> None:
        """Copy step."""
        row = self._current_row()
        if row < 0:
            return
        if len(self._drafts) >= 32:
            self._error.setText("最多添加 32 个步骤。")
            self._error.setAccessibleDescription("最多添加 32 个步骤。")
            return
        self._drafts.insert(row + 1, _DraftStep(**vars(self._drafts[row])))
        self._render_table(selected=row + 1)

    def _move_step(self, offset: int) -> None:
        """Move step."""
        row = self._current_row()
        target = row + offset
        if row < 0 or not 0 <= target < len(self._drafts):
            return
        self._drafts[row], self._drafts[target] = self._drafts[target], self._drafts[row]
        self._render_table(selected=target)

    def _remove_step(self) -> None:
        """Remove step."""
        row = self._current_row()
        if row < 0:
            return
        del self._drafts[row]
        self._render_table(selected=min(row, len(self._drafts) - 1) if self._drafts else None)

    def _save(self) -> None:
        """Save."""
        try:
            name = self._name.text().strip()
            if not name:
                raise ConfigurationError("批量命令名称不能为空。")
            self._on_draft_changed()
            steps: list[CommandBatchStep] = []
            for index, draft in enumerate(self._drafts):
                try:
                    payload = (
                        bytes.fromhex("".join(draft.content.split()))
                        if draft.mode is CommandMode.HEX
                        else draft.content.encode("utf-8")
                    )
                except ValueError as exc:
                    raise ConfigurationError(f"步骤 {index + 1} Hex 格式无效。") from exc
                entry = CommandEntry(
                    name=f"{name[:100]} · 步骤 {index + 1}",
                    payload=payload,
                    mode=draft.mode,
                    append_newline=draft.append_newline,
                    created_at=time.monotonic(),
                )
                delay = draft.delay_after_ms if index < len(self._drafts) - 1 else 0
                steps.append(CommandBatchStep(entry=entry, delay_after_ms=delay))
            if self._batch is None:
                self._result = CommandBatch(name=name, steps=tuple(steps))
            else:
                self._result = CommandBatch(
                    name=name,
                    steps=tuple(steps),
                    batch_id=self._batch.batch_id,
                )
        except ConfigurationError as exc:
            self._error.setText(str(exc))
            self._error.setAccessibleDescription(str(exc))
            return
        self.accept()
