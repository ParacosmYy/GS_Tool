"""Batch 49-4: 仪表盘画布的空态覆盖层 helper。

独立模块，保持 ``dashboard/canvas.py`` 体积 ≤ 300 行（铁律 20）。
提供 ``build_canvas_empty_state(parent)``，返回配置好的 EmptyStateWidget
（图标 + 标题 + 描述），由 canvas 实例化为覆盖层，resizeEvent 居中跟随。

约束：只依赖 PyQt6 + widgets + theme，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.widgets import EmptyStateWidget


def build_canvas_empty_state(parent: QWidget) -> EmptyStateWidget:
    """构建画布空态占位（图标 + 标题 + 描述）。

    Args:
        parent: 父 widget（通常是 DashboardCanvas 实例）。EmptyStateWidget
            作为 parent 的覆盖层子控件，由 parent 的 resizeEvent 设定几何。

    Returns:
        配置好的 EmptyStateWidget（layout-dashboard 图标，"画布为空" 标题，
        拖拽提示描述）。调用方负责 show/show_with_fade/hide_with_fade。
    """

    return EmptyStateWidget(
        icon_name="layout-dashboard",
        title=parent.tr("画布为空"),
        description=parent.tr("从左侧拖入 LED / 滑块 / 仪表到画布开始构建仪表盘"),
        parent=parent,
    )
