"""Application-authored icon resolution for source and frozen builds."""

from __future__ import annotations

import sys
from pathlib import Path

from PyQt6.QtCore import QPointF, QRectF, QSize, Qt
from PyQt6.QtGui import QBrush, QColor, QIcon, QPainter, QPainterPath, QPen, QPixmap

from .icon_contract import IconKey

_ICON_SIZE = QSize(22, 22)
_SWATCH_SIZE = QSize(32, 20)


def application_icon() -> QIcon:
    """Load the authored anime-forge mark without relying on the cwd."""
    path = _resource_path("assets/quillforge.ico")
    return QIcon(str(path)) if path.is_file() else QIcon()


def themed_icon(
    key: IconKey,
    *,
    foreground: str,
    accent: str | None = None,
    disabled_foreground: str | None = None,
    disabled_accent: str | None = None,
) -> QIcon:
    """Render one small, theme-tinted vector icon for a Qt presentation surface."""
    accent_color = QColor(accent or foreground)
    foreground_color = QColor(foreground)
    disabled_foreground_color = QColor(disabled_foreground or foreground)
    disabled_accent_color = QColor(disabled_accent or accent or foreground)
    if disabled_foreground is None:
        disabled_foreground_color.setAlpha(92)
    if disabled_accent is None:
        disabled_accent_color.setAlpha(92)

    icon = QIcon()
    icon.addPixmap(
        _render_icon(key, foreground_color, accent_color),
        QIcon.Mode.Normal,
        QIcon.State.Off,
    )
    icon.addPixmap(
        _render_icon(key, disabled_foreground_color, disabled_accent_color),
        QIcon.Mode.Disabled,
        QIcon.State.Off,
    )
    return icon


def color_swatch_icon(
    primary: str,
    secondary: str,
    *,
    tertiary: str | None = None,
    outline: str,
) -> QIcon:
    """Render a compact multi-tone swatch for a presentation choice."""
    icon = QIcon()
    icon.addPixmap(
        _render_color_swatch(
            QColor(primary),
            QColor(secondary),
            QColor(tertiary or secondary),
            QColor(outline),
        ),
        QIcon.Mode.Normal,
        QIcon.State.Off,
    )
    return icon


def _render_icon(key: IconKey, foreground: QColor, accent: QColor) -> QPixmap:
    pixmap = QPixmap(_ICON_SIZE)
    pixmap.fill(Qt.GlobalColor.transparent)
    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
    line = _line_pen(foreground)
    detail = _line_pen(accent, width=1.65)

    if key is IconKey.DOCUMENT:
        _draw_document(painter, line)
    elif key is IconKey.DOCUMENT_NEW:
        _draw_document(painter, line)
        _draw_plus(painter, detail, center=QPointF(11.2, 14.1))
    elif key is IconKey.FOLDER:
        _draw_folder(painter, line, accent)
    elif key is IconKey.FOLDER_OPEN:
        _draw_folder(painter, line, accent)
        _draw_arrow(painter, detail, start=QPointF(8.5, 13.7), end=QPointF(16.3, 13.7))
    elif key is IconKey.SAVE:
        _draw_document(painter, line)
        _draw_arrow(painter, detail, start=QPointF(11.4, 9.2), end=QPointF(11.4, 15.8))
        painter.drawLine(QPointF(8.9, 13.3), QPointF(11.4, 15.8))
        painter.drawLine(QPointF(13.9, 13.3), QPointF(11.4, 15.8))
    elif key is IconKey.SEARCH:
        painter.setPen(line)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawEllipse(QRectF(4.2, 3.8, 10.2, 10.2))
        painter.drawLine(QPointF(13.1, 12.7), QPointF(18.1, 17.7))
    elif key is IconKey.REPLACE:
        _draw_replace(painter, line, detail)
    elif key is IconKey.COMMAND:
        _draw_sparkle(painter, line, detail)
    elif key is IconKey.ARROW_UP:
        _draw_arrow(painter, line, start=QPointF(11, 18), end=QPointF(11, 4))
        painter.drawLine(QPointF(6.5, 8.5), QPointF(11, 4))
        painter.drawLine(QPointF(15.5, 8.5), QPointF(11, 4))
    elif key is IconKey.ARROW_DOWN:
        _draw_arrow(painter, line, start=QPointF(11, 4), end=QPointF(11, 18))
        painter.drawLine(QPointF(6.5, 13.5), QPointF(11, 18))
        painter.drawLine(QPointF(15.5, 13.5), QPointF(11, 18))
    elif key is IconKey.CLOSE:
        painter.setPen(line)
        painter.drawLine(QPointF(5, 5), QPointF(17, 17))
        painter.drawLine(QPointF(17, 5), QPointF(5, 17))
    elif key is IconKey.MODIFIED:
        _draw_document(painter, line)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QBrush(accent))
        painter.drawEllipse(QRectF(13.1, 13.2, 5.4, 5.4))
    elif key is IconKey.WARNING:
        _draw_warning(painter, line, accent)

    painter.end()
    return pixmap


