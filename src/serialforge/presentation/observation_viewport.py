"""Shared resource-free observation ruler for text preview widgets."""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPlainTextEdit, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class ObservationViewport(QPlainTextEdit):
    """Keep native text editing/scrolling while adding a quiet signal ruler."""

    _SCOPES = frozenset({"terminal", "component", "dataset"})

    def __init__(self, parent: QWidget | None = None, *, scope: str = "terminal") -> None:
        super().__init__(parent)
        self._scope = scope if scope in self._SCOPES else "terminal"
        self._phase = 0.0
        self._animated = False

    def set_scope(self, scope: object) -> None:
        """Select a visual accent without introducing data or state semantics."""

        normalized = str(scope).lower()
        if normalized not in self._SCOPES:
            normalized = "terminal"
        if self._scope == normalized:
            return
        self._scope = normalized
        self.viewport().update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a local clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.viewport().update()

    def stop(self) -> None:
        """Freeze decoration during reduced-motion or lifecycle suspension."""

        self._animated = False
        self.viewport().update()

    def paintEvent(self, event: object) -> None:
        """Paint only inside the existing viewport padding after native text."""

        super().paintEvent(event)
        viewport = self.viewport()
        if viewport.width() < 120 or viewport.height() < 28:
            return

        bounds = QRectF(viewport.rect()).adjusted(6.0, 4.0, -6.0, -4.0)
        if bounds.width() <= 0 or bounds.height() <= 0:
            return

        theme = theme_spec_for_widget(self)
        accent = QColor(self._accent(theme))
        painter = QPainter(viewport)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setClipRect(bounds)

        guide = QColor(theme.border)
        guide.setAlpha(72)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(
            QPointF(bounds.left(), bounds.top()),
            QPointF(bounds.right(), bounds.top()),
        )
        painter.drawLine(
            QPointF(bounds.left(), bounds.bottom()),
            QPointF(bounds.right(), bounds.bottom()),
        )

        bracket = QColor(accent)
        bracket.setAlpha(88 if self._animated else 54)
        painter.setPen(QPen(bracket, 1.0))
        corner = 8.0
        for left, top, horizontal, vertical in (
            (bounds.left(), bounds.top(), 1.0, 1.0),
            (bounds.right(), bounds.top(), -1.0, 1.0),
            (bounds.left(), bounds.bottom(), 1.0, -1.0),
            (bounds.right(), bounds.bottom(), -1.0, -1.0),
        ):
            painter.drawLine(
                QPointF(left, top),
                QPointF(left + horizontal * corner, top),
            )
            painter.drawLine(
                QPointF(left, top),
                QPointF(left, top + vertical * corner),
            )

        self._draw_signal(painter, bounds, accent)
        if self._scope != "terminal":
            self._draw_scope_nodes(painter, bounds, accent, theme.surface_input)

    def _draw_signal(self, painter: QPainter, bounds: QRectF, accent: QColor) -> None:
        if self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            center_x = bounds.left() + bounds.width() * travel
            accent.setAlpha(145)
            painter.setPen(QPen(accent, 1.2))
            painter.drawLine(
                QPointF(max(bounds.left(), center_x - 18.0), bounds.top()),
                QPointF(min(bounds.right(), center_x + 18.0), bounds.top()),
            )
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(accent)
            painter.drawEllipse(QPointF(center_x, bounds.top()), 2.0, 2.0)
            return

        accent.setAlpha(70)
        painter.setPen(QPen(accent, 1.0))
        painter.drawLine(
            QPointF(bounds.left() + 12.0, bounds.top()),
            QPointF(bounds.left() + 34.0, bounds.top()),
        )

    def _draw_scope_nodes(
        self,
        painter: QPainter,
        bounds: QRectF,
        accent: QColor,
        surface_input: str,
    ) -> None:
        count = 3 if self._scope == "component" else 5
        rail = QColor(accent)
        rail.setAlpha(110 if self._animated else 58)
        rail_y = bounds.bottom()
        painter.setPen(QPen(rail, 1.0))
        painter.drawLine(
            QPointF(bounds.left() + 12.0, rail_y),
            QPointF(bounds.right() - 12.0, rail_y),
        )
        for index in range(count):
            travel = index / max(1, count - 1)
            node = QColor(accent)
            node.setAlpha(190 if self._animated else 92)
            x = bounds.left() + 12.0 + (bounds.width() - 24.0) * travel
            painter.setPen(QPen(node, 1.0))
            painter.setBrush(QColor(surface_input))
            painter.drawEllipse(QPointF(x, rail_y), 2.4, 2.4)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(node)
            painter.drawEllipse(QPointF(x, rail_y), 0.9, 0.9)

    def _accent(self, theme):
        return {
            "terminal": theme.accent_blue,
            "component": theme.accent_purple,
            "dataset": theme.accent,
        }[self._scope]


__all__ = ["ObservationViewport"]
