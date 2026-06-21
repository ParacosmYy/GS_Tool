"""命令滑块控件（对齐 VOFA+ 滑动条 + 命令绑定）。

滑块值变化时发出 command 信号，携带格式化命令文本，供 controller 发送。
支持整数/浮点范围、步进、单位标签、实时值显示。

Batch 7-4 改进（诊断报告：Slider 原生无跟手 + valueChanged 刷屏发包）：
1. **跟手气泡**：拖拽时显示悬浮 value 气泡，跟随 handle 水平位置移动，
   拖拽结束隐藏。对齐 Linear/iOS 滑块拖拽反馈。
2. **release 发包语义**：拖拽过程中（sliderPressed→sliderReleased 之间）
   只更新本地值显示不发 command，sliderReleased 时才发一次最终命令，
   避免拖动过程刷屏式发包（原 valueChanged 每像素都发）。

约束：本模块只依赖 PyQt6 + 标准库 + theme，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QSlider,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


class CommandSlider(QWidget):
    """滑块 + 值显示 + 命令模板，值变化时发出 command 信号。

    Batch 7-4：拖拽时显示跟手 value 气泡，release 时才发 command（不刷屏）。
    """

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
        self._dragging = False  # 拖拽中标志（release 才发包）

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
        # Batch 7-4: 拖拽生命周期 — pressed 进入拖拽（抑制发包），released 发最终命令。
        self._slider.sliderPressed.connect(self._on_slider_pressed)
        self._slider.sliderReleased.connect(self._on_slider_released)
        self._slider.valueChanged.connect(self._update_bubble_position)
        layout.addWidget(self._slider)

        # Batch 7-4: 跟手气泡（拖拽时显示，跟随 handle 位置）。
        self._bubble = QLabel(str(value), self)
        self._bubble.setObjectName("serialStationSliderBubble")
        self._bubble.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._bubble.setStyleSheet(
            f"background-color: {P.ACCENT}; color: {P.TEXT_ON_ACCENT};"
            f"border-radius: 8px; padding: 2px 6px; font-family: monospace;"
            f"font-size: {T.FONT_XS}; font-weight: 600;"
        )
        self._bubble.setFixedHeight(18)
        self._bubble.hide()  # 默认隐藏，拖拽时显示

    def _on_slider_pressed(self) -> None:
        """进入拖拽：显示跟手气泡，标记拖拽中（抑制发包）。"""

        self._dragging = True
        self._bubble.setText(str(self._slider.value()))
        self._bubble.show()
        self._bubble.raise_()
        self._update_bubble_position(self._slider.value())

    def _on_slider_released(self) -> None:
        """结束拖拽：隐藏气泡，发送最终命令（release 发包语义）。"""

        self._dragging = False
        self._bubble.hide()
        # 拖拽结束发一次最终命令（拖拽过程中被抑制的）。
        value = self._slider.value()
        self.command.emit(self._build_command(value))

    def _update_bubble_position(self, value: int) -> None:
        """气泡跟随 handle 水平位置移动，并更新文字。"""

        # 先更新文字（无论气泡是否可见，保证状态正确）。
        self._bubble.setText(str(value))
        # 气泡隐藏时不计算位置（避免无意义计算）。
        if self._bubble.isHidden():
            return
        # 计算 handle 在 slider track 内的相对 X 位置。
        slider = self._slider
        if slider.maximum() == slider.minimum():
            ratio = 0.0
        else:
            ratio = (value - slider.minimum()) / (slider.maximum() - slider.minimum())
        # handle 可用区域（减去两端 handle 半宽的 padding）。
        handle_w = 14  # QSS handle 宽度
        track_w = max(1, slider.width() - handle_w)
        x = int(slider.x() + handle_w // 2 + ratio * track_w)
        # 气泡居中于 handle，位于 slider 上方。
        bubble_w = max(self._bubble.sizeHint().width(), 24)
        self._bubble.move(x - bubble_w // 2, slider.y() - 22)
        self._bubble.setFixedWidth(bubble_w)

    def _on_value_changed(self, value: int) -> None:
        """值变化：更新显示；拖拽中不发 command（release 才发）。"""

        self._value_label.setText(str(value))
        if self._dragging:
            # 拖拽中只更新本地显示与气泡，不发 command（避免刷屏）。
            return
        # 非拖拽场景（键盘/代码设值）正常发包。
        self.command.emit(self._build_command(value))

    def _build_command(self, value: int) -> str:
        if self._formatter is not None:
            return self._formatter(value)
        return self._command_template.format(value=value)

    def value(self) -> int:
        return self._slider.value()

    def set_value(self, value: int) -> None:
        """代码设值：非拖拽场景，正常发包。"""

        self._slider.setValue(value)

    def set_formatter(self, formatter: Callable[[int], str]) -> None:
        """设置自定义命令格式化函数（覆盖 command_template）。"""

        self._formatter = formatter

    def label(self) -> str:
        return self._label.text()
