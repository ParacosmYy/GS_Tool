"""仪表盘放置控件右键删除菜单 helper（Batch 31）。

放置的 dashboard 控件此前无删除入口（只有「清空画布」整批删）。本 helper 给每个
放置控件装右键菜单「删除」，调 canvas.remove_item。双击已被全屏占用，故用右键删除。

约束：只依赖 PyQt6 + canvas.remove_item，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import QPoint, Qt
from PyQt6.QtWidgets import QMenu, QWidget


def attach_widget_delete_menu(widget: QWidget, canvas, item_id: str) -> QMenu | None:
    """给放置的控件装右键「删除」菜单（Batch 31）。

    Args:
        widget: 放置的控件实例。
        canvas: 控件所在 DashboardCanvas（提供 remove_item）。
        item_id: 控件在 canvas.items 的 key。

    返回 QMenu（已挂 customContextMenu），或 None 失败。
    """

    if widget is None or canvas is None:
        return None
    widget.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)

    def _on_context(pos: QPoint) -> None:
        menu = QMenu(widget)
        delete_action = menu.addAction(widget.tr("删除控件"))
        delete_action.triggered.connect(lambda _checked=False: _safe_remove(canvas, item_id))
        menu.exec(widget.mapToGlobal(pos))

    widget.customContextMenuRequested.connect(_on_context)
    return None  # 菜单按需创建，无需持有引用


def _safe_remove(canvas, item_id: str) -> None:
    """安全删除控件（remove_item 失败静默，避免竞态崩溃）。"""

    try:
        canvas.remove_item(item_id)
    except Exception:
        pass
