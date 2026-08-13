"""Presentation-only send input decoration.

The native QLineEdit remains responsible for text editing, selection, focus,
clipboard, cursor, and keyboard behavior.  This surface only paints a small
state rail after the native result using an existing presentation projection
and the shell's shared MotionController frame.
"""

from __future__ import annotations

import math

from .qt import QColor, QLineEdit, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class SendInputSurface(QLineEdit):
    """Keep a native send editor while exposing its existing availability state."""

    _STATES = frozenset({"blocked", "waiting", "ready", "busy", "history"})

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._surface_state = "blocked"
        self._phase = 0.0
        self._animated = False

    def set_surface_state(self, state: object) -> None:
        """Reflect the existing send-band state without owning its source."""

        value = getattr(state, "value", state)
        normalized = str(value).lower()
        normalized = normalized if normalized in self._STATES else "blocked"
        if self._surface_state == normalized:
            return
        self._surface_state = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the decorative rail while the shared motion owner is suspended."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if self.width() < 96 or self.height() < 14:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(10.0, 0.0, -10.0, -3.0)
        if bounds.width() <= 0 or bounds.height() <= 0:
            return

        color = self._state_color(theme)
        active = self._surface_state in {"ready", "busy"}
        focused = self.hasFocus() and self.isEnabled()
        line = QColor(color)
        line.setAlpha(190 if focused else 125)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setClipRect(bounds)
        y = bounds.bottom()
        painter.setPen(QPen(line, 1.0 if not focused else 1.3))
        painter.drawLine(QPointF(bounds.left(), y), QPointF(bounds.right(), y))

        if focused:
            focus_line = QColor(theme.focus)
            focus_line.setAlpha(95)
            painter.setPen(QPen(focus_line, 1.0))
            painter.drawLine(
                QPointF(bounds.left(), y - 2.0),
                QPointF(bounds.right(), y - 2.0),
            )

        if active and focused and self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = bounds.left() + bounds.width() * travel
            pulse = QColor(theme.accent)
            pulse.setAlpha(225)
            painter.setPen(QPen(pulse, 1.1))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, y), 4.8, 4.8)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(pulse)
            painter.drawEllipse(QPointF(pulse_x, y), 1.5, 1.5)
        elif active:
            marker = QColor(color)
            marker.setAlpha(190)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(marker)
            painter.drawEllipse(QPointF(bounds.right(), y), 2.0, 2.0)

    def _state_color(self, theme) -> QColor:
        """State color."""
        colors = {
            "blocked": theme.text_muted,
            "waiting": theme.accent_blue,
            "ready": theme.accent,
            "busy": theme.warning,
            "history": theme.accent_purple,
        }
        return QColor(colors[self._surface_state])


__all__ = ["SendInputSurface"]
