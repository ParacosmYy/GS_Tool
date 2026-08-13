"""Presentation-only workspace route beacon.

The beacon gives the four primary workspaces a compact visual anchor while
the tab labels remain the authoritative, accessible navigation surface.  It
consumes only the selected tab index and the shared shell frame.
"""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class WorkspaceRouteSurface(QWidget):
    """Draw a small four-node constellation for the active workspace tab."""

    _TAB_COUNT = 4

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._index = 0
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(148, 28)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_index(self, index: int) -> None:
        """Reflect the existing QTabWidget index without owning navigation."""

        try:
            normalized = max(0, min(self._TAB_COUNT - 1, int(index)))
        except (TypeError, ValueError):
            normalized = 0
        if self._index == normalized:
            return
        self._index = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume one shared frame without creating a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the selected route marker during lifecycle suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        bounds = QRectF(self.rect()).adjusted(4.0, 3.0, -4.0, -3.0)
        if bounds.width() < 80.0 or bounds.height() < 16.0:
            return

        theme = theme_spec_for_widget(self)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        panel = QColor(theme.surface)
        panel.setAlpha(190)
        painter.setBrush(panel)
        border = QColor(theme.border)
        border.setAlpha(150)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 8.0, 8.0)

        left = bounds.left() + 13.0
        right = bounds.right() - 13.0
        y = bounds.center().y()
        span = max(1.0, right - left)
        positions = tuple(
            left + span * index / (self._TAB_COUNT - 1)
            for index in range(self._TAB_COUNT)
        )
        palette = (
            theme.accent_blue,
            theme.accent,
            theme.accent_pink,
            theme.accent_purple,
        )
        selected_color = QColor(palette[self._index])

        guide = QColor(theme.border)
        guide.setAlpha(110)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(positions[0], y), QPointF(positions[-1], y))

        for index in range(self._index):
            segment = QColor(selected_color)
            segment.setAlpha(150)
            painter.setPen(QPen(segment, 1.6))
            painter.drawLine(QPointF(positions[index], y), QPointF(positions[index + 1], y))

        for index, x in enumerate(positions):
            node_color = QColor(palette[index])
            active = index == self._index
            node_color.setAlpha(220 if active else 115)
            painter.setPen(QPen(node_color if active else guide, 1.0))
            painter.setBrush(QColor(theme.surface_input) if not active else node_color)
            painter.drawEllipse(QPointF(x, y), 3.4 if active else 2.8, 3.4 if active else 2.8)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(QColor(theme.text) if active else node_color)
            painter.drawEllipse(QPointF(x, y), 1.1 if active else 0.9, 1.1 if active else 0.9)

        marker = QColor(selected_color)
        marker.setAlpha(190 if self._animated else 120)
        painter.setPen(QPen(marker, 1.0))
        painter.drawLine(
            QPointF(positions[self._index], bounds.top() + 3.0),
            QPointF(positions[self._index], bounds.top() + 6.0),
        )

        if self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            halo = QColor(selected_color)
            halo.setAlpha(28 + int(wave * 38))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(halo)
            painter.drawEllipse(
                QPointF(positions[self._index], y),
                5.5 + wave * 2.0,
                5.5 + wave * 2.0,
            )

            pulse = QColor(theme.accent_purple)
            pulse.setAlpha(170)
            pulse_x = left + span * ((math.sin(self._phase * 0.72) + 1.0) * 0.5)
            painter.setBrush(pulse)
            painter.drawEllipse(QPointF(pulse_x, y), 1.5, 1.5)


__all__ = ["WorkspaceRouteSurface"]
