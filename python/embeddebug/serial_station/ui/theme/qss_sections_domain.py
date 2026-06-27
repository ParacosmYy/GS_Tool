"""QSS 分区生成器 — RTT/CAN/BLE/Automation/Settings 域面板共享样式。

覆盖各域面板的全部 serialStation* objectName（面板根/字段标签/状态/日志/按钮/
输入/表格/树/Tab）。用精确选择器统一覆盖，避免逐个写规则。

颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.qss_sections_domain_parts import (
    _about_labels,
    _accent_swatches,
    _checkboxes,
    _field_labels,
    _inputs,
    _main_buttons,
    _panel_roots,
    _secondary_buttons,
    _settings_tabs,
    _status_labels,
    _tables_and_trees,
    _text_views,
)


def domain_panels_section() -> str:
    """RTT/CAN/BLE/Automation/Settings 面板共享样式。

    由 12 个分区子助手拼接而成；本函数只做编排与连接，分区实体定义在
    ``qss_sections_domain_parts`` 中（保持每个子助手 ≤80 行）。
    """

    sections = [
        "/* === Domain Panels (RTT / CAN / BLE / Automation / Settings) === */",
        _panel_roots(),
        _field_labels(),
        _status_labels(),
        _text_views(),
        _main_buttons(),
        _secondary_buttons(),
        _inputs(),
        _checkboxes(),
        _tables_and_trees(),
        _settings_tabs(),
        _accent_swatches(),
        _about_labels(),
    ]
    return "\n\n".join(sections)
