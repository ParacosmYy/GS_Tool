"""Resource-free sweep overlay for the one-shot theme transition."""

from __future__ import annotations

from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget


class ThemeTransitionSurface(QWidget):
    """Paint a short semantic-color light band without owning animation time."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAttribute(Qt.WidgetAttribute.WA_NoSystemBackground, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        if self.width() < 16 or self.height() < 40:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(0.0, 0.0, -1.0, -1.0)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        veil = QColor(theme.accent_blue)
        veil.setAlpha(22)
        painter.fillRect(bounds, veil)

        height = bounds.height()
        for index in range(-3, 8):
            color = QColor(theme.accent_pink if index % 2 else theme.accent_blue)
            color.setAlpha(82 if index % 2 else 62)
            painter.setPen(QPen(color, 1.1))
            x = bounds.left() + index * 28.0
            painter.drawLine(
                QPointF(x, bounds.bottom()),
                QPointF(x + height * 0.42, bounds.top()),
            )

        core = QColor(theme.accent)
        core.setAlpha(120)
        painter.setPen(QPen(core, 1.8))
        center_x = bounds.center().x()
        painter.drawLine(
            QPointF(center_x, bounds.bottom()),
            QPointF(center_x + height * 0.42, bounds.top()),
        )

        node = QColor(theme.text)
        node.setAlpha(180)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(node)
        painter.drawEllipse(
            QPointF(center_x + height * 0.18, bounds.center().y()),
            2.0,
            2.0,
        )


__all__ = ["ThemeTransitionSurface"]
