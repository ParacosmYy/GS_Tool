"""B49-4 ui_smoke: 仪表盘画布空态切换。

覆盖：
- 空态：画布无 widget 时显示 EmptyStateWidget（layout-dashboard 图标 + 标题）。
- 切换：首个 widget 放置后空态淡出；删除最后一个 widget 后空态重新淡入。
- clear：清空所有 widget 后空态重新显示。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint
from embeddebug.serial_station.ui.dashboard.canvas import DashboardCanvas


def _make_canvas(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.resize(400, 300)
    canvas.show()
    qtbot.waitUntil(lambda: canvas.isVisible(), timeout=1000)
    return canvas


def test_dashboard_empty_state_visible_on_startup(qtbot):
    """启动时画布应显示空态占位（layout-dashboard 图标 + 「画布为空」）。"""

    canvas = _make_canvas(qtbot)
    empty = canvas._empty_state
    assert empty is not None
    assert empty.objectName() == "serialStationEmptyState"
    title = empty._title_label.text()
    assert "画布为空" in title or "empty" in title.lower() or "canvas" in title.lower()


def test_dashboard_add_widget_hides_empty(qtbot):
    """放置首个 widget 后空态淡出。"""

    canvas = _make_canvas(qtbot)
    canvas.add_widget_at("led", QPoint(50, 50))
    qtbot.waitUntil(lambda: canvas._empty_state.isHidden(), timeout=2000)
    assert len(canvas.items) == 1


def test_dashboard_remove_last_widget_shows_empty(qtbot):
    """删除最后一个 widget 后空态重新淡入。"""

    canvas = _make_canvas(qtbot)
    item_id = canvas.add_widget_at("led", QPoint(50, 50))
    qtbot.waitUntil(lambda: canvas._empty_state.isHidden(), timeout=2000)

    canvas.remove_item(item_id)
    qtbot.waitUntil(lambda: not canvas._empty_state.isHidden(), timeout=2000)
    assert len(canvas.items) == 0


def test_dashboard_clear_restores_empty(qtbot):
    """clear() 清空所有 widget 后空态重新显示。"""

    canvas = _make_canvas(qtbot)
    canvas.add_widget_at("led", QPoint(20, 20))
    canvas.add_widget_at("gauge", QPoint(80, 80))
    qtbot.waitUntil(lambda: canvas._empty_state.isHidden(), timeout=2000)
    assert len(canvas.items) == 2

    canvas.clear()
    qtbot.waitUntil(lambda: not canvas._empty_state.isHidden(), timeout=2000)
    assert len(canvas.items) == 0


def test_dashboard_multiple_widgets_keep_empty_hidden(qtbot):
    """多个 widget 时空态保持隐藏（不因第二个 widget 重新显示）。"""

    canvas = _make_canvas(qtbot)
    canvas.add_widget_at("led", QPoint(20, 20))
    qtbot.waitUntil(lambda: canvas._empty_state.isHidden(), timeout=2000)
    canvas.add_widget_at("gauge", QPoint(80, 80))
    # 仍应隐藏。
    assert canvas._empty_state.isHidden()
    assert len(canvas.items) == 2
