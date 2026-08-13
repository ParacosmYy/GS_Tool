"""Theme-aware vector glyphs for the native workspace tab bar."""

from __future__ import annotations

from .qt import QColor, QIcon, QPainter, QPen, QPixmap, QPointF, QRectF, Qt, QTabWidget
from .theme_tokens import ThemeSpec

_ICON_SIZE = 18
_TAB_COUNT = 4


def refresh_workspace_tab_icons(tabs: QTabWidget | None, theme: ThemeSpec) -> None:
    """Refresh tab glyphs without replacing native labels or navigation."""

    if tabs is None:
        return
    for index in range(min(_TAB_COUNT, tabs.count())):
        tabs.setTabIcon(index, _build_icon(theme, index))


def _build_icon(theme: ThemeSpec, index: int) -> QIcon:
    """Build icon."""
    icon = QIcon()
    for mode, color in (
        (QIcon.Mode.Normal, theme.text_muted),
        (QIcon.Mode.Selected, theme.accent),
        (QIcon.Mode.Disabled, theme.disabled_text),
    ):
        icon.addPixmap(_render_glyph(theme, index, color), mode, QIcon.State.Off)
    return icon


def _render_glyph(theme: ThemeSpec, index: int, color: str) -> QPixmap:
    """Render glyph."""
    pixmap = QPixmap(_ICON_SIZE, _ICON_SIZE)
    pixmap.fill(Qt.GlobalColor.transparent)
    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

    ink = QColor(color)
    ink.setAlpha(225)
    pen = QPen(ink, 1.35)
    pen.setCapStyle(Qt.PenCapStyle.RoundCap)
    pen.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
    painter.setPen(pen)
    painter.setBrush(Qt.BrushStyle.NoBrush)

    if index == 0:
        _draw_connection_glyph(painter, ink)
    elif index == 1:
        _draw_protocol_glyph(painter, ink)
    elif index == 2:
        _draw_command_glyph(painter, ink)
    else:
        _draw_extension_glyph(painter, ink)

    accent = QColor(
        (theme.accent_blue, theme.accent, theme.accent_pink, theme.accent_purple)[index]
    )
    accent.setAlpha(185)
    painter.setPen(Qt.PenStyle.NoPen)
    painter.setBrush(accent)
    painter.drawEllipse(QPointF(14.2, 3.8), 1.25, 1.25)
    painter.end()
    return pixmap


def _draw_connection_glyph(painter: QPainter, ink: QColor) -> None:
    """Draw connection glyph."""
    painter.drawLine(QPointF(3.0, 9.0), QPointF(7.0, 9.0))
    painter.drawLine(QPointF(11.0, 9.0), QPointF(15.0, 9.0))
    painter.drawLine(QPointF(7.0, 9.0), QPointF(9.0, 6.0))
    painter.drawLine(QPointF(9.0, 6.0), QPointF(11.0, 9.0))
    painter.drawEllipse(QPointF(3.0, 9.0), 1.8, 1.8)
    painter.drawEllipse(QPointF(15.0, 9.0), 1.8, 1.8)
    painter.setPen(Qt.PenStyle.NoPen)
    painter.setBrush(ink)
    painter.drawEllipse(QPointF(9.0, 6.0), 1.2, 1.2)


def _draw_protocol_glyph(painter: QPainter, ink: QColor) -> None:
    """Draw protocol glyph."""
    painter.drawPolyline(
        (
            QPointF(2.5, 11.0),
            QPointF(5.0, 11.0),
            QPointF(7.0, 5.0),
            QPointF(9.5, 13.0),
            QPointF(12.0, 7.0),
            QPointF(15.5, 7.0),
        )
    )
    painter.setPen(Qt.PenStyle.NoPen)
    painter.setBrush(ink)
    for point in (QPointF(7.0, 5.0), QPointF(9.5, 13.0), QPointF(12.0, 7.0)):
        painter.drawEllipse(point, 1.15, 1.15)


def _draw_command_glyph(painter: QPainter, ink: QColor) -> None:
    """Draw command glyph."""
    painter.drawRoundedRect(QRectF(2.5, 4.0, 13.0, 10.0), 2.0, 2.0)
    painter.drawLine(QPointF(5.0, 7.0), QPointF(7.0, 9.0))
    painter.drawLine(QPointF(7.0, 9.0), QPointF(5.0, 11.0))
    painter.drawLine(QPointF(9.0, 11.0), QPointF(12.5, 11.0))


def _draw_extension_glyph(painter: QPainter, ink: QColor) -> None:
    """Draw extension glyph."""
    painter.drawRoundedRect(QRectF(3.0, 3.0, 12.0, 12.0), 3.0, 3.0)
    painter.drawLine(QPointF(6.0, 9.0), QPointF(12.0, 9.0))
    painter.drawLine(QPointF(9.0, 6.0), QPointF(9.0, 12.0))
    painter.setPen(Qt.PenStyle.NoPen)
    painter.setBrush(ink)
    painter.drawEllipse(QPointF(5.0, 5.0), 1.0, 1.0)
    painter.drawEllipse(QPointF(13.0, 13.0), 1.0, 1.0)


__all__ = ["refresh_workspace_tab_icons"]
