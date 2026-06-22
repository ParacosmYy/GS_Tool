"""仪表盘子系统（对齐 VOFA+ 拖拽式 GUI）。

提供可拖拽装配的仪表盘：控件画布 + 控件库面板 + 多标签页 + 布局持久化。
"""

from embeddebug.serial_station.ui.dashboard.canvas import (
    GRID_SIZE,
    SUPPORTED_WIDGET_TYPES,
    DashboardCanvas,
    DashboardItem,
    snap_to_grid,
)
from embeddebug.serial_station.ui.dashboard.factory import (
    WIDGET_CATALOG,
    create_widget,
)
from embeddebug.serial_station.ui.dashboard.fullscreen import (
    WidgetFullscreenHandler,
    attach_double_click_fullscreen,
)
from embeddebug.serial_station.ui.dashboard.palette import (
    MIME_TYPE,
    WidgetPalette,
    WidgetPaletteButton,
)
from embeddebug.serial_station.ui.dashboard.tabs import DashboardTabs

__all__ = [
    "GRID_SIZE",
    "MIME_TYPE",
    "SUPPORTED_WIDGET_TYPES",
    "WIDGET_CATALOG",
    "DashboardCanvas",
    "DashboardItem",
    "DashboardTabs",
    "WidgetFullscreenHandler",
    "WidgetPalette",
    "WidgetPaletteButton",
    "attach_double_click_fullscreen",
    "create_widget",
    "snap_to_grid",
]
