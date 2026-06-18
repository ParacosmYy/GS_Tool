"""仪表盘 Gauge 控件（对齐 VOFA+ 仪表盘）。

自绘圆弧指针表盘，可绑定通道值，量程/单位可设。
支持刻度、指针、当前值大字号显示。

约束：本模块只依赖 PyQt6 + theme.palette，不访问 controller/transport。
"""

from __future__ import annotations

import math

from PyQt6.QtCore import QRectF, QSize, Qt
from PyQt6.QtGui import QColor, QConicalGradient, QFont, QPainter, QPen
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.theme import palette as P


class GaugeWidget(QWidget):
    """圆弧指针仪表盘。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationGauge")
        self._minimum = 0.0
        self._maximum = 100.0
        self._value = 0.0
        self._unit = ""
        self._label = ""
        self.setMinimumSize(QSize(120, 120))

    def set_range(self, minimum: float, maximum: float) -> None:
        self._minimum = minimum
        self._maximum = max(maximum, minimum + 1e-9)
        self.update()

    def set_value(self, value: float) -> None:
        self._value = value
        self.update()

    def set_unit(self, unit: str) -> None:
        self._unit = unit
        self.update()

    def set_label(self, label: str) -> None:
        self._label = label
        self.update()

    def value(self) -> float:
        return self._value

    def _value_to_angle(self, value: float) -> float:
        """值映射到角度（-210° 到 30°，即 240° 扫描范围）。"""

        clamped = max(self._minimum, min(self._maximum, value))
        ratio = (clamped - self._minimum) / (self._maximum - self._minimum)
        # 从 225° 顺时针扫到 -45°（即 315°），共 270°。
        return math.radians(225.0 - ratio * 270.0)

    def paintEvent(self, event: object) -> None:
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        rect = QRectF(self.rect()).adjusted(8, 8, -8, -8)
        cx = rect.center().x()
        cy = rect.center().y()
        radius = min(rect.width(), rect.height()) / 2.0 - 4.0
        if radius <= 0:
            return
        # 背景弧（灰）。
        pen = QPen(QColor(P.BG_DISABLED), 8)
        pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(pen)
        painter.drawArc(
            QRectF(cx - radius, cy - radius, radius * 2, radius * 2),
            int(math.radians(225.0) * 16),
            int(math.radians(-270.0) * 16),
        )
        # 值弧（渐变色）。
        ratio = (max(self._minimum, min(self._maximum, self._value)) - self._minimum) / (
            self._maximum - self._minimum
        )
        value_angle = self._value_to_angle(self._value)
        gradient = QConicalGradient(cx, cy, 225.0)
        gradient.setColorAt(0.0, QColor(P.ACCENT))
        gradient.setColorAt(0.5, QColor(P.SUCCESS))
        gradient.setColorAt(1.0, QColor(P.WARNING))
        pen = QPen(gradient if ratio > 0 else QColor(P.BG_DISABLED), 8)
        pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(pen)
        sweep = int((225.0 - (225.0 - ratio * 270.0)) * 16)
        painter.drawArc(
            QRectF(cx - radius, cy - radius, radius * 2, radius * 2),
            int(math.radians(225.0) * 16),
            -sweep,
        )
        # 指针。
        needle_len = radius - 12
        nx = cx + math.cos(value_angle) * needle_len
        ny = cy - math.sin(value_angle) * needle_len
        painter.setPen(QPen(QColor(P.TEXT_PRIMARY), 2))
        painter.drawLine(int(cx), int(cy), int(nx), int(ny))
        painter.setBrush(QColor(P.TEXT_PRIMARY))
        painter.drawEllipse(QRectF(cx - 4, cy - 4, 8, 8))
        # 数值文本。
        font = QFont()
        font.setPointSize(14)
        font.setBold(True)
        painter.setFont(font)
        painter.setPen(QColor(P.TEXT_PRIMARY))
        text = f"{self._value:.1f}{self._unit}"
        painter.drawText(
            QRectF(cx - radius, cy - radius * 0.3, radius * 2, radius * 0.6),
            Qt.AlignmentFlag.AlignCenter,
            text,
        )
        # 标签。
        if self._label:
            font.setPointSize(8)
            font.setBold(False)
            painter.setFont(font)
            painter.setPen(QColor(P.TEXT_MUTED))
            painter.drawText(
                QRectF(cx - radius, cy + radius * 0.3, radius * 2, radius * 0.4),
                Qt.AlignmentFlag.AlignCenter,
                self._label,
            )

    def sizeHint(self) -> QSize:
        return QSize(140, 140)
