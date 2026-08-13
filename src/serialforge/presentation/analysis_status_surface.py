"""Presentation-only status rail for protocol-derived analysis surfaces.

The label keeps its text and accessibility contract in the owning controller.
It reads only the already-projected ``state`` and ``source`` properties, then
adds a small anime-inspired signal rail driven by the shell motion frame.
"""

from __future__ import annotations

import math

from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class AnalysisStatusLabel(QLabel):
    """Render a decorative signal rail without owning analysis state."""

    _NODE_COUNT = 5
    _KNOWN_STATES = frozenset(
        {"active", "waiting", "empty", "error", "blocked", "draft", "history", "idle"}
    )
    _MOVING_STATES = frozenset({"active", "waiting", "draft"})
    _STATIC_MARKER_STATES = frozenset({"error", "blocked", "history"})

    def __init__(self, text: str = "", parent: QWidget | None = None) -> None:
        super().__init__(text, parent)
        self._phase = 0.0
        self._animated = False
        self.setContentsMargins(0, 0, 0, 12)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume one shared shell frame without creating a local timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the rail while reduced motion or lifecycle suspension is active."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        super().paintEvent(event)
        if self.width() < 96 or self.height() < 18:
            return

        state = self._normalized_property("state", "idle", self._KNOWN_STATES)
        source = self._normalized_property("source", "live", frozenset({"live", "history"}))
        theme = theme_spec_for_widget(self)
        content_rect = QRectF(self.contentsRect())
        self._paint_state_marker(theme, state, source, content_rect)
        band_top = content_rect.bottom() + 1.0
        band_bottom = float(self.rect().bottom()) - 1.0
        band_height = band_bottom - band_top
        if band_height < 5.0:
            return
        bounds = QRectF(content_rect.left(), band_top, content_rect.width(), band_height)
        bounds = bounds.adjusted(10.0, 0.0, -10.0, 0.0)
        left = bounds.left()
        right = bounds.right()
        baseline_y = bounds.center().y()
        span = max(1.0, right - left)
        color = self._state_color(theme, state, source)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        guide = QColor(theme.border)
        guide.setAlpha(110)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, baseline_y), QPointF(right, baseline_y))

        active_nodes = self._active_node_count(state)
        gap = span / (self._NODE_COUNT - 1)
        for index in range(self._NODE_COUNT):
            x = left + gap * index
            node = QColor(color)
            node.setAlpha(60 if index >= active_nodes else 185)
            painter.setPen(QPen(node, 1.0))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(x, baseline_y), 2.5, 2.5)
            if index < active_nodes:
                painter.setPen(Qt.PenStyle.NoPen)
                painter.setBrush(node)
                painter.drawEllipse(QPointF(x, baseline_y), 1.15, 1.15)

        if self._animated and state in self._MOVING_STATES:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            spark_x = left + span * travel
            spark = QColor(color)
            spark.setAlpha(145 + int(travel * 80))
            painter.setPen(QPen(spark, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(spark_x, baseline_y), 4.2, 4.2)
        if state in self._STATIC_MARKER_STATES:
            marker_index = max(0, min(self._NODE_COUNT - 1, active_nodes - 1))
            marker_x = left + marker_index * gap
            marker = QColor(color)
            marker.setAlpha(225)
            painter.setPen(QPen(marker, 1.2))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(marker_x, baseline_y), 4.0, 4.0)
            painter.setPen(QPen(marker, 1.0))
            if state == "error":
                painter.drawLine(
                    QPointF(marker_x - 2.0, baseline_y - 2.0),
                    QPointF(marker_x + 2.0, baseline_y + 2.0),
                )
                painter.drawLine(
                    QPointF(marker_x + 2.0, baseline_y - 2.0),
                    QPointF(marker_x - 2.0, baseline_y + 2.0),
                )
            elif state == "blocked":
                painter.drawLine(
                    QPointF(marker_x - 2.0, baseline_y - 2.0),
                    QPointF(marker_x + 2.0, baseline_y - 2.0),
                )
                painter.drawLine(
                    QPointF(marker_x - 2.0, baseline_y + 2.0),
                    QPointF(marker_x + 2.0, baseline_y + 2.0),
                )
            else:
                painter.drawLine(
                    QPointF(marker_x + 2.0, baseline_y - 2.0),
                    QPointF(marker_x - 1.0, baseline_y),
                )
                painter.drawLine(
                    QPointF(marker_x - 1.0, baseline_y),
                    QPointF(marker_x + 2.0, baseline_y + 2.0),
                )

    def _paint_state_marker(
        self,
        theme,
        state: str,
        source: str,
        content_rect: QRectF,
    ) -> None:
        """Paint a compact state affordance without owning analysis state."""

        marker = self._state_color(theme, state, source)
        marker.setAlpha(225)
        center = QPointF(11.0, content_rect.center().y())
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        if self._animated and state in self._MOVING_STATES:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            halo = QColor(marker)
            halo.setAlpha(26 + int(wave * 36))
            painter.setPen(QPen(halo, 1.0))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            radius = 5.4 + wave * 1.6
            painter.drawEllipse(center, radius, radius)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(marker)
        painter.drawEllipse(center, 3.0, 3.0)

    @classmethod
    def _active_node_count(cls, state: str) -> int:
        return {
            "active": cls._NODE_COUNT,
            "waiting": 3,
            "draft": 2,
            "history": 4,
            "error": 1,
            "blocked": 1,
            "empty": 1,
            "idle": 1,
        }.get(state, 1)

    def _normalized_property(
        self,
        name: str,
        fallback: str,
        allowed: frozenset[str],
    ) -> str:
        value = self.property(name)
        return value if isinstance(value, str) and value in allowed else fallback

    def _state_color(self, theme, state: str, source: str) -> QColor:
        if source == "history" and state not in {"error", "blocked"}:
            value = theme.accent_purple
        else:
            value = {
                "active": theme.success,
                "waiting": theme.accent_blue,
                "draft": theme.accent_purple,
                "history": theme.accent_purple,
                "error": theme.error,
                "blocked": theme.text_subtle,
                "empty": theme.text_subtle,
                "idle": theme.text_subtle,
            }.get(state, theme.text_subtle)
        color = QColor(value)
        color.setAlpha(210)
        return color


__all__ = ["AnalysisStatusLabel"]
