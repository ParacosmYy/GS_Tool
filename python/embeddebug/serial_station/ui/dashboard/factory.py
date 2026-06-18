"""仪表盘控件工厂（按类型创建控件实例）。"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.controls import (
    CommandSlider,
    ConfigurableButton,
    GaugeWidget,
    StatusLed,
    ValueDisplay,
)


def create_widget(widget_type: str, parent: QWidget) -> QWidget:
    """按类型创建一个控件实例。

    不支持的类型回退到 StatusLed（保证拖拽不崩溃）。
    """

    if widget_type == "led":
        return StatusLed(parent)
    if widget_type == "slider":
        return CommandSlider(label="Param", parent=parent)
    if widget_type == "button":
        return ConfigurableButton(text="Action", parent=parent)
    if widget_type == "gauge":
        return GaugeWidget(parent)
    if widget_type == "value_display":
        return ValueDisplay(parent)
    return StatusLed(parent)


WIDGET_CATALOG: tuple[dict[str, str], ...] = (
    {"type": "led", "label": "Status LED", "icon": "circle"},
    {"type": "slider", "label": "Slider", "icon": "arrow-up-down"},
    {"type": "button", "label": "Button", "icon": "square"},
    {"type": "gauge", "label": "Gauge", "icon": "gauge"},
    {"type": "value_display", "label": "Value", "icon": "hash"},
)
