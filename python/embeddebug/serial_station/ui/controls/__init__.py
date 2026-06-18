"""Serial Station 控件组件库（对齐 VOFA+ 控件目录）。

提供可复用的调试控件：状态 LED、滑块、可配置按钮、仪表盘、数值显示。
所有控件自包含、可独立测试，不依赖 controller/transport/protocol。
"""

from embeddebug.serial_station.ui.controls.gauge import GaugeWidget
from embeddebug.serial_station.ui.controls.led import LedState, StatusLed
from embeddebug.serial_station.ui.controls.ripple import RippleButton
from embeddebug.serial_station.ui.controls.slider import CommandSlider
from embeddebug.serial_station.ui.controls.status_dot import DotState, StatusDot
from embeddebug.serial_station.ui.controls.value_display import ValueDisplay
from embeddebug.serial_station.ui.controls.configurable_button import ConfigurableButton

__all__ = [
    "CommandSlider",
    "ConfigurableButton",
    "DotState",
    "GaugeWidget",
    "LedState",
    "RippleButton",
    "StatusDot",
    "StatusLed",
    "ValueDisplay",
]
