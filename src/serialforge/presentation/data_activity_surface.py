"""Presentation-only receive-activity rail for the native terminal toolbar."""

from __future__ import annotations

import math
from dataclasses import dataclass

from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt
from .theme import theme_spec_for_widget

MAX_ACTIVITY_BYTES = 1_000_000


@dataclass(frozen=True, slots=True)
class DataActivityProjection:
    """Bounded receive facts used only to shape a decorative signal rail."""

    source: str = "实时"
    latest_bytes: int = 0
    window_bytes: int = 0

    def __post_init__(self) -> None:
        source = self.source if self.source in {"实时", "历史"} else "实时"
        object.__setattr__(self, "source", source)
        object.__setattr__(self, "latest_bytes", _bounded_int(self.latest_bytes))
        object.__setattr__(self, "window_bytes", _bounded_int(self.window_bytes))


class DataActivitySurface(QLabel):
    """Keep QLabel text authoritative while adding a bounded receive rail."""

    _BAR_COUNT = 12

    def __init__(self, parent: QLabel | None = None) -> None:
        super().__init__(parent)
        self._projection = DataActivityProjection()
        self._active = False
        self._phase = 0.0
        self._animated = False

    def set_projection(self, projection: DataActivityProjection) -> None:
        """Consume existing receive facts without becoming their state owner."""

        if not isinstance(projection, DataActivityProjection):
            return
        if self._projection == projection:
            return
        self._projection = projection
        self.update()

    def set_activity(self, active: bool) -> None:
        """Reflect lifecycle's existing short receive-activity projection."""

        normalized = bool(active)
        if self._active == normalized:
            return
        self._active = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the receive rail during lifecycle suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if self.width() < 120 or self.height() < 18:
            return

        projection = self._projection
        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(8.0, 3.0, -8.0, -4.0)
        left = bounds.left()
        right = bounds.right()
        baseline = bounds.bottom()
        span = max(1.0, right - left)
        gap = span / max(1, self._BAR_COUNT - 1)
        color = QColor(theme.accent_purple if projection.source == "历史" else theme.accent)
        if not self._active:
            color = QColor(theme.text_subtle)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        guide = QColor(theme.border)
        guide.setAlpha(125)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, baseline), QPointF(right, baseline))

        seed = projection.latest_bytes + projection.window_bytes
        for index in range(self._BAR_COUNT):
            height = 2.0 + ((seed + index * 13) % 4) * 1.25
            bar = QColor(color)
            bar.setAlpha(70 + min(125, index * 5) if self._active else 80)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(bar)
            x = left + index * gap
            painter.drawRoundedRect(
                QRectF(x - 1.2, baseline - height, 2.4, height),
                1.0,
                1.0,
            )

        if self._active and self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = left + span * travel
            pulse = QColor(theme.accent_blue)
            pulse.setAlpha(150 + int(((math.sin(self._phase) + 1.0) * 0.5) * 80))
            painter.setPen(QPen(pulse, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, baseline - 2.0), 3.0, 3.0)


def _bounded_int(value: object) -> int:
    """Bounded int."""
    if isinstance(value, bool):
        return 0
    try:
        return max(0, min(MAX_ACTIVITY_BYTES, int(value)))
    except (TypeError, ValueError, OverflowError):
        return 0


__all__ = ["DataActivityProjection", "DataActivitySurface"]
