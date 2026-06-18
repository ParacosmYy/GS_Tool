"""Serial Station 可复用展示组件库。

提供 EmptyStateWidget（空状态）与 SkeletonWidget（骨架屏 shimmer），
用于面板/列表/数据区在无数据或加载中时的占位，替代朴素的双 QLabel 占位。

约束：只依赖 PyQt6 + theme + animations，不访问 controller/transport/protocol。
"""

from embeddebug.serial_station.ui.widgets.empty_state import EmptyStateWidget
from embeddebug.serial_station.ui.widgets.skeleton import SkeletonWidget
from embeddebug.serial_station.ui.widgets.toast import ToastWidget
from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer

__all__ = ["EmptyStateWidget", "SkeletonWidget", "ToastContainer", "ToastWidget"]