def _render_color_swatch(
    primary: QColor,
    secondary: QColor,
    tertiary: QColor,
    outline: QColor,
) -> QPixmap:
    pixmap = QPixmap(_SWATCH_SIZE)
    pixmap.fill(Qt.GlobalColor.transparent)
    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

    frame = QRectF(1.0, 2.0, 30.0, 16.0)
    painter.setPen(_line_pen(outline, width=1.0))
    painter.setBrush(QBrush(primary))
    painter.drawRoundedRect(frame, 6.0, 6.0)

    painter.setPen(_line_pen(outline, width=0.9))
    painter.setBrush(QBrush(secondary))
    painter.drawEllipse(QRectF(14.0, 4.0, 12.0, 12.0))
    painter.setBrush(QBrush(tertiary))
    painter.drawEllipse(QRectF(22.0, 9.0, 7.0, 7.0))

    painter.end()
    return pixmap


def _line_pen(color: QColor, *, width: float = 1.9) -> QPen:
    pen = QPen(color)
    pen.setWidthF(width)
    pen.setCapStyle(Qt.PenCapStyle.RoundCap)
    pen.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
    return pen


def _draw_document(painter: QPainter, pen: QPen) -> None:
    painter.setPen(pen)
    painter.setBrush(Qt.BrushStyle.NoBrush)
    path = QPainterPath()
    path.moveTo(5.4, 3.2)
    path.lineTo(13.2, 3.2)
    path.lineTo(17.1, 7.2)
    path.lineTo(17.1, 18.8)
    path.lineTo(5.4, 18.8)
    path.closeSubpath()
    painter.drawPath(path)
    painter.drawLine(QPointF(13.2, 3.2), QPointF(13.2, 7.2))
    painter.drawLine(QPointF(13.2, 7.2), QPointF(17.1, 7.2))


def _draw_folder(painter: QPainter, pen: QPen, accent: QColor) -> None:
    path = QPainterPath()
    path.moveTo(3.5, 7.2)
    path.lineTo(8.1, 7.2)
    path.lineTo(9.8, 5.2)
    path.lineTo(13.6, 5.2)
    path.lineTo(15.3, 7.2)
    path.lineTo(18.8, 7.2)
    path.lineTo(17.1, 18.3)
    path.lineTo(4.8, 18.3)
    path.closeSubpath()
    fill = QColor(accent)
    fill.setAlpha(28)
    painter.setPen(pen)
    painter.setBrush(QBrush(fill))
    painter.drawPath(path)
    painter.setBrush(Qt.BrushStyle.NoBrush)


def _draw_plus(painter: QPainter, pen: QPen, *, center: QPointF) -> None:
    painter.setPen(pen)
    painter.drawLine(QPointF(center.x() - 2.7, center.y()), QPointF(center.x() + 2.7, center.y()))
    painter.drawLine(QPointF(center.x(), center.y() - 2.7), QPointF(center.x(), center.y() + 2.7))


def _draw_arrow(painter: QPainter, pen: QPen, *, start: QPointF, end: QPointF) -> None:
    painter.setPen(pen)
    painter.drawLine(start, end)


def _draw_replace(painter: QPainter, line: QPen, detail: QPen) -> None:
    painter.setPen(line)
    painter.drawLine(QPointF(4.2, 7.2), QPointF(16.7, 7.2))
    painter.drawLine(QPointF(13.8, 4.4), QPointF(16.7, 7.2))
    painter.drawLine(QPointF(13.8, 10.0), QPointF(16.7, 7.2))
    painter.drawLine(QPointF(17.8, 14.8), QPointF(5.3, 14.8))
    painter.setPen(detail)
    painter.drawLine(QPointF(8.2, 12.1), QPointF(5.3, 14.8))
    painter.drawLine(QPointF(8.2, 17.5), QPointF(5.3, 14.8))


def _draw_sparkle(painter: QPainter, line: QPen, detail: QPen) -> None:
    painter.setPen(line)
    painter.drawLine(QPointF(11, 3.2), QPointF(11, 15.5))
    painter.drawLine(QPointF(5, 9.3), QPointF(17, 9.3))
    painter.setPen(detail)
    painter.drawLine(QPointF(17.1, 14.8), QPointF(17.1, 19))
    painter.drawLine(QPointF(15, 16.9), QPointF(19.2, 16.9))


def _draw_warning(painter: QPainter, pen: QPen, accent: QColor) -> None:
    path = QPainterPath()
    path.moveTo(11, 3.5)
    path.lineTo(19, 18.4)
    path.lineTo(3, 18.4)
    path.closeSubpath()
    fill = QColor(accent)
    fill.setAlpha(32)
    painter.setPen(pen)
    painter.setBrush(QBrush(fill))
    painter.drawPath(path)
    painter.setBrush(Qt.BrushStyle.NoBrush)
    painter.drawLine(QPointF(11, 8.2), QPointF(11, 13.3))
    painter.drawPoint(QPointF(11, 16.1))


def _resource_path(relative: str) -> Path:
    frozen_root = getattr(sys, "_MEIPASS", None)
    if isinstance(frozen_root, str):
        return Path(frozen_root) / relative
    return Path(__file__).resolve().parents[3] / relative
