"""Presentation-only status footer signal surface.

The native QStatusBar message remains the authoritative, accessible status
text.  This small permanent widget adds a compact visual signal rail for the
existing session, fault, and RX-activity facts without owning a timer or
duplicating application state.
"""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, QSizePolicy, Qt, QWidget
from .theme import theme_spec_for_widget


class StatusFooterSurface(QWidget):
    """Draw a compact session/RX signal rail beside the native status text."""

    _STATES = frozenset({"discovered", "opening", "open", "closing", "closed", "error"})
    _ACTIVE_NODES = {
        "closed": 0,
        "discovered": 1,
        "opening": 2,
        "open": 4,
        "closing": 3,
        "error": 1,
    }

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._state = "closed"
        self._fault = False
        self._activity = False
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(116, 18)
        self.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_state(self, state: object) -> None:
        """Reflect the existing session state without interpreting transport data."""

        value = getattr(state, "value", state)
        normalized = str(value).lower()
        normalized = normalized if normalized in self._STATES else "closed"
        if self._state == normalized:
            return
        self._state = normalized
        self.update()

    def set_fault(self, fault: bool) -> None:
        """Show an application fault accent while native status text stays authoritative."""

        normalized = bool(fault)
        if self._fault == normalized:
            return
        self._fault = normalized
        self.update()

    def set_activity(self, active: bool) -> None:
        """Reflect the existing short RX activity projection."""

        normalized = bool(active)
        if self._activity == normalized:
            return
        self._activity = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the footer while the shared lifecycle suspends motion."""

        self._animated = False
        self.update()

    def motion_active(self) -> bool:
        """Return whether the footer currently renders a moving pulse."""

        return self._activity or self._state in {"opening", "open", "closing"}

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        if self.width() < 60 or self.height() < 12:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(7.0, 5.0, -7.0, -5.0)
        left = bounds.left()
        right = bounds.right()
        y = bounds.center().y()
        span = max(1.0, right - left)
        positions = tuple(left + span * index / 3.0 for index in range(4))
        active_nodes = self._ACTIVE_NODES[self._state]
        color = self._state_color(theme)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        guide = QColor(theme.border)
        guide.setAlpha(120)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, y), QPointF(right, y))

        if active_nodes > 1:
            active_line = QColor(color)
            active_line.setAlpha(175 if self._activity else 125)
            painter.setPen(QPen(active_line, 1.6))
            for index in range(active_nodes - 1):
                painter.drawLine(
                    QPointF(positions[index], y),
                    QPointF(positions[index + 1], y),
                )

        for index, x in enumerate(positions):
            node_color = QColor(color if index < active_nodes else theme.surface_input)
            node_color.setAlpha(225 if index < active_nodes else 175)
            painter.setPen(QPen(color if index < active_nodes else guide, 1.0))
            painter.setBrush(node_color)
            painter.drawEllipse(QPointF(x, y), 2.5, 2.5)

        if self._activity and self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = left + span * travel
            pulse = QColor(theme.accent)
            pulse.setAlpha(220)
            painter.setPen(QPen(pulse, 1.0))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, y), 5.0, 5.0)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(pulse)
            painter.drawEllipse(QPointF(pulse_x, y), 1.7, 1.7)
        elif self._fault:
            fault = QColor(theme.error)
            fault.setAlpha(210)
            painter.setPen(QPen(fault, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(right, y), 4.2, 4.2)

    def _state_color(self, theme) -> QColor:
        """State color."""
        if self._fault or self._state == "error":
            return QColor(theme.error)
        if self._state in {"opening", "closing"}:
            return QColor(theme.warning)
        if self._state == "discovered":
            return QColor(theme.accent_blue)
        if self._state == "open":
            return QColor(theme.accent)
        return QColor(theme.text_muted)


__all__ = ["StatusFooterSurface"]
