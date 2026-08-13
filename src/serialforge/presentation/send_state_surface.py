"""Presentation-only state rail for the native send-status label."""

from __future__ import annotations

import math

from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class SendStateSurface(QLabel):
    """Keep send-state text authoritative while rendering a compact state rail."""

    _STATES = frozenset({"blocked", "waiting", "ready", "busy", "history"})
    _ACTIVE_NODES = {
        "blocked": 1,
        "waiting": 2,
        "ready": 4,
        "busy": 4,
        "history": 3,
    }

    def __init__(self, text: str = "", parent: QWidget | None = None) -> None:
        super().__init__(text, parent)
        self._phase = 0.0
        self._animated = False
        self.setContentsMargins(0, 0, 0, 12)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume one shared shell frame without creating a local timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the rail during reduced-motion or lifecycle suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if self.width() < 96 or self.height() < 18:
            return

        state = self._state()
        theme = theme_spec_for_widget(self)
        content = QRectF(self.contentsRect())
        band_top = content.bottom() + 1.0
        band_bottom = float(self.rect().bottom()) - 1.0
        if band_bottom - band_top < 5.0:
            return
        bounds = QRectF(content.left(), band_top, content.width(), band_bottom - band_top)
        bounds = bounds.adjusted(8.0, 0.0, -8.0, 0.0)
        left = bounds.left()
        right = bounds.right()
        center_y = bounds.center().y()
        span = max(1.0, right - left)
        color = QColor(self._state_color(theme, state))
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        guide = QColor(theme.border)
        guide.setAlpha(115)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, center_y), QPointF(right, center_y))

        active_nodes = self._ACTIVE_NODES[state]
        positions = tuple(
            left + span * index / 3.0 for index in range(4)
        )
        for index, x in enumerate(positions):
            active = index < active_nodes
            node = QColor(color)
            node.setAlpha(195 if active else 65)
            painter.setPen(QPen(node, 1.0))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(x, center_y), 2.5, 2.5)
            if active:
                painter.setPen(Qt.PenStyle.NoPen)
                painter.setBrush(node)
                painter.drawEllipse(QPointF(x, center_y), 1.1, 1.1)

        if state == "busy" and self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = left + span * travel
            pulse = QColor(theme.warning)
            pulse.setAlpha(145 + int(travel * 75))
            painter.setPen(QPen(pulse, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, center_y), 4.0, 4.0)

    def _state(self) -> str:
        """State."""
        value = self.property("state")
        return value if isinstance(value, str) and value in self._STATES else "blocked"

    @staticmethod
    def _state_color(theme, state: str) -> str:
        """State color."""
        return {
            "blocked": theme.text_subtle,
            "waiting": theme.accent_blue,
            "ready": theme.success,
            "busy": theme.warning,
            "history": theme.accent_purple,
        }[state]


__all__ = ["SendStateSurface"]
