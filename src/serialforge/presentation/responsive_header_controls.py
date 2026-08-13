"""Responsive geometry owner for the global header control clusters."""

from __future__ import annotations

from enum import StrEnum

from .qt import QEvent, QGridLayout, QLayout, QSizePolicy, QWidget


class _HeaderControlsMode(StrEnum):
    """Responsive compositions for the existing header control clusters."""

    REGULAR = "regular"
    COMPACT = "compact"
    NARROW = "narrow"


class ResponsiveHeaderControls(QWidget):
    """Arrange status, motion, and theme clusters without owning their state."""

    def __init__(
        self,
        *,
        status_cluster: QWidget,
        motion_controls: QWidget,
        theme_controls: QWidget,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("responsiveHeaderControls")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._clusters = (status_cluster, motion_controls, theme_controls)
        self._mode: _HeaderControlsMode | None = None
        self._reflowing = False
        self._layout = QGridLayout(self)
        self._layout.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(8)
        self._layout.setVerticalSpacing(6)
        self._relayout(force=True)

    @property
    def mode(self) -> _HeaderControlsMode | None:
        """Expose the current geometry mode for diagnostics and visual audits."""

        return self._mode

    def showEvent(self, event: object) -> None:
        super().showEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def changeEvent(self, event: object) -> None:
        super().changeEvent(event)  # type: ignore[arg-type]
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self.invalidate_sizing()

    def event(self, event: object) -> bool:
        accepted = super().event(event)  # type: ignore[arg-type]
        if event.type() in {
            QEvent.Type.LayoutRequest,
            QEvent.Type.Polish,
        }:
            self.invalidate_sizing()
        return accepted

    def invalidate_sizing(self) -> None:
        """Refresh child sizing contracts and reflow only when the mode changes."""

        if self._reflowing:
            return
        self._layout.invalidate()
        self._layout.activate()
        self.updateGeometry()
        self._relayout()

    def set_density(self, *, compact: bool) -> None:
        """Apply the header density spacing before refreshing its row contract."""

        spacing = 6 if compact else 8
        if self._layout.horizontalSpacing() == spacing:
            return
        self._layout.setHorizontalSpacing(spacing)
        self.invalidate_sizing()

    def minimumSizeHint(self) -> object:
        """Expose the narrow row floor and the current mode's natural height."""

        hint = super().minimumSizeHint()
        hint.setWidth(self._required_width(_HeaderControlsMode.NARROW, preferred=False))
        mode = self._mode or _HeaderControlsMode.NARROW
        hint.setHeight(max(hint.height(), self._required_height(mode)))
        return hint

    def _relayout(self, *, force: bool = False) -> None:
        if self._reflowing:
            return
        mode = self._mode_for_width()
        if not force and mode is self._mode:
            self._layout.invalidate()
            self._layout.activate()
            return

        self._reflowing = True
        try:
            for cluster in self._clusters:
                self._layout.removeWidget(cluster)
            for column in range(3):
                self._layout.setColumnStretch(column, 0)
            if mode is _HeaderControlsMode.REGULAR:
                self._place_regular_layout()
            elif mode is _HeaderControlsMode.COMPACT:
                self._place_compact_layout()
            else:
                self._place_narrow_layout()
            self._mode = mode
            self._layout.invalidate()
            self._layout.activate()
            self.updateGeometry()
        finally:
            self._reflowing = False

    def _mode_for_width(self) -> _HeaderControlsMode:
        """Choose the widest composition that fits the current content width."""

        available_width = max(0, self.contentsRect().width())
        if available_width >= self._required_width(_HeaderControlsMode.REGULAR):
            return _HeaderControlsMode.REGULAR
        if available_width >= self._required_width(_HeaderControlsMode.COMPACT):
            return _HeaderControlsMode.COMPACT
        return _HeaderControlsMode.NARROW

    def _required_width(
        self,
        mode: _HeaderControlsMode,
        *,
        preferred: bool = True,
    ) -> int:
        """Derive a mode floor from the clusters' own sizing contracts."""

        rows = self._rows_for_mode(mode)
        spacing = max(0, self._layout.horizontalSpacing())
        widest_row = max(
            sum(self._cluster_width(cluster, preferred=preferred) for cluster in row)
            + spacing * max(0, len(row) - 1)
            for row in rows
        )
        margins = self._layout.contentsMargins()
        return widest_row + margins.left() + margins.right()

    def _required_height(self, mode: _HeaderControlsMode) -> int:
        """Return the current mode's natural row height contract."""

        spacing = max(0, self._layout.verticalSpacing())
        row_height = sum(
            max(self._cluster_height(cluster) for cluster in row)
            for row in self._rows_for_mode(mode)
        )
        margins = self._layout.contentsMargins()
        return row_height + margins.top() + margins.bottom() + spacing * max(
            0,
            len(self._rows_for_mode(mode)) - 1,
        )

    def _rows_for_mode(
        self,
        mode: _HeaderControlsMode,
    ) -> tuple[tuple[QWidget, ...], ...]:
        status_cluster, motion_controls, theme_controls = self._clusters
        if mode is _HeaderControlsMode.REGULAR:
            return ((status_cluster, motion_controls, theme_controls),)
        if mode is _HeaderControlsMode.COMPACT:
            return ((status_cluster,), (motion_controls, theme_controls))
        return ((status_cluster,), (motion_controls,), (theme_controls,))

    def _place_regular_layout(self) -> None:
        for column, cluster in enumerate(self._clusters):
            self._layout.addWidget(cluster, 0, column)
        # Let the status summary keep its readable preferred width; the
        # motion/theme clusters remain at their own preferred widths.
        self._layout.setColumnStretch(0, 1)

    def _place_compact_layout(self) -> None:
        status_cluster, motion_controls, theme_controls = self._clusters
        self._layout.addWidget(status_cluster, 0, 0, 1, 2)
        self._layout.addWidget(motion_controls, 1, 0)
        self._layout.addWidget(theme_controls, 1, 1)
        self._layout.setColumnStretch(0, 1)
        self._layout.setColumnStretch(1, 1)

    def _place_narrow_layout(self) -> None:
        for row, cluster in enumerate(self._clusters):
            self._layout.addWidget(cluster, row, 0)
        self._layout.setColumnStretch(0, 1)

    @staticmethod
    def _cluster_width(cluster: QWidget, *, preferred: bool) -> int:
        width = max(cluster.minimumWidth(), cluster.minimumSizeHint().width())
        if preferred:
            width = max(width, cluster.sizeHint().width())
        return width

    @staticmethod
    def _cluster_height(cluster: QWidget) -> int:
        return max(
            cluster.minimumHeight(),
            cluster.minimumSizeHint().height(),
            cluster.sizeHint().height(),
        )


__all__ = ["ResponsiveHeaderControls"]
