"""仪表盘放置控件右键菜单 helper（Batch 31/32）。

放置的 dashboard 控件此前无删除入口（只有「清空画布」整批删）。本 helper 给每个
放置控件装右键菜单「复制控件」「删除」，调 canvas.add_widget_at / remove_item。
双击已被全屏占用（Batch 18），故用右键。

约束：只依赖 PyQt6 + canvas API，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import QPoint, Qt
from PyQt6.QtWidgets import QMenu, QWidget

# 复制控件时的偏移（避免与源控件完全重叠）。
_DUPLICATE_OFFSET = 20


def attach_widget_delete_menu(widget: QWidget, canvas, item_id: str) -> QMenu | None:
    """给放置的控件装右键菜单（复制控件/删除）（Batch 31/32）。

    Args:
        widget: 放置的控件实例。
        canvas: 控件所在 DashboardCanvas（提供 remove_item / add_widget_at / items）。
        item_id: 控件在 canvas.items 的 key。

    返回 None（菜单按需创建）；失败也返回 None（None 安全）。
    """

    if widget is None or canvas is None:
        return None
    widget.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)

    def _on_context(pos: QPoint) -> None:
        menu = QMenu(widget)
        menu.addAction(widget.tr("复制控件"), lambda: _safe_duplicate(canvas, item_id))
        menu.addAction(widget.tr("属性..."), lambda: _edit_properties(widget, canvas, item_id))
        delete_action = menu.addAction(widget.tr("删除控件"))
        delete_action.triggered.connect(lambda _checked=False: _safe_remove(canvas, item_id))
        menu.exec(widget.mapToGlobal(pos))

    widget.customContextMenuRequested.connect(_on_context)
    return None  # 菜单按需创建，无需持有引用


def _edit_properties(widget: QWidget, canvas, item_id: str) -> None:
    """属性编辑：弹 QInputDialog 调整宽/高（Batch 33）。

    调整后同步 widget.setGeometry + canvas.items[item_id].geometry/config，
    保持持久化（to_layout_dict 读 geometry）一致。
    """

    from PyQt6.QtCore import QRect
    from PyQt6.QtWidgets import QInputDialog

    item = canvas.items.get(item_id) if canvas is not None else None
    if item is None:
        return
    geo = item.geometry
    # 宽度。
    new_w, ok = QInputDialog.getInt(
        widget, widget.tr("控件宽度"), widget.tr("宽度（px）："), geo.width(), 40, 2000,
    )
    if not ok:
        return
    # 高度。
    new_h, ok = QInputDialog.getInt(
        widget, widget.tr("控件高度"), widget.tr("高度（px）："), geo.height(), 30, 2000,
    )
    if not ok:
        return
    new_geo = QRect(geo.x(), geo.y(), new_w, new_h)
    try:
        widget.setGeometry(new_geo)
        item.geometry = new_geo
        item.config["width"] = new_w
        item.config["height"] = new_h
    except Exception:
        pass


def _safe_duplicate(canvas, item_id: str) -> None:
    """复制控件：在源位置偏移 _DUPLICATE_OFFSET 处克隆同类型控件（Batch 32）。

    从 canvas.items 取源 DashboardItem（含 widget_type + geometry + config），
    调 add_widget_at 重建。失败静默（item 已被删等竞态）。
    """

    try:
        item = canvas.items.get(item_id)
        if item is None:
            return
        geo = item.geometry
        offset_pos = QPoint(geo.x() + _DUPLICATE_OFFSET, geo.y() + _DUPLICATE_OFFSET)
        config = dict(item.config) if item.config else {}
        config["width"] = geo.width()
        config["height"] = geo.height()
        canvas.add_widget_at(item.widget_type, offset_pos, config)
    except Exception:
        pass


def _safe_remove(canvas, item_id: str) -> None:
    """安全删除控件（remove_item 失败静默，避免竞态崩溃）。"""

    try:
        canvas.remove_item(item_id)
    except Exception:
        pass
