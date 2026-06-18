"""波形游标交互（双击添加 / 右键删除）— Batch 9-3。

把 CursorManager 的运行时交互（双击波形区添加 X 游标，右键弹出删除菜单）
从 SerialWaveformPreview 拆出，避免 waveform_preview.py 超 300 行门禁。

约束：只依赖 PyQt6 + pyqtgraph，不访问 controller/transport。
"""

from __future__ import annotations

import pyqtgraph as pg
from PyQt6.QtWidgets import QMenu, QWidget


def install_cursor_interactions(plot: pg.PlotWidget, cursor_manager_ref) -> None:
    """给 plot 装双击添加 X 游标 + 右键删除菜单。

    Args:
        plot: pyqtgraph PlotWidget。
        cursor_manager_ref: 返回当前 CursorManager 的 callable（惰性求值，
            因为 preview 首次 update_batch 才初始化 CursorManager）。
    """

    # cursor_manager_ref 可以是 lambda 返回 CursorManager 或 None。
    original_double_click = plot.mouseDoubleClickEvent
    original_context = plot.contextMenuEvent

    def _on_double_click(event):
        try:
            original_double_click(event)
        except Exception:
            pass
        cm = _resolve(cursor_manager_ref)
        if cm is None:
            return
        # Qt6: position() 返回 QPointF（.pos() 已弃用但仍可用，兼容兜底）。
        local = _event_local_point(event)
        if local is None:
            return
        mouse_point = plot.plotItem.vb.mapSceneToView(plot.mapToScene(local))
        cm.add_x_cursor(float(mouse_point.x()))
        event.accept()

    def _on_context(event):
        cm = _resolve(cursor_manager_ref)
        if cm is None or (cm.x_cursors == () and cm.y_cursors == ()):
            try:
                original_context(event)
            except Exception:
                pass
            return
        # Qt6: globalPos() 已移除，改用 globalPosition().toPoint()。
        global_point = _event_global_point(event)
        local = _event_local_point(event)
        if global_point is None or local is None:
            try:
                original_context(event)
            except Exception:
                pass
            return
        scene_pos = plot.mapToScene(local)
        mouse_point = plot.plotItem.vb.mapSceneToView(scene_pos)
        target_x = float(mouse_point.x())
        menu = QMenu(plot)
        menu.addAction(plot.tr("添加 X 游标 @{x:.2f}").format(x=target_x),
                       lambda: cm.add_x_cursor(target_x))
        # 删除现有 X 游标选项。
        for cursor in cm.x_cursors:
            cv = float(cursor.value())
            menu.addAction(plot.tr("删除 X 游标 @{x:.2f}").format(x=cv),
                           lambda c=cursor: cm.remove_cursor(c))
        menu.addAction(plot.tr("清除所有游标"), cm.clear)
        menu.exec(global_point)
        event.accept()

    # 替换 plot 的双击/右键 handler（必须在 install_cursor_interactions 内）。
    plot.mouseDoubleClickEvent = _on_double_click  # type: ignore[assignment]
    plot.contextMenuEvent = _on_context  # type: ignore[assignment]


def _event_local_point(event) -> object | None:
    """从鼠标事件提取局部 QPoint（Qt6 position()/Qt5 pos() 兼容）。"""

    if hasattr(event, "position"):
        try:
            return event.position().toPoint()
        except Exception:
            pass
    pos = getattr(event, "pos", None)
    if callable(pos):
        try:
            return pos()
        except Exception:
            return None
    return pos


def _event_global_point(event) -> object | None:
    """从鼠标事件提取全局 QPoint（Qt6 globalPosition()/Qt5 globalPos() 兼容）。"""

    if hasattr(event, "globalPosition"):
        try:
            return event.globalPosition().toPoint()
        except Exception:
            pass
    gpos = getattr(event, "globalPos", None)
    if callable(gpos):
        try:
            return gpos()
        except Exception:
            return None
    return gpos


def _resolve(ref) -> object | None:
    """解析 cursor_manager 引用（callable 或直接对象）。"""

    if callable(ref):
        try:
            return ref()
        except Exception:
            return None
    return ref
