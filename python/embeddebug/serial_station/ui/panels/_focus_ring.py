"""域面板 focus_ring 装配 helper（Batch 22）。

域面板（OTA/BLE/RTT/CAN/Automation/Settings）的输入控件（QLineEdit/QComboBox 等）
此前未装 focus_ring 微交互（仅 serial workbench 经 button_icons.apply_focus_rings
全量装）。本 helper 复用 micro_interactions.install_focus_ring，给面板子树的可聚焦
输入控件装 focus 光环，激活该微交互在域面板的覆盖。

约束：只依赖 PyQt6 + micro_interactions，不访问 controller/transport/protocol。
"""

from __future__ import annotations

import logging

from PyQt6.QtWidgets import QWidget

_log = logging.getLogger(__name__)

# 需要装 focus_ring 的可聚焦输入控件类型（与 button_icons._FOCUSABLE_WIDGET_TYPES 对齐）。
_FOCUSABLE_WIDGET_TYPES = (
    "QLineEdit",
    "QComboBox",
    "QPlainTextEdit",
    "QSpinBox",
    "QDoubleSpinBox",
)


def apply_panel_focus_rings(root: QWidget) -> int:
    """给 root 子树所有可聚焦输入控件装 focus_ring，返回已装饰数量（Batch 22）。

    遍历 root 下所有 QLineEdit/QComboBox/QPlainTextEdit/QSpinBox/QDoubleSpinBox，
    调用 ``micro_interactions.install_focus_ring``。跳过只读控件（focus ring 对只读
    无意义）。装配失败静默（focus ring 是锦上添花，不阻塞面板构建）。

    Args:
        root: 面板顶层 QWidget（通常 build 返回的控件）。
    """

    from embeddebug.serial_station.ui.micro_interactions import install_focus_ring
    from PyQt6 import QtWidgets

    applied = 0
    for type_name in _FOCUSABLE_WIDGET_TYPES:
        cls = getattr(QtWidgets, type_name, None)
        if cls is None:
            continue
        for widget in root.findChildren(cls):
            if _is_readonly(widget):
                continue
            try:
                install_focus_ring(widget)
                applied += 1
            except Exception:
                _log.warning("panel focus ring install failed", exc_info=True)  # focus ring 失败不阻塞。
    return applied


def _is_readonly(widget) -> bool:
    """判断控件是否只读（focus ring 对只读控件无意义）。"""

    check = getattr(widget, "isReadOnly", None)
    if callable(check):
        try:
            return bool(check())
        except Exception:
            _log.warning("operation failed", exc_info=True)
            return False
    return False
