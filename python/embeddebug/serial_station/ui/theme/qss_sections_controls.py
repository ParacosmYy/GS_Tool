"""QSS 分区生成器 — 控件组件库（LED/滑块/按钮/仪表盘/数值显示/Dashboard/微交互控件）。

颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。子分区实体定义在
``qss_sections_controls_parts`` 中（保持每个子助手 ≤80 行），本模块只做串联编排。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.qss_sections_controls_parts import (
    _auxiliary_widgets_part,
    _button_part,
    _dashboard_panel_part,
    _dashboard_part,
    _gauge_value_part,
    _led_part,
    _slider_part,
)


def controls_section() -> str:
    """控件分区 QSS：LED/滑块/按钮/仪表盘/数值/Dashboard/微交互控件。

    由 7 个分区子助手拼接而成；分区实体定义在 ``qss_sections_controls_parts``。
    """

    return "\n".join((
        "/* === Control Widgets (LED / Slider / Button / Gauge / Value) === */",
        _led_part(),
        _slider_part(),
        _button_part(),
        _gauge_value_part(),
        _dashboard_part(),
        _dashboard_panel_part(),
        _auxiliary_widgets_part(),
    ))
