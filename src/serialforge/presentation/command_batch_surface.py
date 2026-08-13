"""Presentation-only command-batch progress rail.

The rail is a decorative projection of the already validated command-batch
snapshot.  It keeps the accessible label text authoritative, owns no business
state, and consumes only the shared shell motion frame.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt
from .theme import theme_spec_for_widget

MAX_SURFACE_STEPS = 32


@dataclass(frozen=True, slots=True)
class CommandBatchSurfaceProjection:
    """Small presentation contract derived from one command-batch snapshot."""

    state: str = "empty"
    step_count: int = 0
    accepted_steps: int = 0
    current_step: int | None = None
    failed_step: int | None = None


class CommandBatchSurfaceLabel(QLabel):
    """Keep the existing status copy while adding a bounded step rail."""

    _STATES = frozenset({"empty", "ready", "running", "completed", "stopped", "failed"})
    _TERMINAL_STATES = frozenset({"completed", "stopped", "failed"})

    def __init__(self, parent: QLabel | None = None) -> None:
        super().__init__(parent)
        self._projection = CommandBatchSurfaceProjection()
        self._phase = 0.0
        self._animated = False
        self.setMinimumHeight(42)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_projection(self, projection: CommandBatchSurfaceProjection) -> None:
        """Consume a bounded visual projection without reading application state."""

        if not isinstance(projection, CommandBatchSurfaceProjection):
            return
        normalized = CommandBatchSurfaceProjection(
            state=projection.state if projection.state in self._STATES else "empty",
            step_count=_bounded_int(projection.step_count, 0, MAX_SURFACE_STEPS),
            accepted_steps=_bounded_int(projection.accepted_steps, 0, MAX_SURFACE_STEPS),
            current_step=_bounded_step(projection.current_step, projection.step_count),
            failed_step=_bounded_step(projection.failed_step, projection.step_count),
        )
        normalized = CommandBatchSurfaceProjection(
            state=normalized.state,
            step_count=normalized.step_count,
            accepted_steps=min(normalized.accepted_steps, normalized.step_count),
            current_step=(
                normalized.current_step
                if normalized.current_step is not None
                and normalized.current_step <= normalized.step_count
                else None
            ),
            failed_step=(
                normalized.failed_step
                if normalized.failed_step is not None
                and normalized.failed_step <= normalized.step_count
                else None
            ),
        )
        if self._projection == normalized:
            return
        self._projection = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume one shared presentation frame without creating a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the decorative pulse during reduced motion or suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        projection = self._projection
        if self.width() < 96 or self.height() < 30:
            return

        theme = theme_spec_for_widget(self)
        self._paint_status_marker(theme)
        if projection.step_count == 0:
            return

        bounds = QRectF(self.rect()).adjusted(10.0, 0.0, -10.0, -5.0)
        left = bounds.left()
        right = bounds.right()
        y = bounds.bottom() - 1.0
        span = max(1.0, right - left)
        slot = span / projection.step_count

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        baseline = QColor(theme.border)
        baseline.setAlpha(150)
        painter.setPen(QPen(baseline, 1.0))
        painter.drawLine(QPointF(left, y), QPointF(right, y))

        for index in range(projection.step_count):
            step = index + 1
            state = self._step_state(step)
            color, fill = self._step_colors(theme, state)
            segment = QRectF(
                left + index * slot + 1.0,
                y - 4.0,
                max(2.0, slot - 2.0),
                4.0,
            )
            painter.setPen(QPen(color, 1.0))
            painter.setBrush(fill)
            painter.drawRoundedRect(segment, 1.8, 1.8)

        if projection.state == "running" and self._animated:
            active_step = projection.current_step or min(
                projection.step_count,
                projection.accepted_steps + 1,
            )
            pulse_x = left + (active_step - 0.5) * slot
            wave = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(theme.warning)
            pulse.setAlpha(110 + int(wave * 100))
            painter.setPen(QPen(pulse, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            diameter = 3.0 + wave * 1.8
            painter.drawEllipse(
                QPointF(pulse_x, y - 2.0),
                diameter,
                diameter,
            )
        if projection.state in self._TERMINAL_STATES:
            marker_step = projection.failed_step
            if marker_step is None:
                marker_step = (
                    projection.step_count
                    if projection.state == "completed"
                    else max(1, projection.accepted_steps)
                )
            marker_step = min(max(1, marker_step), projection.step_count)
            marker_x = left + (marker_step - 0.5) * slot
            marker_color = {
                "completed": QColor(theme.success),
                "stopped": QColor(theme.warning),
                "failed": QColor(theme.error),
            }[projection.state]
            marker_color.setAlpha(225)
            painter.setPen(QPen(marker_color, 1.2))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(marker_x, y - 2.0), 4.0, 4.0)
            painter.setPen(QPen(marker_color, 1.0))
            if projection.state == "completed":
                painter.drawLine(
                    QPointF(marker_x - 2.0, y - 2.0),
                    QPointF(marker_x - 0.5, y - 0.5),
                )
                painter.drawLine(
                    QPointF(marker_x - 0.5, y - 0.5),
                    QPointF(marker_x + 2.5, y - 3.5),
                )
            elif projection.state == "failed":
                painter.drawLine(
                    QPointF(marker_x - 2.0, y - 4.0),
                    QPointF(marker_x + 2.0, y),
                )
                painter.drawLine(
                    QPointF(marker_x + 2.0, y - 4.0),
                    QPointF(marker_x - 2.0, y),
                )
            else:
                painter.drawLine(
                    QPointF(marker_x - 2.0, y - 2.0),
                    QPointF(marker_x + 2.0, y - 2.0),
                )

    def _paint_status_marker(self, theme) -> None:
        """Paint a compact state affordance from the existing projection."""

        color = self._status_color(theme, self._projection.state)
        color.setAlpha(220)
        center = QPointF(15.0, self.height() / 2.0)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(color)
        painter.drawEllipse(center, 3.5, 3.5)

        if self._projection.state == "running" and self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            halo = QColor(color)
            halo.setAlpha(32 + int(wave * 42))
            painter.setBrush(halo)
            radius = 6.0 + wave * 2.0
            painter.drawEllipse(center, radius, radius)

    @staticmethod
    def _status_color(theme, state: str) -> QColor:
        """Status color."""
        if state in {"ready", "completed"}:
            return QColor(theme.success)
        if state in {"running", "stopped"}:
            return QColor(theme.warning)
        if state == "failed":
            return QColor(theme.error)
        return QColor(theme.neutral_border)

    def _step_state(self, step: int) -> str:
        """Step state."""
        projection = self._projection
        if projection.failed_step == step:
            return "failed"
        if step <= projection.accepted_steps:
            return "accepted"
        if projection.state == "running" and projection.current_step == step:
            return "current"
        return "pending"

    @staticmethod
    def _step_colors(theme, state: str) -> tuple[QColor, QColor]:
        """Step colors."""
        if state == "accepted":
            border = QColor(theme.success_border)
            fill = QColor(theme.success_surface)
            fill.setAlpha(230)
            return border, fill
        if state == "current":
            border = QColor(theme.warning_border)
            fill = QColor(theme.warning_surface)
            fill.setAlpha(235)
            return border, fill
        if state == "failed":
            border = QColor(theme.error_border)
            fill = QColor(theme.error_surface)
            fill.setAlpha(240)
            return border, fill
        border = QColor(theme.border)
        border.setAlpha(150)
        fill = QColor(theme.surface_input)
        fill.setAlpha(190)
        return border, fill


def _bounded_int(value: object, minimum: int, maximum: int) -> int:
    """Bounded int."""
    if isinstance(value, bool):
        return minimum
    try:
        return max(minimum, min(maximum, int(value)))
    except (TypeError, ValueError, OverflowError):
        return minimum


def _bounded_step(value: object, step_count: object) -> int | None:
    """Bounded step."""
    if value is None or isinstance(value, bool):
        return None
    try:
        step = int(value)
        total = _bounded_int(step_count, 0, MAX_SURFACE_STEPS)
    except (TypeError, ValueError, OverflowError):
        return None
    return step if 1 <= step <= total else None


__all__ = ["CommandBatchSurfaceLabel", "CommandBatchSurfaceProjection"]
