"""Command-management workspace composition.

The builder owns only command-page widgets. Definitions, snapshots, execution,
and enablement remain in the ViewModel and command controller.
"""

from __future__ import annotations

from enum import StrEnum
from functools import partial

from ..action_surface import ActionRailButton, BusyActionButton
from ..command_batch_empty_state import CommandBatchEmptyState
from ..command_batch_surface import CommandBatchSurfaceLabel
from ..command_context_band import CommandContextBand
from ..qt import (
    QComboBox,
    QEvent,
    QGridLayout,
    QHeaderView,
    QLabel,
    QLayout,
    QSizePolicy,
    QTableWidget,
    QVBoxLayout,
    QWidget,
)
from .commands import (
    delete_command_batch_action,
    edit_command_batch_action,
    new_command_batch_action,
    on_command_batch_selection_changed,
    run_command_batch_action,
)


def _section_label(text: str) -> QLabel:
    """Create a non-interactive command-page section label."""

    label = QLabel(text)
    label.setProperty("role", "section")
    return label


class _CommandActionRowMode(StrEnum):
    """Responsive compositions for the command action row."""

    REGULAR = "regular"
    COMPACT = "compact"
    NARROW = "narrow"


class _ResponsiveCommandActionRow(QWidget):
    """Arrange existing command actions without owning their callbacks."""

    def __init__(
        self,
        widgets: tuple[QWidget, ...],
        *,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("responsiveCommandActionRow")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._widgets = tuple(widgets)
        self._mode: _CommandActionRowMode | None = None
        self._reflowing = False
        self._layout = QGridLayout(self)
        self._layout.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(8)
        self._layout.setVerticalSpacing(8)
        self._relayout(force=True)

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def changeEvent(self, event: object) -> None:
        super().changeEvent(event)  # type: ignore[arg-type]
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self._invalidate_sizing()

    def event(self, event: object) -> bool:
        accepted = super().event(event)  # type: ignore[arg-type]
        if event.type() is QEvent.Type.LayoutRequest:
            self._invalidate_sizing()
        return accepted

    def minimumSizeHint(self) -> object:
        hint = super().minimumSizeHint()
        hint.setWidth(self._required_width(_CommandActionRowMode.NARROW))
        return hint

    def sizeHint(self) -> object:
        hint = super().sizeHint()
        hint.setWidth(
            max(
                hint.width(),
                self._required_width(_CommandActionRowMode.REGULAR, preferred=True),
            )
        )
        return hint

    def _invalidate_sizing(self) -> None:
        if self._reflowing:
            return
        self._layout.invalidate()
        self._layout.activate()
        self.updateGeometry()
        self._relayout()

    def _relayout(self, *, force: bool = False) -> None:
        if self._reflowing:
            return
        mode = self._mode_for_width()
        if not force and mode is self._mode:
            self._apply_column_contract(mode)
            self._layout.invalidate()
            self._layout.activate()
            return

        self._reflowing = True
        try:
            for widget in self._widgets:
                self._layout.removeWidget(widget)
            for row_index, row in enumerate(self._rows_for_mode(mode)):
                for column, widget in enumerate(row):
                    self._layout.addWidget(widget, row_index, column)
            self._apply_column_contract(mode)
            self._mode = mode
            self._layout.invalidate()
            self._layout.activate()
            self.updateGeometry()
        finally:
            self._reflowing = False

    def _mode_for_width(self) -> _CommandActionRowMode:
        available_width = max(0, self.contentsRect().width())
        if available_width >= self._required_width(
            _CommandActionRowMode.REGULAR,
            preferred=True,
        ):
            return _CommandActionRowMode.REGULAR
        if available_width >= self._required_width(_CommandActionRowMode.COMPACT):
            return _CommandActionRowMode.COMPACT
        return _CommandActionRowMode.NARROW

    def _required_width(
        self,
        mode: _CommandActionRowMode,
        *,
        preferred: bool = False,
    ) -> int:
        spacing = max(0, self._layout.horizontalSpacing())
        widest_row = max(
            sum(self._widget_width(widget, preferred=preferred) for widget in row)
            + spacing * max(0, len(row) - 1)
            for row in self._rows_for_mode(mode)
        )
        margins = self._layout.contentsMargins()
        return widest_row + margins.left() + margins.right()

    def _rows_for_mode(
        self,
        mode: _CommandActionRowMode,
    ) -> tuple[tuple[QWidget, ...], ...]:
        if mode is _CommandActionRowMode.REGULAR:
            return (self._widgets,)
        if mode is _CommandActionRowMode.COMPACT:
            split = max(1, (len(self._widgets) + 1) // 2)
            return ((self._widgets[0],), self._widgets[1:split + 1], self._widgets[split + 1:])
        return tuple((widget,) for widget in self._widgets)

    def _apply_column_contract(self, mode: _CommandActionRowMode) -> None:
        rows = self._rows_for_mode(mode)
        widths = [0] * max(len(row) for row in rows)
        for row in rows:
            for column, widget in enumerate(row):
                widths[column] = max(widths[column], self._widget_width(widget))
        for column in range(len(self._widgets)):
            self._layout.setColumnMinimumWidth(column, 0)
            self._layout.setColumnStretch(column, 0)
        for column, width in enumerate(widths):
            self._layout.setColumnMinimumWidth(column, width)
            self._layout.setColumnStretch(column, 1)

    @staticmethod
    def _widget_width(widget: QWidget, *, preferred: bool = False) -> int:
        minimum = max(widget.minimumWidth(), widget.minimumSizeHint().width())
        return max(minimum, widget.sizeHint().width()) if preferred else minimum


def build_command_workspace(window) -> QVBoxLayout:
    """Build a responsive command context band and a separate command action row."""

    root = QVBoxLayout()
    root.setContentsMargins(0, 0, 0, 0)
    root.setSpacing(12)

    history_label = _section_label("发送历史")
    window._history_combo = QComboBox()
    window._history_combo.setAccessibleName("发送历史选择")
    window._history_combo.setEditable(False)
    window._history_combo.setToolTip("选择一条已发送历史，将内容加载到发送区；不会自动发送。")
    window._history_combo.setAccessibleDescription(
        "选择一条已有发送历史并加载到发送区；加载不会自动发送或连接设备。"
    )
    window._history_combo.setMinimumWidth(220)
    window._history_combo.activated.connect(window._load_history)
    clear_history = ActionRailButton("清除历史")
    clear_history.setAccessibleName("清除发送历史")
    clear_history.setToolTip("清除本地发送历史；不会删除原始记录或断开连接。")
    clear_history.setAccessibleDescription(
        "清除本地发送历史；不会删除原始记录、快捷命令或断开设备连接。"
    )
    clear_history.clicked.connect(window._view_model.clear_history)
    window._clear_history_button = clear_history

    batch_label = _section_label("批量命令")
    window._command_batch_combo = QComboBox()
    window._command_batch_combo.setPlaceholderText("暂无批量命令 · 点击新建")
    window._command_batch_combo.setAccessibleName("批量命令选择")
    window._command_batch_combo.setEditable(False)
    window._command_batch_combo.setToolTip("选择批量命令后可编辑、删除或执行；不会自动执行。")
    window._command_batch_combo.setAccessibleDescription(
        "选择一个批量命令后进行编辑、删除或显式执行；选择本身不会向设备发送数据。"
    )
    window._command_batch_combo.currentIndexChanged.connect(
        partial(on_command_batch_selection_changed, window)
    )
    window._command_context_band = CommandContextBand(
        history_label,
        window._history_combo,
        clear_history,
        batch_label,
        window._command_batch_combo,
    )
    root.addWidget(window._command_context_band)

    new_batch = ActionRailButton("新建")
    new_batch.setAccessibleName("新建批量命令")
    new_batch.clicked.connect(partial(new_command_batch_action, window))
    edit_batch = ActionRailButton("编辑")
    edit_batch.setAccessibleName("编辑所选批量命令")
    edit_batch.clicked.connect(partial(edit_command_batch_action, window))
    delete_batch = ActionRailButton("删除")
    delete_batch.setAccessibleName("删除所选批量命令")
    delete_batch.setObjectName("dangerButton")
    delete_batch.clicked.connect(partial(delete_command_batch_action, window))
    run_batch = ActionRailButton("执行")
    run_batch.setAccessibleName("执行所选批量命令")
    run_batch.clicked.connect(partial(run_command_batch_action, window))
    stop_batch = BusyActionButton("停止")
    stop_batch.setAccessibleName("停止当前批量命令")
    stop_batch.setObjectName("dangerButton")
    stop_batch.clicked.connect(window._view_model.stop_command_batch)
    actions_row = _ResponsiveCommandActionRow(
        (
            _section_label("命令操作"),
            new_batch,
            edit_batch,
            delete_batch,
            run_batch,
            stop_batch,
        )
    )
    root.addWidget(actions_row)
    window._new_batch_button = new_batch
    window._edit_batch_button = edit_batch
    window._delete_batch_button = delete_batch
    window._run_batch_button = run_batch
    window._stop_batch_button = stop_batch
    window._command_batch_status = CommandBatchSurfaceLabel()
    window._command_batch_status.setText(
        "固定顺序 · 无循环/脚本/广播；RTT 批量命令留到最后阶段"
    )
    window._command_batch_status.setObjectName("commandBatchStatus")
    window._command_batch_status.setProperty("role", "status")
    window._command_batch_status.setProperty("state", "empty")
    window._command_batch_status.setWordWrap(True)
    window._command_batch_status.setAccessibleName("批量命令状态")
    window._command_batch_status.setAccessibleDescription(window._command_batch_status.text())
    window._command_batch_status.setToolTip(window._command_batch_status.text())
    root.addWidget(window._command_batch_status)

    window._command_batch_results = QTableWidget(0, 3)
    window._command_batch_results.setHorizontalHeaderLabels(("步骤", "状态", "说明"))
    window._command_batch_results.setMaximumHeight(130)
    window._command_batch_results.setAccessibleName("批量命令步骤结果")
    window._command_batch_results.setAccessibleDescription(
        "只读显示当前批量命令各步骤的待执行、执行中、已提交或失败状态。"
    )
    window._command_batch_results.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
    window._command_batch_results.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
    window._command_batch_results.setAlternatingRowColors(True)
    window._command_batch_results.verticalHeader().setVisible(False)
    batch_header = window._command_batch_results.horizontalHeader()
    batch_header.setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
    batch_header.setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)
    batch_header.setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
    root.addWidget(window._command_batch_results)

    window._command_batch_empty = CommandBatchEmptyState()
    window._command_batch_empty.new_requested.connect(
        partial(new_command_batch_action, window)
    )
    # Keep the empty state at its natural height. The page owns the scrollable
    # canvas; the empty-state card owns only its onboarding content.
    root.addWidget(window._command_batch_empty)
    return root


__all__ = ["build_command_workspace"]
