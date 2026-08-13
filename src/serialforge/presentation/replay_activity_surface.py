"""Presentation-only historical replay activity rail."""

from __future__ import annotations

import math
from dataclasses import dataclass

from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt
from .theme import theme_spec_for_widget


@dataclass(frozen=True, slots=True)
class ReplayActivityProjection:
    """Observable replay facts used only to shape a decorative activity trace."""

    state: str = "idle"
    records_emitted: int = 0


class ReplayActivityLabel(QLabel):
    """Keep replay copy authoritative while rendering a non-percent activity trace."""

    _STATES = frozenset({"idle", "active", "paused", "history", "error"})
    _TERMINAL_STATES = frozenset({"history", "error"})
    _TRACE_POINTS = 14

    def __init__(self, parent: QLabel | None = None) -> None:
        super().__init__(parent)
        self._projection = ReplayActivityProjection()
        self._phase = 0.0
        self._animated = False
        self.setMinimumHeight(40)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_projection(self, projection: ReplayActivityProjection) -> None:
        """Consume replay counters without inventing a total or a percentage."""

        if not isinstance(projection, ReplayActivityProjection):
            return
        normalized = ReplayActivityProjection(
            state=projection.state if projection.state in self._STATES else "idle",
            records_emitted=max(0, _safe_int(projection.records_emitted)),
        )
        if self._projection == normalized:
            return
        self._projection = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without owning a clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the activity spark during reduced motion or suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if self.width() < 96 or self.height() < 28:
            return

        projection = self._projection
        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(10.0, 0.0, -10.0, -5.0)
        left = bounds.left()
        right = bounds.right()
        baseline_y = bounds.bottom() - 1.0
        span = max(1.0, right - left)
        point_gap = span / (self._TRACE_POINTS - 1)
        color = self._state_color(theme, projection.state)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        base = QColor(theme.border)
        base.setAlpha(145)
        painter.setPen(QPen(base, 1.0))
        painter.drawLine(
            QPointF(left, baseline_y),
            QPointF(right, baseline_y),
        )

        emitted = projection.records_emitted
        for index in range(self._TRACE_POINTS):
            x = left + index * point_gap
            if emitted == 0:
                height = 2.0
            else:
                height = 2.0 + ((emitted + index * 5) % 5) * 1.1
            tick = QColor(color)
            tick.setAlpha(70 + min(120, index * 4))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(tick)
            painter.drawRoundedRect(
                QRectF(x - 1.2, baseline_y - height, 2.4, height),
                1.0,
                1.0,
            )

        if projection.state == "active" and self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            spark_x = left + span * travel
            spark = QColor(color)
            spark.setAlpha(135 + int(((math.sin(self._phase) + 1.0) * 0.5) * 90))
            painter.setPen(QPen(spark, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(spark_x, baseline_y - 2.0), 3.0, 3.0)
        elif emitted > 0:
            tail_x = left + (emitted % self._TRACE_POINTS) * point_gap
            tail = QColor(color)
            tail.setAlpha(205)
            painter.setPen(QPen(tail, 1.0))
            painter.drawLine(
                QPointF(tail_x, baseline_y - 6.0),
                QPointF(tail_x, baseline_y + 1.0),
            )
        if projection.state in self._TERMINAL_STATES:
            marker_index = emitted % self._TRACE_POINTS if emitted > 0 else 0
            marker_x = left + marker_index * point_gap
            marker = QColor(color)
            marker.setAlpha(225)
            painter.setPen(QPen(marker, 1.2))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(marker_x, baseline_y - 2.0), 4.0, 4.0)
            painter.setPen(QPen(marker, 1.0))
            painter.drawLine(
                QPointF(marker_x - 2.0, baseline_y - 2.0),
                QPointF(marker_x + 2.0, baseline_y - 2.0),
            )
            if projection.state == "error":
                painter.drawLine(
                    QPointF(marker_x, baseline_y - 4.0),
                    QPointF(marker_x, baseline_y),
                )

    @staticmethod
    def _state_color(theme, state: str) -> QColor:
        """State color."""
        values = {
            "idle": theme.text_subtle,
            "active": theme.accent,
            "paused": theme.warning,
            "history": theme.accent_purple,
            "error": theme.error,
        }
        color = QColor(values[state])
        color.setAlpha(220)
        return color


def _safe_int(value: object) -> int:
    """Safe int."""
    if isinstance(value, bool):
        return 0
    try:
        return int(value)
    except (TypeError, ValueError, OverflowError):
        return 0


__all__ = ["ReplayActivityLabel", "ReplayActivityProjection"]
