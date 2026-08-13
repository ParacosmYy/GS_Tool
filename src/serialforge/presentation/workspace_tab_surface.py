"""Animated presentation surface for the native workspace tab bar."""

from __future__ import annotations

import math

from .qt import (
    QColor,
    QEvent,
    QPainter,
    QPen,
    QPointF,
    QRectF,
    QSizePolicy,
    Qt,
    QTabBar,
    QWidget,
)
from .theme import theme_spec_for_widget


class AnimatedWorkspaceTabBar(QTabBar):
    """Keep native navigation while projecting labels to the available width.

    The tab bar owns only the presentation projection of workspace labels.  A
    full label remains the accessible name and tooltip; visible text may use a
    shorter or icon-only form when the native tab bar cannot fit every item.
    The QTabWidget, tab indexes, icons, signals and focus order remain native.
    """

    _TAB_ACCENTS = ("accent_blue", "accent", "accent_pink", "accent_purple")

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self._tab_label_contract: tuple[tuple[str, str], ...] = ()
        self._tab_label_mode = "full"
        self._applying_tab_labels = False
        self.setDrawBase(False)
        self.setElideMode(Qt.TextElideMode.ElideNone)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        parent_widget = self.parentWidget()
        if parent_widget is not None:
            parent_widget.installEventFilter(self)

    def set_workspace_tab_contract(
        self,
        labels: tuple[tuple[str, str], ...],
    ) -> None:
        """Inject full and compact labels without taking ownership of routes."""

        if len(labels) != self.count():
            raise ValueError("workspace tab contract must match the tab count")
        self._tab_label_contract = tuple(
            (str(full), str(compact)) for full, compact in labels
        )
        for index, (full, _compact) in enumerate(self._tab_label_contract):
            self.setAccessibleTabName(index, full)
            self.setTabToolTip(index, full)
        self._apply_tab_label_mode()

    def tab_label_mode(self) -> str:
        """Expose the current visual density for diagnostics and UI audits."""

        return self._tab_label_mode

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._apply_tab_label_mode()

    def changeEvent(self, event: object) -> None:
        """Changeevent."""
        super().changeEvent(event)  # type: ignore[arg-type]
        event_type = event.type() if hasattr(event, "type") else None
        if event_type in (QEvent.Type.FontChange, QEvent.Type.StyleChange):
            self._apply_tab_label_mode()

    def event(self, event: object) -> bool:
        """Event."""
        handled = super().event(event)  # type: ignore[arg-type]
        event_type = event.type() if hasattr(event, "type") else None
        if event_type == QEvent.Type.LayoutRequest:
            self._apply_tab_label_mode()
        return handled

    def eventFilter(self, watched: object, event: object) -> bool:
        """Re-evaluate labels when the owning tab widget offers new space."""

        parent_widget = self.parentWidget()
        event_type = event.type() if hasattr(event, "type") else None
        if watched is parent_widget and event_type in (
            QEvent.Type.FontChange,
            QEvent.Type.LayoutRequest,
            QEvent.Type.Resize,
            QEvent.Type.Show,
            QEvent.Type.StyleChange,
        ):
            self._apply_tab_label_mode()
        return False

    def _apply_tab_label_mode(self) -> None:
        """Choose the densest readable mode that fits the native tab bar."""

        if self._applying_tab_labels or not self._tab_label_contract:
            return
        if self.width() <= 0:
            return

        self._applying_tab_labels = True
        try:
            candidates = (
                ("full", tuple(full for full, _compact in self._tab_label_contract)),
                (
                    "compact",
                    tuple(compact for _full, compact in self._tab_label_contract),
                ),
                ("icon", ("",) * len(self._tab_label_contract)),
            )
            selected_mode, selected_labels = candidates[-1]
            for mode, labels in candidates:
                if self._labels_fit(labels):
                    selected_mode, selected_labels = mode, labels
                    break
            self._set_visible_labels(selected_mode, selected_labels)
        finally:
            self._applying_tab_labels = False

    def _labels_fit(self, labels: tuple[str, ...]) -> bool:
        """Measure candidate labels through the native tab sizing contract."""

        current = tuple(self.tabText(index) for index in range(self.count()))
        try:
            self._set_texts(labels)
            widths = sum(self.tabSizeHint(index).width() for index in range(self.count()))
            return widths <= self._offered_width()
        finally:
            self._set_texts(current)

    def _offered_width(self) -> int:
        """Read width from the owning tab widget, not projected tab geometry."""

        parent_widget = self.parentWidget()
        if parent_widget is None:
            return self.width()
        margins = self.contentsMargins()
        return max(
            0,
            parent_widget.contentsRect().width()
            - margins.left()
            - margins.right(),
        )

    def _set_visible_labels(self, mode: str, labels: tuple[str, ...]) -> None:
        """Set visible labels."""
        if self._tab_label_mode == mode and tuple(
            self.tabText(index) for index in range(self.count())
        ) == labels:
            return
        self._tab_label_mode = mode
        self._set_texts(labels)
        self.style().unpolish(self)
        self.style().polish(self)
        self.updateGeometry()

    def _set_texts(self, labels: tuple[str, ...]) -> None:
        """Set texts."""
        for index, label in enumerate(labels):
            self.setTabText(index, label)

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shell frame without creating a timer or state source."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the underline while preserving the native selected tab."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        index = self.currentIndex()
        if index < 0 or index >= self.count():
            return

        tab = self.tabRect(index)
        if tab.width() < 28 or tab.height() < 18:
            return
        bounds = QRectF(tab).adjusted(10.0, 0.0, -10.0, -2.0)
        if bounds.width() <= 0 or bounds.height() <= 0:
            return

        theme = theme_spec_for_widget(self)
        accent_name = self._TAB_ACCENTS[min(index, len(self._TAB_ACCENTS) - 1)]
        accent = QColor(getattr(theme, accent_name))
        baseline = bounds.bottom()

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setClipRect(tab)

        guide = QColor(theme.border)
        guide.setAlpha(150)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(
            QPointF(bounds.left(), baseline),
            QPointF(bounds.right(), baseline),
        )

        if self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            center_x = bounds.left() + bounds.width() * travel
            accent.setAlpha(210)
            painter.setPen(QPen(accent, 1.6))
            painter.drawLine(
                QPointF(max(bounds.left(), center_x - 16.0), baseline),
                QPointF(min(bounds.right(), center_x + 16.0), baseline),
            )
            halo = QColor(accent)
            halo.setAlpha(42)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(halo)
            painter.drawEllipse(QPointF(center_x, baseline), 5.0, 5.0)
            painter.setBrush(accent)
            painter.drawEllipse(QPointF(center_x, baseline), 1.8, 1.8)
        else:
            accent.setAlpha(145)
            painter.setPen(QPen(accent, 1.4))
            painter.drawLine(
                QPointF(bounds.left() + 8.0, baseline),
                QPointF(bounds.right() - 8.0, baseline),
            )


__all__ = ["AnimatedWorkspaceTabBar"]
