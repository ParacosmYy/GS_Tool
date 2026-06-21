"""数值显示控件（对齐 VOFA+ 数值显示）。

大字号实时数值 + 单位 + 趋势箭头（上升/下降/持平）。
可绑定通道值，自动计算趋势。

约束：本模块只依赖 PyQt6 + theme.palette，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QColor, QFont
from PyQt6.QtWidgets import QHBoxLayout, QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T

TREND_UP = "▲"
TREND_DOWN = "▼"
TREND_FLAT = "◆"


class ValueDisplay(QWidget):
    """大字号实时数值 + 单位 + 趋势箭头。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationValueDisplay")
        self._value = 0.0
        self._previous = 0.0
        self._unit = ""
        self._label = ""

        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        layout.setSpacing(2)

        self._label_widget = QLabel("", self)
        self._label_widget.setObjectName("serialStationValueLabel")
        self._label_widget.setStyleSheet(f"color: {P.TEXT_MUTED}; font-size: {T.FONT_XS};")
        layout.addWidget(self._label_widget)

        row = QHBoxLayout()
        row.setSpacing(6)
        self._value_widget = QLabel("0.00", self)
        self._value_widget.setObjectName("serialStationValueNumber")
        font = QFont()
        font.setPointSize(20)
        font.setBold(True)
        self._value_widget.setFont(font)
        self._value_widget.setStyleSheet(f"color: {P.TEXT_PRIMARY};")
        row.addWidget(self._value_widget, 1)

        self._trend_widget = QLabel(TREND_FLAT, self)
        self._trend_widget.setObjectName("serialStationValueTrend")
        trend_font = QFont()
        trend_font.setPointSize(12)
        self._trend_widget.setFont(trend_font)
        self._trend_widget.setStyleSheet(f"color: {P.TEXT_MUTED};")
        row.addWidget(self._trend_widget)
        layout.addLayout(row)

    def set_label(self, label: str) -> None:
        self._label = label
        self._label_widget.setText(label)

    def set_unit(self, unit: str) -> None:
        self._unit = unit
        self._refresh()

    def set_value(self, value: float) -> None:
        self._previous = self._value
        self._value = value
        self._refresh()

    def value(self) -> float:
        return self._value

    def _refresh(self) -> None:
        self._value_widget.setText(f"{self._value:.2f}{self._unit}")
        delta = self._value - self._previous
        if abs(delta) < 1e-9:
            self._trend_widget.setText(TREND_FLAT)
            self._trend_widget.setStyleSheet(f"color: {P.TEXT_MUTED};")
        elif delta > 0:
            self._trend_widget.setText(TREND_UP)
            self._trend_widget.setStyleSheet(f"color: {P.SUCCESS};")
        else:
            self._trend_widget.setText(TREND_DOWN)
            self._trend_widget.setStyleSheet(f"color: {P.ERROR};")
