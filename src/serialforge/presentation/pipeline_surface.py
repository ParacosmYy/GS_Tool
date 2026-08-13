"""Presentation-only pipeline rail for the protocol summary surface.

The label keeps the existing accessible text and dynamic QSS properties.  The
rail is a decorative layer drawn inside the label's safe padding; it consumes
the shared motion frame supplied by the shell and never owns a clock or reads
application state.
"""

from __future__ import annotations

import math

from .qt import QColor, QLabel, QPainter, QPen, QPointF, Qt
from .theme import theme_spec_for_widget


class PipelineSurfaceLabel(QLabel):
    """Keep the pipeline summary readable while adding a subtle flow rail."""

    _NODE_COUNT = 4
    _KNOWN_STATES = frozenset({"idle", "active", "transition", "draft", "blocked", "history"})
    _MOVING_STATES = frozenset({"active", "transition", "draft"})

    def __init__(self, parent: QLabel | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self.setMinimumHeight(34)

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume one shared presentation frame without owning a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the rail while reduced motion or lifecycle suspension is active."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        super().paintEvent(event)
        if self.width() < 96 or self.height() < 24:
            return

        theme = theme_spec_for_widget(self)
        state = self._normalized_property("state", "idle", self._KNOWN_STATES)
        source = self._normalized_property("source", "live", frozenset({"live", "history"}))
        bounds = self.rect().adjusted(12, 0, -12, -4)
        y = float(bounds.bottom())
        left = float(bounds.left())
        right = float(bounds.right())
        span = max(1.0, right - left)
        accent = self._state_color(theme, state, source)
        node_palette = self._node_palette(theme, state, source)
        active_nodes = self._active_node_count(state)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setPen(QPen(self._with_alpha(accent, 100), 1.0))
        painter.drawLine(QPointF(left, y), QPointF(right, y))

        node_positions = tuple(
            left + span * index / (self._NODE_COUNT - 1)
            for index in range(self._NODE_COUNT)
        )
        for index, x in enumerate(node_positions):
            node_color = QColor(node_palette[index])
            node_color.setAlpha(185 if index < active_nodes else 55)
            painter.setPen(QPen(node_color, 1.0))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(x, y), 3.2, 3.2)
            if index < active_nodes:
                painter.setBrush(node_color)
                painter.drawEllipse(QPointF(x, y), 1.2, 1.2)

        if self._animated and state in self._MOVING_STATES:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = left + span * travel
            pulse = QColor(accent)
            pulse.setAlpha(210)
            painter.setPen(QPen(pulse, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, y), 4.8, 4.8)

        if state == "blocked":
            marker = QColor(accent)
            marker.setAlpha(220)
            marker_x = node_positions[0]
            painter.setPen(QPen(marker, 1.1))
            painter.drawLine(QPointF(marker_x - 2.0, y - 2.0), QPointF(marker_x + 2.0, y + 2.0))
            painter.drawLine(QPointF(marker_x + 2.0, y - 2.0), QPointF(marker_x - 2.0, y + 2.0))

    @classmethod
    def _active_node_count(cls, state: str) -> int:
        return {
            "active": cls._NODE_COUNT,
            "transition": 3,
            "draft": 2,
            "history": cls._NODE_COUNT,
            "blocked": 1,
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

    @staticmethod
    def _state_color(theme, state: str, source: str) -> QColor:
        if source == "history" and state not in {"blocked"}:
            value = theme.accent_purple
        else:
            value = {
                "active": theme.accent,
                "transition": theme.accent_blue,
                "draft": theme.accent_purple,
                "blocked": theme.text_subtle,
                "history": theme.accent_purple,
                "idle": theme.text_subtle,
            }.get(state, theme.text_subtle)
        color = QColor(value)
        color.setAlpha(210)
        return color

    @staticmethod
    def _node_palette(theme, state: str, source: str) -> tuple[str, str, str, str]:
        if state == "blocked":
            return (theme.text_subtle,) * 4
        if source == "history" or state == "history":
            return (theme.accent_purple,) * 4
        if state == "draft":
            return (theme.accent_purple, theme.accent_purple, theme.border, theme.border)
        if state == "transition":
            return (theme.accent_blue, theme.accent_blue, theme.accent, theme.accent)
        if state == "active":
            return (theme.accent, theme.accent_blue, theme.accent_purple, theme.accent_pink)
        return (theme.text_subtle,) * 4

    @staticmethod
    def _with_alpha(color: QColor, alpha: int) -> QColor:
        color.setAlpha(alpha)
        return color


__all__ = ["PipelineSurfaceLabel"]
