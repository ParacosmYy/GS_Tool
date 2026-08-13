"""Presentation-only error notification signal surface."""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class ErrorSignalSurface(QWidget):
    """Draw a compact fault beacon beside the authoritative error message."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._active = False
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(28, 28)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_active(self, active: bool) -> None:
        """Reflect whether the existing error surface is visible."""

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
        """Freeze the beacon while the shared motion owner is suspended."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        if not self._active or self.width() < 18 or self.height() < 18:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(3.0, 3.0, -3.0, -3.0)
        center = bounds.center()
        error = QColor(theme.error)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        halo = QColor(error)
        halo.setAlpha(55 if self._animated else 35)
        painter.setPen(QPen(halo, 1.0))
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawEllipse(bounds.adjusted(1.0, 1.0, -1.0, -1.0))

        if self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(error)
            pulse.setAlpha(int(80 + wave * 95))
            painter.setPen(QPen(pulse, 1.2))
            painter.drawEllipse(center, 7.0 + wave * 2.0, 7.0 + wave * 2.0)

        ring = QColor(error)
        ring.setAlpha(225)
        painter.setPen(QPen(ring, 1.5))
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawEllipse(center, 6.0, 6.0)

        marker = QColor(theme.text)
        marker.setAlpha(235)
        painter.setPen(QPen(marker, 1.4))
        painter.drawLine(
            QPointF(center.x(), center.y() - 3.0),
            QPointF(center.x(), center.y() + 1.0),
        )
        painter.drawEllipse(center.x() - 0.8, center.y() + 3.0, 1.6, 1.6)


__all__ = ["ErrorSignalSurface"]
