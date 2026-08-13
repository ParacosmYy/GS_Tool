"""Shared presentation-only action surfaces.

The controls in this module remain native Qt buttons.  They only decorate the
existing paint result with a small signal rail driven by the shell's shared
MotionController frame; they do not own timers, application state, or actions.
"""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QPushButton, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget

ACTION_RAIL_MIN_HEIGHT = 36


def _paint_signal_rail(
    button: QPushButton,
    phase: float,
    animated: bool,
    color: str,
    *,
    animated_alpha: int,
    static_alpha: int,
    half_span: float,
    radius: float,
    pen_width: float,
) -> None:
    """Render one shared rail geometry for native button surfaces."""

    if not button.isVisible() or button.width() < 52 or button.height() < 16:
        return
    bounds = QRectF(button.rect()).adjusted(8.0, 0.0, -8.0, -3.0)
    if bounds.width() <= 0 or bounds.height() <= 0:
        return

    accent = QColor(color)
    painter = QPainter(button)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
    painter.setClipRect(bounds)
    y = bounds.bottom()
    if animated:
        travel = (math.sin(phase) + 1.0) * 0.5
        center_x = bounds.left() + bounds.width() * travel
        accent.setAlpha(animated_alpha)
        painter.setPen(QPen(accent, pen_width))
        painter.drawLine(
            QPointF(max(bounds.left(), center_x - half_span), y),
            QPointF(min(bounds.right(), center_x + half_span), y),
        )
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(accent)
        painter.drawEllipse(QPointF(center_x, y), radius, radius)
    else:
        accent.setAlpha(static_alpha)
        painter.setPen(QPen(accent, 1.0))
        painter.drawLine(QPointF(bounds.left(), y), QPointF(bounds.right(), y))


class ActionRailButton(QPushButton):
    """Keep a native action button visually connected to the shared signal field."""

    MOTION_MODE = "activity"

    def __init__(self, text: str, parent: QWidget | None = None) -> None:
        super().__init__(text, parent)
        # Keep the action contract in one place.  The shared stylesheet and
        # the active system font can produce a 36px content minimum; callers
        # must not replace it with a smaller fixed maximum height.
        self.setMinimumHeight(ACTION_RAIL_MIN_HEIGHT)
        self._phase = 0.0
        self._animated = False

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume one shell frame without creating a timer or business state."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the rail when the shared motion owner enters a static mode."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        _paint_signal_rail(
            self,
            self._phase,
            self._animated,
            theme_spec_for_widget(self).accent,
            animated_alpha=168,
            static_alpha=72,
            half_span=14.0,
            radius=1.7,
            pen_width=1.1,
        )


class BusyActionButton(QPushButton):
    """Keep a native button while exposing an existing async busy projection."""

    MOTION_MODE = "activity"

    def __init__(self, text: str, parent: QWidget | None = None) -> None:
        super().__init__(text, parent)
        self._busy = False
        self._phase = 0.0
        self._animated = False

    def set_busy(self, busy: bool) -> None:
        """Show a bounded activity rail without owning the busy state source."""

        normalized = bool(busy)
        if self._busy == normalized:
            return
        self._busy = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the activity rail while the shared motion owner is suspended."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if not self._busy:
            return
        _paint_signal_rail(
            self,
            self._phase,
            self._animated,
            theme_spec_for_widget(self).accent_blue,
            animated_alpha=210,
            static_alpha=130,
            half_span=12.0,
            radius=1.9,
            pen_width=1.2,
        )


__all__ = ["ActionRailButton", "BusyActionButton"]
