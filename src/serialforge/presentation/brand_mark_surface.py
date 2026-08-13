"""Presentation-only geometric brand mark for the application header."""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class BrandMarkSurface(QWidget):
    """Draw a compact, resource-free constellation mark beside the wordmark."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(38, 38)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without owning a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze decorative motion while retaining a readable static mark."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(2.0, 2.0, -2.0, -2.0)
        if bounds.width() < 26.0 or bounds.height() < 26.0:
            return

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        center = bounds.center()

        shell = QColor(theme.surface_input)
        shell.setAlpha(232)
        border = QColor(theme.accent_purple)
        border.setAlpha(185)
        painter.setBrush(shell)
        painter.setPen(QPen(border, 1.2))
        painter.drawRoundedRect(bounds, 11.0, 11.0)

        orbit = QColor(theme.accent_blue)
        orbit.setAlpha(135 if self._animated else 92)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(orbit, 0.9))
        painter.drawArc(
            QRectF(center.x() - 13.0, center.y() - 8.0, 26.0, 16.0),
            18 * 16,
            144 * 16,
        )
        painter.drawArc(
            QRectF(center.x() - 8.0, center.y() - 13.0, 16.0, 26.0),
            198 * 16,
            144 * 16,
        )

        guide = QColor(theme.border)
        guide.setAlpha(130)
        painter.setPen(QPen(guide, 0.8))
        painter.drawLine(
            QPointF(center.x() - 12.0, center.y()),
            QPointF(center.x() + 12.0, center.y()),
        )
        painter.drawLine(
            QPointF(center.x(), center.y() - 12.0),
            QPointF(center.x(), center.y() + 12.0),
        )

        mark = QColor(theme.accent)
        mark.setAlpha(240)
        mark_pen = QPen(mark, 1.8)
        mark_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        mark_pen.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
        painter.setPen(mark_pen)
        painter.drawLine(QPointF(12.0, 13.0), QPointF(17.0, 10.5))
        painter.drawLine(QPointF(17.0, 10.5), QPointF(24.0, 11.0))
        painter.drawLine(QPointF(24.0, 11.0), QPointF(26.0, 14.0))
        painter.drawLine(QPointF(26.0, 14.0), QPointF(19.0, 17.5))
        painter.drawLine(QPointF(19.0, 17.5), QPointF(13.0, 20.5))
        painter.drawLine(QPointF(13.0, 20.5), QPointF(15.0, 24.5))
        painter.drawLine(QPointF(15.0, 24.5), QPointF(22.0, 26.0))
        painter.drawLine(QPointF(22.0, 26.0), QPointF(26.0, 23.5))

        for point, color_name in (
            (QPointF(8.5, 9.0), "accent_pink"),
            (QPointF(29.5, 8.5), "accent_blue"),
            (QPointF(29.0, 28.5), "accent_purple"),
        ):
            node = QColor(getattr(theme, color_name))
            node.setAlpha(210)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(node)
            painter.drawEllipse(point, 1.5, 1.5)

        if self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            halo = QColor(theme.accent_pink)
            halo.setAlpha(24 + int(wave * 36))
            painter.setBrush(halo)
            painter.drawEllipse(center, 7.0 + wave * 2.2, 7.0 + wave * 2.2)

            pulse = QColor(theme.accent)
            pulse.setAlpha(210)
            pulse_x = center.x() + math.cos(self._phase * 0.72) * 10.0
            pulse_y = center.y() + math.sin(self._phase * 0.72) * 6.0
            painter.setBrush(pulse)
            painter.drawEllipse(QPointF(pulse_x, pulse_y), 1.15, 1.15)

        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QColor(theme.text))
        painter.drawEllipse(center, 1.6, 1.6)


__all__ = ["BrandMarkSurface"]
