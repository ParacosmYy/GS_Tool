"""Presentation-only connection path rail.

The rail gives the connection band a readable visual path without becoming a
second status source.  Its state is explicitly supplied by the lifecycle
controller and its animation is driven by the shared shell frame.
"""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, QSizePolicy, Qt, QWidget
from .theme import theme_spec_for_widget


class ConnectionStatusRail(QWidget):
    """Draw the decorative endpoint → transport → session → data path."""

    _STATES = frozenset({"discovered", "opening", "open", "closing", "closed", "error"})
    _ACTIVE_NODES = {
        "closed": 0,
        "discovered": 1,
        "opening": 2,
        "open": 4,
        "closing": 3,
        "error": 1,
    }

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._state = "closed"
        self._phase = 0.0
        self._animated = False
        self.setMinimumHeight(28)
        self.setMaximumHeight(32)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_state(self, state: str) -> None:
        """Accept the existing presentation state without owning its source."""

        normalized = state if state in self._STATES else "closed"
        if self._state == normalized:
            return
        self._state = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared frame; this widget never creates a clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Keep a deterministic static path during lifecycle suspension."""

        self._animated = False
        self.update()

    def motion_active(self) -> bool:
        """Return whether the current connection path has a moving pulse."""

        return self._state in {"opening", "open", "closing"}

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        if self.width() < 120 or self.height() < 20:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(12.0, 4.0, -12.0, -4.0)
        left = bounds.left()
        right = bounds.right()
        y = bounds.center().y()
        span = max(1.0, right - left)
        positions = tuple(left + span * index / 3.0 for index in range(4))
        active_nodes = self._ACTIVE_NODES[self._state]
        state_color = self._state_color(theme)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        guide = QColor(theme.border)
        guide.setAlpha(105)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, y), QPointF(right, y))

        for index in range(max(0, active_nodes - 1)):
            active_line = QColor(state_color)
            active_line.setAlpha(175)
            painter.setPen(QPen(active_line, 2.0))
            painter.drawLine(QPointF(positions[index], y), QPointF(positions[index + 1], y))

        for index, x in enumerate(positions):
            node_color = QColor(state_color if index < active_nodes else theme.surface_input)
            node_color.setAlpha(220 if index < active_nodes else 185)
            painter.setPen(QPen(state_color if index < active_nodes else guide, 1.0))
            painter.setBrush(node_color)
            painter.drawEllipse(QPointF(x, y), 4.2, 4.2)
            if index < active_nodes:
                core = QColor(theme.text)
                core.setAlpha(220)
                painter.setPen(Qt.PenStyle.NoPen)
                painter.setBrush(core)
                painter.drawEllipse(QPointF(x, y), 1.25, 1.25)

        self._paint_terminal_marker(painter, theme, positions, y)

        if self._animated and active_nodes > 1 and self._state not in {"error", "closed"}:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            segment = min(2.999, travel * (active_nodes - 1))
            index = int(segment)
            fraction = segment - index
            pulse_x = positions[index] + (positions[index + 1] - positions[index]) * fraction

            for trail_step, alpha, radius in ((1, 105, 2.3), (2, 68, 1.6), (3, 36, 1.0)):
                trail_position = max(0.0, segment - trail_step * 0.18)
                trail_index = min(active_nodes - 2, int(trail_position))
                trail_fraction = trail_position - trail_index
                trail_x = positions[trail_index] + (
                    positions[trail_index + 1] - positions[trail_index]
                ) * trail_fraction
                trail = QColor(state_color)
                trail.setAlpha(alpha)
                painter.setPen(Qt.PenStyle.NoPen)
                painter.setBrush(trail)
                painter.drawEllipse(QPointF(trail_x, y), radius, radius)

            pulse = QColor(state_color)
            pulse.setAlpha(190)
            painter.setPen(QPen(pulse, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, y), 7.0, 7.0)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(pulse)
            painter.drawEllipse(QPointF(pulse_x, y), 1.7, 1.7)

    def _paint_terminal_marker(
        self,
        painter: QPainter,
        theme,
        positions: tuple[float, ...],
        y: float,
    ) -> None:
        """Add a non-color-only marker for the existing terminal session states."""

        if self._state not in {"open", "error"}:
            return

        marker = QColor(theme.success if self._state == "open" else theme.error)
        marker.setAlpha(235)
        marker_x = positions[-1] if self._state == "open" else positions[0]
        center = QPointF(marker_x, y)
        painter.setPen(QPen(marker, 1.2))
        painter.setBrush(QColor(theme.surface_input))
        painter.drawEllipse(center, 6.5, 6.5)
        painter.setPen(QPen(marker, 1.25))
        if self._state == "open":
            painter.drawLine(
                QPointF(marker_x - 2.8, y),
                QPointF(marker_x - 0.8, y + 2.0),
            )
            painter.drawLine(
                QPointF(marker_x - 0.8, y + 2.0),
                QPointF(marker_x + 3.2, y - 2.5),
            )
        else:
            painter.drawLine(
                QPointF(marker_x - 2.5, y - 2.5),
                QPointF(marker_x + 2.5, y + 2.5),
            )
            painter.drawLine(
                QPointF(marker_x + 2.5, y - 2.5),
                QPointF(marker_x - 2.5, y + 2.5),
            )

    def _state_color(self, theme) -> QColor:
        """State color."""
        if self._state == "error":
            return QColor(theme.error)
        if self._state in {"opening", "closing"}:
            return QColor(theme.warning)
        if self._state == "discovered":
            return QColor(theme.accent_blue)
        if self._state == "open":
            return QColor(theme.success_border)
        return QColor(theme.accent_purple)


__all__ = ["ConnectionStatusRail"]
