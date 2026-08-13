"""Presentation-only theme palette swatch for the shell header."""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class ThemePaletteSwatch(QWidget):
    """Show the active semantic palette without becoming another control."""

    _COLORS = ("accent", "accent_blue", "accent_purple", "accent_pink")

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(62, 22)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shell frame without creating a widget-owned clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the decorative palette pulse during motion suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(1.5, 2.0, -1.5, -2.0)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        surface = QColor(theme.surface_input)
        surface.setAlpha(190)
        border = QColor(theme.border)
        border.setAlpha(175)
        painter.setBrush(surface)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 7.0, 7.0)

        left = bounds.left() + 9.0
        right = bounds.right() - 9.0
        y = bounds.center().y()
        span = max(1.0, right - left)
        positions = tuple(
            left + span * index / (len(self._COLORS) - 1)
            for index in range(len(self._COLORS))
        )
        guide = QColor(theme.border)
        guide.setAlpha(110)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, y), QPointF(right, y))

        for position, token in zip(positions, self._COLORS, strict=True):
            color = QColor(getattr(theme, token))
            color.setAlpha(220)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(color)
            painter.drawEllipse(QPointF(position, y), 2.8, 2.8)

        if self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = left + span * travel
            pulse = QColor(theme.accent)
            pulse.setAlpha(155 + int(travel * 65))
            painter.setPen(QPen(pulse, 1.1))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, y), 4.7, 4.7)


__all__ = ["ThemePaletteSwatch"]
