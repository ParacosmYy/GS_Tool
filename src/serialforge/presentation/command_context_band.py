"""Responsive presentation owner for command-page selection context."""

from __future__ import annotations

from .action_surface import ActionRailButton
from .qt import (
    QComboBox,
    QEvent,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLayout,
    QSizePolicy,
    QVBoxLayout,
    QWidget,
)


class _CommandContextCell(QFrame):
    """Group one existing selector and its optional trailing action."""

    def __init__(
        self,
        label: QLabel,
        combo: QComboBox,
        trailing_action: ActionRailButton | None = None,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("commandContextCell")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )

        root = QVBoxLayout(self)
        root.setContentsMargins(10, 8, 10, 10)
        root.setSpacing(6)

        heading = QHBoxLayout()
        heading.setContentsMargins(0, 0, 0, 0)
        heading.setSpacing(8)
        heading.addWidget(label)
        heading.addStretch(1)
        if trailing_action is not None:
            heading.addWidget(trailing_action)
        root.addLayout(heading)
        root.addWidget(combo)


class CommandContextBand(QFrame):
    """Arrange existing command-page selectors without owning their behavior."""

    _WIDE_COLUMNS = 2
    _COMPACT_COLUMNS = 1

    def __init__(
        self,
        history_label: QLabel,
        history_combo: QComboBox,
        clear_history_button: ActionRailButton,
        batch_label: QLabel,
        batch_combo: QComboBox,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("commandContextBand")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self.setAccessibleName("命令上下文选择")
        self.setAccessibleDescription(
            "选择发送历史或批量命令；选择不会自动发送或执行。"
        )

        self._columns = 0
        self._layout = QGridLayout(self)
        self._layout.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(10)
        self._layout.setVerticalSpacing(10)
        self._cells = (
            _CommandContextCell(
                history_label,
                history_combo,
                clear_history_button,
                self,
            ),
            _CommandContextCell(batch_label, batch_combo, parent=self),
        )
        self._relayout()

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def changeEvent(self, event: object) -> None:
        """Changeevent."""
        super().changeEvent(event)  # type: ignore[arg-type]
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self._relayout()

    def event(self, event: object) -> bool:
        """Event."""
        accepted = super().event(event)  # type: ignore[arg-type]
        if event.type() is QEvent.Type.LayoutRequest:
            self._relayout()
        return accepted

    def _cell_minimum_size(self, cell: QWidget) -> object:
        """Cell minimum size."""
        return cell.minimumSize().expandedTo(cell.minimumSizeHint())

    def _cell_preferred_size(self, cell: QWidget) -> object:
        """Cell preferred size."""
        return cell.sizeHint().expandedTo(self._cell_minimum_size(cell))

    def _vertical_size(self, columns: int, preferred: bool) -> int:
        """Vertical size."""
        sizes = tuple(
            self._cell_preferred_size(cell) if preferred else self._cell_minimum_size(cell)
            for cell in self._cells
        )
        rows = tuple(
            max(
                sizes[index].height()
                for index in range(row * columns, min((row + 1) * columns, len(sizes)))
            )
            for row in range((len(sizes) + columns - 1) // columns)
        )
        margins = self._layout.contentsMargins()
        return (
            sum(rows)
            + max(0, len(rows) - 1) * max(0, self._layout.verticalSpacing())
            + margins.top()
            + margins.bottom()
        )

    def _compact_minimum_width(self) -> int:
        """Compact minimum width."""
        margins = self._layout.contentsMargins()
        return (
            max(self._cell_minimum_size(cell).width() for cell in self._cells)
            + margins.left()
            + margins.right()
        )

    def _wide_preferred_width(self) -> int:
        """Wide preferred width."""
        margins = self._layout.contentsMargins()
        return (
            sum(self._cell_preferred_size(cell).width() for cell in self._cells)
            + max(0, self._layout.horizontalSpacing())
            + margins.left()
            + margins.right()
        )

    def minimumSizeHint(self) -> object:
        """Keep the band shrinkable enough to enter its single-column mode."""

        hint = super().minimumSizeHint()
        columns = self._columns if self._columns in (1, 2) else self._WIDE_COLUMNS
        hint.setWidth(self._compact_minimum_width())
        hint.setHeight(self._vertical_size(columns, False))
        return hint

    def sizeHint(self) -> object:
        """Prefer the two-column composition without forcing it as a minimum."""

        hint = super().sizeHint()
        columns = self._columns if self._columns in (1, 2) else self._WIDE_COLUMNS
        if columns == self._WIDE_COLUMNS:
            hint.setWidth(max(hint.width(), self._wide_preferred_width()))
        else:
            margins = self._layout.contentsMargins()
            hint.setWidth(
                max(
                    hint.width(),
                    max(self._cell_preferred_size(cell).width() for cell in self._cells)
                    + margins.left()
                    + margins.right(),
                )
            )
        hint.setHeight(max(hint.height(), self._vertical_size(columns, True)))
        return hint

    def _wide_required_width(self) -> int:
        """Return the hard lower bound for two non-compressed cells."""

        margins = self._layout.contentsMargins()
        spacing = max(0, self._layout.horizontalSpacing())
        return (
            sum(self._cell_minimum_size(cell).width() for cell in self._cells)
            + spacing
            + margins.left()
            + margins.right()
        )

    def _relayout(self) -> None:
        """Place existing cells in one or two columns using Qt size contracts."""

        available_width = max(0, self.contentsRect().width())
        required_width = max(self._wide_required_width(), self._wide_preferred_width())
        columns = (
            self._WIDE_COLUMNS
            if available_width >= required_width
            else self._COMPACT_COLUMNS
        )
        if columns == self._columns and self._layout.count() == len(self._cells):
            return

        self._columns = columns
        for cell in self._cells:
            self._layout.removeWidget(cell)
        for index, cell in enumerate(self._cells):
            row, column = divmod(index, columns)
            self._layout.addWidget(cell, row, column)
        for column in range(self._WIDE_COLUMNS):
            self._layout.setColumnStretch(column, 1 if column < columns else 0)
        self.updateGeometry()


__all__ = ["CommandContextBand"]
