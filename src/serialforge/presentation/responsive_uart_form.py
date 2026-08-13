"""Responsive geometry owner for the UART configuration fields."""

from __future__ import annotations

from enum import StrEnum

from .qt import QEvent, QGridLayout, QLayout, QSizePolicy, QWidget


class _UartFormMode(StrEnum):
    """Responsive compositions for the existing UART field wrappers."""

    REGULAR = "regular"
    COMPACT = "compact"
    NARROW = "narrow"


class ResponsiveUartForm(QWidget):
    """Arrange existing labeled fields without owning UART behavior."""

    def __init__(
        self,
        *,
        port: QWidget,
        baud: QWidget,
        data_bits: QWidget,
        parity: QWidget,
        stop_bits: QWidget,
        flow_control: QWidget,
        read_timeout: QWidget,
        write_timeout: QWidget,
        inter_byte_timeout: QWidget,
        line_controls: QWidget,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("responsiveUartForm")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._fields = (
            port,
            baud,
            data_bits,
            parity,
            stop_bits,
            flow_control,
            read_timeout,
            write_timeout,
            inter_byte_timeout,
            line_controls,
        )
        self._mode: _UartFormMode | None = None
        self._reflowing = False
        self._layout = QGridLayout(self)
        self._layout.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(12)
        self._layout.setVerticalSpacing(10)
        self._relayout(force=True)

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
            self._invalidate_sizing()

    def event(self, event: object) -> bool:
        """Event."""
        accepted = super().event(event)  # type: ignore[arg-type]
        if event.type() is QEvent.Type.LayoutRequest:
            self._invalidate_sizing()
        return accepted

    def minimumSizeHint(self) -> object:
        """Expose only the single-column field contract as the width floor."""

        hint = super().minimumSizeHint()
        hint.setWidth(self._required_width(_UartFormMode.NARROW))
        return hint

    def sizeHint(self) -> object:
        """Prefer a readable regular form without making it the hard floor."""

        hint = super().sizeHint()
        hint.setWidth(
            max(
                hint.width(),
                self._required_width(_UartFormMode.REGULAR, preferred=True),
            )
        )
        return hint

    def _invalidate_sizing(self) -> None:
        """Refresh sizing inputs without churning unchanged widget positions."""

        if self._reflowing:
            return
        self._layout.invalidate()
        self._layout.activate()
        self.updateGeometry()
        self._relayout()

    def _relayout(self, *, force: bool = False) -> None:
        """Select and apply one of the three field compositions idempotently."""

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
            for field in self._fields:
                self._layout.removeWidget(field)
            rows = self._rows_for_mode(mode)
            for row_index, row in enumerate(rows):
                if mode is _UartFormMode.REGULAR and row_index == 0:
                    for column, field in enumerate(row):
                        self._layout.addWidget(field, row_index, column * 2, 1, 2)
                else:
                    for column, field in enumerate(row):
                        self._layout.addWidget(field, row_index, column)
            self._apply_column_contract(mode)
            self._mode = mode
            self._layout.invalidate()
            self._layout.activate()
            self.updateGeometry()
        finally:
            self._reflowing = False

    def _mode_for_width(self) -> _UartFormMode:
        """Choose the widest composition whose sizing contract fits."""

        available_width = max(0, self.contentsRect().width())
        if available_width >= self._required_width(
            _UartFormMode.REGULAR,
            preferred=True,
        ):
            return _UartFormMode.REGULAR
        if available_width >= self._required_width(_UartFormMode.COMPACT):
            return _UartFormMode.COMPACT
        return _UartFormMode.NARROW

    def _required_width(
        self,
        mode: _UartFormMode,
        *,
        preferred: bool = False,
    ) -> int:
        """Derive a mode width from explicit column sizing contracts."""

        spacing = max(0, self._layout.horizontalSpacing())
        columns = self._column_widths(mode, preferred=preferred)
        margins = self._layout.contentsMargins()
        return (
            sum(columns)
            + spacing * max(0, len(columns) - 1)
            + margins.left()
            + margins.right()
        )

    def _column_widths(
        self,
        mode: _UartFormMode,
        *,
        preferred: bool = False,
    ) -> tuple[int, ...]:
        """Return deterministic per-column widths for the active composition."""

        if mode is _UartFormMode.REGULAR:
            return self._regular_column_widths(preferred=preferred)
        if mode is _UartFormMode.COMPACT:
            return self._compact_column_widths()
        return (max(self._minimum_field_width(field) for field in self._fields),)

    def _regular_column_widths(self, *, preferred: bool) -> tuple[int, ...]:
        """Regular column widths."""
        (
            port,
            baud,
            data_bits,
            parity,
            stop_bits,
            flow_control,
            read_timeout,
            write_timeout,
            inter_byte_timeout,
            line_controls,
        ) = self._fields
        width = self._preferred_field_width if preferred else self._minimum_field_width
        columns = [
            max(width(data_bits), width(read_timeout)),
            max(width(parity), width(write_timeout)),
            max(width(stop_bits), width(inter_byte_timeout)),
            max(width(flow_control), width(line_controls)),
        ]
        spacing = max(0, self._layout.horizontalSpacing())
        left = columns[:2]
        right = columns[2:]
        self._water_fill(left, width(port) - spacing)
        self._water_fill(right, width(baud) - spacing)
        return tuple(left + right)

    def _compact_column_widths(self) -> tuple[int, int]:
        """Compact column widths."""
        (
            port,
            baud,
            data_bits,
            parity,
            stop_bits,
            flow_control,
            read_timeout,
            write_timeout,
            inter_byte_timeout,
            line_controls,
        ) = self._fields
        return (
            max(
                self._minimum_field_width(field)
                for field in (
                    port,
                    data_bits,
                    stop_bits,
                    read_timeout,
                    inter_byte_timeout,
                )
            ),
            max(
                self._minimum_field_width(field)
                for field in (
                    baud,
                    parity,
                    flow_control,
                    write_timeout,
                    line_controls,
                )
            ),
        )

    def _apply_column_contract(self, mode: _UartFormMode) -> None:
        """Make grid columns honor the same contract used for mode selection."""

        for column in range(4):
            self._layout.setColumnMinimumWidth(column, 0)
            self._layout.setColumnStretch(column, 0)
        widths = self._column_widths(
            mode,
            preferred=mode is _UartFormMode.REGULAR,
        )
        for column, width in enumerate(widths):
            self._layout.setColumnMinimumWidth(column, width)
            self._layout.setColumnStretch(column, 1)

    @staticmethod
    def _water_fill(widths: list[int], required_total: int) -> None:
        """Fill a spanning row deficit deterministically from narrow to wide."""

        deficit = max(0, required_total - sum(widths))
        while deficit:
            narrowest = min(widths)
            indices = [
                index for index, width in enumerate(widths) if width == narrowest
            ]
            wider = [width for width in widths if width > narrowest]
            allocation = deficit
            if wider:
                allocation = min(
                    deficit,
                    (min(wider) - narrowest) * len(indices),
                )
            share, remainder = divmod(allocation, len(indices))
            for rank, index in enumerate(indices):
                widths[index] += share + (1 if rank < remainder else 0)
            deficit -= allocation

    def _rows_for_mode(self, mode: _UartFormMode) -> tuple[tuple[QWidget, ...], ...]:
        """Rows for mode."""
        (
            port,
            baud,
            data_bits,
            parity,
            stop_bits,
            flow_control,
            read_timeout,
            write_timeout,
            inter_byte_timeout,
            line_controls,
        ) = self._fields
        if mode is _UartFormMode.REGULAR:
            return (
                (port, baud),
                (data_bits, parity, stop_bits, flow_control),
                (read_timeout, write_timeout, inter_byte_timeout, line_controls),
            )
        if mode is _UartFormMode.COMPACT:
            return (
                (port, baud),
                (data_bits, parity),
                (stop_bits, flow_control),
                (read_timeout, write_timeout),
                (inter_byte_timeout, line_controls),
            )
        return tuple((field,) for field in self._fields)

    @staticmethod
    def _minimum_field_width(field: QWidget) -> int:
        """Use hard minimum contracts without turning preferred width into a floor."""

        return max(field.minimumWidth(), field.minimumSizeHint().width())

    @staticmethod
    def _preferred_field_width(field: QWidget) -> int:
        """Read the preferred contract only after honoring the hard minimum."""

        return max(
            ResponsiveUartForm._minimum_field_width(field),
            field.sizeHint().width(),
        )


__all__ = ["ResponsiveUartForm"]
