"""Presentation-only responsive grid for embedded station capability cards."""

from __future__ import annotations

from collections.abc import Iterable

from .qt import QGridLayout, QSizePolicy, Qt, QWidget

_DEFAULT_CARD_MIN_WIDTH = 360
_GRID_SPACING = 10
_MAX_COLUMNS = 3


class ResponsiveCapabilityGrid(QWidget):
    """Reflow existing capability cards without owning their interaction state."""

    def __init__(
        self,
        cards: Iterable[QWidget],
        *,
        card_min_width: int = _DEFAULT_CARD_MIN_WIDTH,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        card_min_width = int(card_min_width)
        if card_min_width <= 0:
            raise ValueError("card_min_width must be positive")
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._cards = tuple(cards)
        self._card_min_width = card_min_width
        self._column_count = 0
        self._layout = QGridLayout(self)
        self._layout.setHorizontalSpacing(_GRID_SPACING)
        self._layout.setVerticalSpacing(_GRID_SPACING)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._refresh_columns()

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._refresh_columns()

    def _refresh_columns(self) -> None:
        """Refresh columns."""
        columns = self._columns_for_width(self.width())
        if columns == self._column_count:
            return
        self._column_count = columns
        for column in range(_MAX_COLUMNS):
            self._layout.setColumnStretch(column, 0)
        for card in self._cards:
            self._layout.removeWidget(card)
        for index, card in enumerate(self._cards):
            self._layout.addWidget(card, index // columns, index % columns)
        for column in range(columns):
            self._layout.setColumnStretch(column, 1)
        self._layout.activate()

    def _columns_for_width(self, width: int) -> int:
        """Columns for width."""
        if not self._cards:
            return 1
        margins = self._layout.contentsMargins()
        available = max(
            0,
            int(width) - margins.left() - margins.right(),
        )
        spacing = max(0, self._layout.horizontalSpacing())
        max_columns = min(len(self._cards), _MAX_COLUMNS)
        for columns in range(max_columns, 1, -1):
            required = columns * self._card_min_width + (columns - 1) * spacing
            if available >= required:
                return columns
        return 1


__all__ = ["ResponsiveCapabilityGrid"]
