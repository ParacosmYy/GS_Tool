"""仪表盘控件工厂（按类型创建控件实例）。

Batch 49：新增 ``create_bound_widget`` / ``apply_binding``，把 binding spec
（来自对话框或布局 JSON）应用到新建或既有 widget 上。binding 解析逻辑放在
``services.dashboard_binding_service``，本模块只做 widget 配置（set_formatter /
set_command_template / 内部 QSlider 范围 / 动态属性）。

不修改既有 widget 源文件 —— 通过既有 public API + Qt dynamic property 完成。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.services.dashboard_binding_service import (
    ParsedBinding,
    parse_binding_spec,
)
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


def create_bound_widget(
    widget_type: str,
    binding_spec: str | None,
    parent: QWidget,
) -> QWidget:
    """创建控件并应用 binding spec（spec 为空 / 无效时退化为普通 create_widget）。"""

    widget = create_widget(widget_type, parent)
    if binding_spec:
        apply_binding(widget, widget_type, binding_spec)
    return widget


def apply_binding(widget: QWidget, widget_type: str, spec: str) -> bool:
    """把 binding spec 应用到既有控件上（Batch 49）。

    - ``value_display`` / ``gauge``: 把通道索引写到 dynamic property
      ``binding_channel``，由 Panel 在 route_measurement 时调 ``set_value``。
    - ``led``: 把目标 level 写到 ``binding_level``。
    - ``slider``: 设置 QSlider 范围 + ``set_formatter`` 让命令前缀拼值。
    - ``button``: 设置 ``command_template`` 为命令文本。

    始终把 raw spec 写到 ``binding_spec`` 属性，便于持久化。
    """

    parsed = parse_binding_spec(spec)
    if parsed is None:
        return False
    widget.setProperty("binding_spec", spec)
    _apply_parsed(widget, widget_type, parsed)
    return True


def _apply_parsed(
    widget: QWidget, widget_type: str, parsed: ParsedBinding
) -> None:
    """按 widget_type 把解析后的字段写到 widget 上。"""

    if widget_type in ("value_display", "gauge"):
        if parsed.channel is not None:
            widget.setProperty("binding_channel", parsed.channel)
        return
    if widget_type == "led":
        if parsed.level is not None:
            widget.setProperty("binding_level", parsed.level)
        return
    if widget_type == "slider":
        _apply_slider_binding(widget, parsed)
        return
    if widget_type == "button":
        if parsed.command:
            widget.set_command_template(parsed.command)  # type: ignore[attr-defined]
        return


def _apply_slider_binding(widget: QWidget, parsed: ParsedBinding) -> None:
    """slider 特殊：要同步设范围 + 命令 formatter。"""

    if not isinstance(widget, CommandSlider):
        return
    if parsed.minimum is not None and parsed.maximum is not None:
        try:
            widget._slider.setMinimum(parsed.minimum)  # type: ignore[attr-defined]
            widget._slider.setMaximum(parsed.maximum)  # type: ignore[attr-defined]
        except Exception:
            pass  # 竞态 / 类型不符 → 保留默认 0-100。
    if parsed.prefix:
        prefix = parsed.prefix
        widget.set_formatter(lambda value: f"{prefix} {value}")


def read_binding(widget: QWidget) -> str | None:
    """读取 widget 上次的 binding spec（来自 dynamic property，无则 None）。"""

    value = widget.property("binding_spec")
    if isinstance(value, str) and value:
        return value
    return None


WIDGET_CATALOG: tuple[dict[str, str], ...] = (
    {"type": "led", "label": "Status LED", "icon": "circle"},
    {"type": "slider", "label": "Slider", "icon": "arrow-up-down"},
    {"type": "button", "label": "Button", "icon": "square"},
    {"type": "gauge", "label": "Gauge", "icon": "gauge"},
    {"type": "value_display", "label": "Value", "icon": "hash"},
)
