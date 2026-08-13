"""Presentation-only glyph for the selected connection transport."""

from __future__ import annotations

import math

from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class TransportModeSurface(QWidget):
    """Draw a compact transport glyph without becoming a second state source.

    The adjacent ``QComboBox`` remains the authoritative, keyboard-accessible
    mode selector.  This surface only mirrors its bounded string value and a
    shared shell frame; it never configures or starts a transport.
    """

    _MODES = frozenset({"uart", "tcp_stream", "tcp_server", "udp_datagram", "ble_gatt", "rtt"})

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._mode = "uart"
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(42, 26)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_mode(self, mode: object) -> None:
        """Mirror the existing combo value without importing domain state."""

        normalized = str(getattr(mode, "value", mode)).lower()
        if normalized not in self._MODES:
            normalized = "uart"
        if self._mode == normalized:
            return
        self._mode = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared presentation frame without creating a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the glyph during reduced-motion or lifecycle suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        del event
        bounds = QRectF(self.rect()).adjusted(1.0, 2.0, -1.0, -2.0)
        if bounds.width() < 30.0 or bounds.height() < 18.0:
            return

        theme = theme_spec_for_widget(self)
        accent = QColor(self._accent(theme))
        accent.setAlpha(220)
        guide = QColor(theme.border)
        guide.setAlpha(150)
        panel = QColor(theme.surface_input)
        panel.setAlpha(220)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setBrush(panel)
        painter.setPen(QPen(guide, 1.0))
        painter.drawRoundedRect(bounds, 7.0, 7.0)

        center = bounds.center()
        painter.setPen(QPen(accent, 1.35))
        painter.setBrush(Qt.BrushStyle.NoBrush)
        if self._mode == "uart":
            self._draw_uart(painter, center, accent)
        elif self._mode == "tcp_stream":
            self._draw_stream(painter, center, accent, server=False)
        elif self._mode == "tcp_server":
            self._draw_stream(painter, center, accent, server=True)
        elif self._mode == "udp_datagram":
            self._draw_datagram(painter, center, accent)
        elif self._mode == "ble_gatt":
            self._draw_ble(painter, center, accent)
        else:
            self._draw_rtt(painter, center, accent)

        if self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            halo = QColor(accent)
            halo.setAlpha(26 + int(wave * 34))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(halo)
            painter.drawEllipse(center, 8.0 + wave * 2.0, 8.0 + wave * 2.0)
            pulse = QColor(theme.accent)
            pulse.setAlpha(185)
            pulse_x = bounds.left() + 7.0 + ((math.sin(self._phase * 0.72) + 1.0) * 0.5) * (
                bounds.width() - 14.0
            )
            painter.setBrush(pulse)
            painter.drawEllipse(QPointF(pulse_x, bounds.bottom() - 3.0), 1.2, 1.2)

    def _accent(self, theme):
        return {
            "uart": theme.accent,
            "tcp_stream": theme.accent_blue,
            "tcp_server": theme.accent_purple,
            "udp_datagram": theme.warning,
            "ble_gatt": theme.accent_pink,
            "rtt": theme.accent,
        }[self._mode]

    @staticmethod
    def _draw_uart(painter: QPainter, center: QPointF, accent: QColor) -> None:
        painter.drawRoundedRect(QRectF(center.x() - 9.0, center.y() - 6.0, 13.0, 12.0), 2.0, 2.0)
        painter.drawLine(
            QPointF(center.x() + 4.0, center.y()),
            QPointF(center.x() + 10.0, center.y()),
        )
        painter.drawLine(
            QPointF(center.x() + 7.0, center.y() - 3.0),
            QPointF(center.x() + 10.0, center.y()),
        )
        painter.drawLine(
            QPointF(center.x() + 7.0, center.y() + 3.0),
            QPointF(center.x() + 10.0, center.y()),
        )
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(accent)
        painter.drawEllipse(QPointF(center.x() - 5.0, center.y()), 1.3, 1.3)

    @staticmethod
    def _draw_stream(
        painter: QPainter,
        center: QPointF,
        accent: QColor,
        *,
        server: bool,
    ) -> None:
        painter.drawLine(
            QPointF(center.x() - 13.0, center.y()),
            QPointF(center.x() + 13.0, center.y()),
        )
        if server:
            painter.drawLine(
                QPointF(center.x(), center.y() - 6.0),
                QPointF(center.x(), center.y() + 6.0),
            )
            painter.drawEllipse(QPointF(center.x(), center.y()), 3.0, 3.0)
        else:
            painter.drawLine(
                QPointF(center.x() - 5.0, center.y() - 4.0),
                QPointF(center.x() - 9.0, center.y()),
            )
            painter.drawLine(
                QPointF(center.x() - 5.0, center.y() + 4.0),
                QPointF(center.x() - 9.0, center.y()),
            )
            painter.drawLine(
                QPointF(center.x() + 5.0, center.y() - 4.0),
                QPointF(center.x() + 9.0, center.y()),
            )
            painter.drawLine(
                QPointF(center.x() + 5.0, center.y() + 4.0),
                QPointF(center.x() + 9.0, center.y()),
            )
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(accent)
        painter.drawEllipse(QPointF(center.x(), center.y()), 1.4, 1.4)

    @staticmethod
    def _draw_datagram(painter: QPainter, center: QPointF, accent: QColor) -> None:
        points = (
            QPointF(center.x(), center.y() - 7.0),
            QPointF(center.x() + 8.0, center.y()),
            QPointF(center.x(), center.y() + 7.0),
            QPointF(center.x() - 8.0, center.y()),
        )
        painter.drawPolygon(points)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(accent)
        painter.drawEllipse(center, 2.0, 2.0)

    @staticmethod
    def _draw_ble(painter: QPainter, center: QPointF, accent: QColor) -> None:
        painter.drawLine(
            QPointF(center.x(), center.y() + 7.0),
            QPointF(center.x(), center.y() - 7.0),
        )
        painter.drawLine(
            QPointF(center.x(), center.y() - 7.0),
            QPointF(center.x() + 5.0, center.y() - 2.0),
        )
        painter.drawLine(
            QPointF(center.x() + 5.0, center.y() - 2.0),
            QPointF(center.x(), center.y() + 2.0),
        )
        painter.drawLine(
            QPointF(center.x(), center.y() + 2.0),
            QPointF(center.x() + 5.0, center.y() + 7.0),
        )
        painter.drawLine(
            QPointF(center.x() - 8.0, center.y() - 4.0),
            QPointF(center.x() - 4.0, center.y() - 4.0),
        )
        painter.drawLine(
            QPointF(center.x() - 9.0, center.y() + 4.0),
            QPointF(center.x() - 4.0, center.y() + 4.0),
        )
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(accent)
        painter.drawEllipse(QPointF(center.x() - 10.0, center.y()), 1.5, 1.5)

    @staticmethod
    def _draw_rtt(painter: QPainter, center: QPointF, accent: QColor) -> None:
        painter.drawRoundedRect(QRectF(center.x() - 8.0, center.y() - 6.0, 16.0, 12.0), 2.0, 2.0)
        painter.drawLine(
            QPointF(center.x() - 4.0, center.y() - 2.0),
            QPointF(center.x() + 4.0, center.y() - 2.0),
        )
        painter.drawLine(
            QPointF(center.x() - 4.0, center.y() + 2.0),
            QPointF(center.x() + 1.0, center.y() + 2.0),
        )
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(accent)
        painter.drawEllipse(QPointF(center.x() + 5.0, center.y() + 2.0), 1.3, 1.3)


__all__ = ["TransportModeSurface"]
