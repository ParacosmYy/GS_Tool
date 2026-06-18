"""仪表盘标签页右键菜单 helper（Batch 29）。

从 DashboardPanel 拆出，避免 dashboard_panel.py 超 300 行门禁。提供：
- show_tab_context_menu(panel, pos)：弹右键菜单（重命名/复制/关闭）。
- duplicate_tab(panel, index)：克隆源画布全部控件到新标签页。
- close_tab_by_index(panel, index)：按 index 关闭标签页（至少保留一个）。

约束：只依赖 PyQt6 + dashboard 子系统公共 API，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import QPoint


def show_tab_context_menu(panel, pos: QPoint) -> None:
    """标签页右键菜单：重命名/复制标签页/关闭（Batch 29）。

    customContextMenuRequested 发的是相对于 QTabWidget 的坐标，映射到 tabBar
    后用 tabBar().tabAt(...) 定位右键所在标签页 index。
    """

    from PyQt6.QtWidgets import QMenu

    tabs = getattr(panel, "_tabs", None)
    widget = getattr(panel, "_widget", None)
    if tabs is None or widget is None:
        return
    bar = tabs.tabBar()
    local = bar.mapFrom(tabs, pos)
    index = bar.tabAt(local)
    if index < 0:
        return  # 未点中标签页，不弹菜单。
    menu = QMenu(widget)
    menu.addAction(widget.tr("重命名"), lambda: panel._rename_tab_on_double_click(index))
    menu.addAction(widget.tr("复制标签页"), lambda: duplicate_tab(panel, index))
    close_action = menu.addAction(widget.tr("关闭"))
    close_action.setEnabled(tabs.count() > 1)
    close_action.triggered.connect(
        lambda _checked=False: close_tab_by_index(panel, index)
    )
    menu.exec(bar.mapToGlobal(local))


def duplicate_tab(panel, index: int) -> None:
    """复制标签页：新建标签页并克隆源画布的全部控件（Batch 29）。"""

    tabs = getattr(panel, "_tabs", None)
    widget = getattr(panel, "_widget", None)
    if tabs is None or widget is None or not (0 <= index < tabs.count()):
        return
    src_canvas = tabs.widget(index) if hasattr(tabs, "widget") else None
    if src_canvas is None:
        return
    try:
        layout = src_canvas.to_layout_dict()
    except Exception:
        layout = {"items": []}
    new_name = f"{tabs.tabText(index)} copy"
    new_canvas = tabs.add_tab(new_name)
    for item in layout.get("items", []):
        config = dict(item.get("config", {}))
        config["width"] = item.get("width", 160)
        config["height"] = item.get("height", 80)
        try:
            new_canvas.add_widget_at(
                item["type"], QPoint(item["x"], item["y"]), config
            )
        except Exception:
            pass
    panel._status.setText(widget.tr("已复制标签页：{name}").format(name=new_name))
    panel._autosave_layout()


def close_tab_by_index(panel, index: int) -> None:
    """按 index 关闭标签页（Batch 29，至少保留一个）。"""

    tabs = getattr(panel, "_tabs", None)
    if tabs is None or tabs.count() <= 1:
        return
    if 0 <= index < tabs.count():
        canvas = tabs.widget(index) if hasattr(tabs, "widget") else None
        tabs.removeTab(index)
        if canvas is not None:
            canvas.setParent(None)
            canvas.deleteLater()
        widget = getattr(panel, "_widget", None)
        if widget is not None:
            panel._status.setText(widget.tr("已关闭标签页"))
        panel._autosave_layout()
