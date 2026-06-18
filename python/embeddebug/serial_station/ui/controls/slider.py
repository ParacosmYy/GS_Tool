"""命令滑块控件（对齐 VOFA+ 滑动条 + 命令绑定）。

滑块值变化时发出 command 信号，携带格式化命令文本，供 controller 发送。
支持整数/浮点范围、步进、单位标签、实时值显示。

约束：本模块只依赖 PyQt6 + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QSlider,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.ui.theme import palette as P


class CommandSlider(QWidget):
    """滑块 + 值显示 + 命令模板，值变化时发出 command 信号。"""

    command = pyqtSignal(str)

    def __init__(
        self,
        label: str = "",
        minimum: int = 0,
        maximum: int = 100,
        value: int = 0,
        command_template: str = "{value}",
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationCommandSlider")
        self._command_template = command_template
        self._formatter: Callable[[int], str] | None = None

        layout = QVBoxLayout(self)
        layout.setContentsMargins(4, 4, 4, 4)
        layout.setSpacing(4)

        header = QHBoxLayout()
        self._label = QLabel(label, self)
        self._label.setObjectName("serialStationSliderLabel")
        self._value_label = QLabel(str(value), self)
        self._value_label.setObjectName("serialStationSliderValue")
        self._value_label.setStyleSheet(f"color: {P.ACCENT}; font-family: monospace;")
        header.addWidget(self._label)
        header.addStretch(1)
        header.addWidget(self._value_label)
        layout.addLayout(header)

        self._slider = QSlider(Qt.Orientation.Horizontal, self)
        self._slider.setObjectName("serialStationSliderTrack")
        self._slider.setMinimum(minimum)
        self._slider.setMaximum(maximum)
        self._slider.setValue(value)
        self._slider.valueChanged.connect(self._on_value_changed)
        layout.addWidget(self._slider)

    def _on_value_changed(self, value: int) -> None:
        self._value_label.setText(str(value))
        command_text = self._build_command(value)
        self.command.emit(command_text)

    def _build_command(self, value: int) -> str:
        if self._formatter is not None:
            return self._formatter(value)
        return self._command_template.format(value=value)

    def value(self) -> int:
        return self._slider.value()

    def set_value(self, value: int) -> None:
        self._slider.setValue(value)

    def set_formatter(self, formatter: Callable[[int], str]) -> None:
        """设置自定义命令格式化函数（覆盖 command_template）。"""

        self._formatter = formatter

    def label(self) -> str:
        return self._label.text()
