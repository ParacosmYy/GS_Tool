"""仪表盘放置控件右键菜单 helper（Batch 31/32/49）。

放置的 dashboard 控件此前无删除入口（只有「清空画布」整批删）。本 helper 给每个
放置控件装右键菜单「复制控件」「删除」「配置数据源...」，调 canvas.add_widget_at /
remove_item / set_item_binding。

双击已被全屏占用（Batch 18），故用右键。

约束：只依赖 PyQt6 + canvas API，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import QPoint, Qt
from PyQt6.QtWidgets import QMenu, QWidget

# 复制控件时的偏移（避免与源控件完全重叠）。
_DUPLICATE_OFFSET = 20


def attach_widget_delete_menu(
    widget: QWidget,
    canvas,
    item_id: str,
    on_configure_binding: Callable[[str], None] | None = None,
) -> QMenu | None:
    """给放置的控件装右键菜单（复制/属性/置顶置底/锁定/配置数据源.../删除）。

    Args:
        widget: 放置的控件实例。
        canvas: 控件所在 DashboardCanvas（提供 remove_item / add_widget_at / items）。
        item_id: 控件在 canvas.items 的 key。
        on_configure_binding: 可选回调，提供时菜单加「配置数据源...」一项，
            点击时回调 ``on_configure_binding(item_id)``（Batch 49）。

    返回 None（菜单按需创建）；失败也返回 None（None 安全）。
    """

    if widget is None or canvas is None:
        return None
    widget.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)

    def _on_context(pos: QPoint) -> None:
        menu = QMenu(widget)
        menu.addAction(widget.tr("复制控件"), lambda: _safe_duplicate(canvas, item_id))
        menu.addAction(widget.tr("属性..."), lambda: _edit_properties(widget, canvas, item_id))
        # Batch 37: z-order 置顶/置底（控件重叠时调整叠放次序）。
        menu.addAction(widget.tr("置顶"), lambda: _safe_raise(widget))
        menu.addAction(widget.tr("置底"), lambda: _safe_lower(widget, canvas))
        # Batch 38: 锁定/解锁（防意外删除；config[locked] 持久化）。
        is_locked = _is_locked(canvas, item_id)
        lock_text = widget.tr("解锁") if is_locked else widget.tr("锁定")
        menu.addAction(lock_text, lambda: _toggle_lock(canvas, item_id))
        # Batch 49: 配置数据源...（仅在 panel 提供回调时出现）。
        if on_configure_binding is not None:
            menu.addAction(
                widget.tr("配置数据源..."),
                lambda: _safe_configure(on_configure_binding, item_id),
            )
        # 锁定控件禁用删除（防意外删）。
        delete_action = menu.addAction(widget.tr("删除控件"))
        delete_action.setEnabled(not is_locked)
        delete_action.triggered.connect(lambda _checked=False: _safe_remove(canvas, item_id))
        menu.exec(widget.mapToGlobal(pos))

    widget.customContextMenuRequested.connect(_on_context)
    return None  # 菜单按需创建，无需持有引用


def _safe_configure(
    on_configure_binding: Callable[[str], None] | None,
    item_id: str,
) -> None:
    """安全调用配置数据源回调（Batch 49）。"""

    if on_configure_binding is None:
        return
    try:
        on_configure_binding(item_id)
    except Exception:
        pass


def _edit_properties(widget: QWidget, canvas, item_id: str) -> None:
    """属性编辑：弹 QInputDialog 调整位置 X/Y（网格吸附）+ 宽/高（Batch 33/35）。

    调整后同步 widget.setGeometry + canvas.items[item_id].geometry/config，
    保持持久化（to_layout_dict 读 geometry）一致。任一步取消保持原值。
    """

    from PyQt6.QtCore import QRect
    from PyQt6.QtWidgets import QInputDialog
    from embeddebug.serial_station.ui.dashboard import snap_to_grid

    item = canvas.items.get(item_id) if canvas is not None else None
    if item is None:
        return
    geo = item.geometry
    # Batch 35: 位置 X（网格吸附）。
    new_x, ok = QInputDialog.getInt(
        widget, widget.tr("控件 X 坐标"), widget.tr("X（px，自动网格吸附）："),
        geo.x(), 0, 10000,
    )
    if not ok:
        return
    new_x = snap_to_grid(new_x)
    # 位置 Y（网格吸附）。
    new_y, ok = QInputDialog.getInt(
        widget, widget.tr("控件 Y 坐标"), widget.tr("Y（px，自动网格吸附）："),
        geo.y(), 0, 10000,
    )
    if not ok:
        return
    new_y = snap_to_grid(new_y)
    # Batch 33: 宽度。
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
    new_geo = QRect(new_x, new_y, new_w, new_h)
    try:
        widget.setGeometry(new_geo)
        try:
            from embeddebug.serial_station.ui.animations.elastic_snap import ElasticSnapAnimation
            ElasticSnapAnimation.snap_to(widget, new_geo).start()
        except Exception:
            pass
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
    """安全删除控件（remove_item 失败静默，避免竞态崩溃）。

    Batch 38：锁定控件（config[locked]=True）跳过删除（防意外删）。
    """

    try:
        if _is_locked(canvas, item_id):
            return  # 锁定控件不可删。
        canvas.remove_item(item_id)
    except Exception:
        pass


def _is_locked(canvas, item_id: str) -> bool:
    """控件是否锁定（config[locked]=True，Batch 38）。"""

    try:
        item = canvas.items.get(item_id) if canvas is not None else None
        if item is None:
            return False
        return bool(item.config.get("locked", False))
    except Exception:
        return False


def _toggle_lock(canvas, item_id: str) -> None:
    """切换控件锁定态（Batch 38）：config[locked] 翻转。"""

    try:
        item = canvas.items.get(item_id) if canvas is not None else None
        if item is None:
            return
        item.config["locked"] = not bool(item.config.get("locked", False))
    except Exception:
        pass


def _safe_raise(widget: QWidget) -> None:
    """置顶控件 z-order（Batch 37）。raise_ 把控件提到同 parent 兄弟最上层。"""

    try:
        widget.raise_()
    except Exception:
        pass


def _safe_lower(widget: QWidget, canvas) -> None:
    """置底控件 z-order（Batch 37）。lower 把控件压到同 parent 兄弟最下层。

    QWidget.lower() 是实例方法（非 lower_），压到 parent 栈底。
    """

    try:
        widget.lower()
    except Exception:
        pass


def make_grid_toggle(panel):
    """返回网格切换 slot（Batch 36）：切换当前画布网格 + 更新状态标签。

    由 DashboardPanel 的网格按钮 clicked.connect 调用，避免面板超 300 行门禁。
    """

    def _toggle(checked: bool) -> None:
        tabs = getattr(panel, "_tabs", None)
        widget = getattr(panel, "_widget", None)
        if tabs is None or widget is None:
            return
        canvas = tabs.current_canvas()
        if canvas is None:
            return
        canvas.set_show_grid(checked)
        panel._status.setText(
            widget.tr("网格已显示") if checked else widget.tr("网格已隐藏"))

    return _toggle
