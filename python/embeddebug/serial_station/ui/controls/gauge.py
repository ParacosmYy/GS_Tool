"""仪表盘 Gauge 控件（对齐 VOFA+ 仪表盘）。

自绘圆弧指针表盘，可绑定通道值，量程/单位可设。
支持刻度、指针、当前值大字号显示。

Batch 3 (B1) 改进：指针角度 tween —— ``set_value`` 时不再硬跳，而是用
``QPropertyAnimation`` 把 ``displayed_value`` 从旧值插值到新值（OutCubic，
240ms），指针平滑扫掠，符合 05-ui-standard 铁律 18（禁止突然出现/消失）的
数据可视化延伸：数值变化也应有过渡，而非瞬切。

约束：本模块只依赖 PyQt6 + theme.palette + animations，不访问 controller/transport。
"""

from __future__ import annotations

import math

from PyQt6.QtCore import QPropertyAnimation, QRectF, QSize, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QConicalGradient, QFont, QPainter, QPen
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


class GaugeWidget(QWidget):
    """圆弧指针仪表盘，指针平滑扫掠。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationGauge")
        self._minimum = 0.0
        self._maximum = 100.0
        self._value = 0.0
        # 实际绘制的值（tween 插值用），与 _value 分离：_value 是目标，_displayed 是当前绘制。
        self._displayed = 0.0
        self._unit = ""
        self._label = ""
        self._tween_enabled = True
        self._tween_anim: QPropertyAnimation | None = None
        self.setMinimumSize(QSize(120, 120))

    def set_range(self, minimum: float, maximum: float) -> None:
        self._minimum = minimum
        self._maximum = max(maximum, minimum + 1e-9)
        self.update()

    def set_value(self, value: float) -> None:
        """设置目标值；若 tween 开启则从当前显示值平滑插值到目标值。"""

        self._value = value
        if not self._tween_enabled:
            self._displayed = value
            self.update()
            return
        # 停止进行中的 tween，从当前显示值插值到新目标。
        if self._tween_anim is not None:
            self._tween_anim.stop()
        self._tween_anim = QPropertyAnimation(self, b"displayed_value", self)
        self._tween_anim.setDuration(AnimationTokens.DURATION_NORMAL)
        self._tween_anim.setStartValue(self._displayed)
        self._tween_anim.setEndValue(value)
        self._tween_anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._tween_anim.start()

    def set_tween(self, enabled: bool) -> None:
        """启用/禁用指针 tween（高频数据流场景可关闭以减少动画干扰）。"""

        self._tween_enabled = enabled
        if not enabled and self._tween_anim is not None:
            self._tween_anim.stop()
            self._displayed = self._value
            self.update()

    def set_unit(self, unit: str) -> None:
        self._unit = unit
        self.update()

    def set_label(self, label: str) -> None:
        self._label = label
        self.update()

    def value(self) -> float:
        return self._value

    # ── displayed_value 属性（tween 插值目标） ──────────────────────
    def _get_displayed(self) -> float:
        return self._displayed

    def _set_displayed(self, value: float) -> None:
        self._displayed = value
        self.update()

    displayed_value = pyqtProperty(float, _get_displayed, _set_displayed)

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
        # 值弧（渐变色）— 用 displayed 值驱动，与指针同步扫掠。
        disp_clamped = max(self._minimum, min(self._maximum, self._displayed))
        ratio = (disp_clamped - self._minimum) / (self._maximum - self._minimum)
        value_angle = self._value_to_angle(self._displayed)
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
        # 数值文本（显示 displayed 值，与指针同步跳动）。
        font = QFont()
        font.setPointSize(T.FONT_POINT_HEADING)
        font.setBold(True)
        painter.setFont(font)
        painter.setPen(QColor(P.TEXT_PRIMARY))
        text = f"{self._displayed:.1f}{self._unit}"
        painter.drawText(
            QRectF(cx - radius, cy - radius * 0.3, radius * 2, radius * 0.6),
            Qt.AlignmentFlag.AlignCenter,
            text,
        )
        # 标签。
        if self._label:
            font.setPointSize(T.FONT_POINT_TINY)
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
