"""Dashboard 画布空态覆盖层 helper 单元测试（_empty_state_overlay）。

Batch 49-4 抽出的小模块，保持 dashboard/canvas.py ≤ 300 行。
build_canvas_empty_state(parent) 返回配置好的 EmptyStateWidget。

覆盖：
- 返回 EmptyStateWidget 实例（类型契约）。
- objectName 走 EmptyStateWidget 默认（serialStationEmptyState）。
- icon_label / title_label / desc_label 文案正确（通过 QLabel.text()）。
- parent 关系正确（widget.parent() is parent）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.dashboard._empty_state_overlay import (
    build_canvas_empty_state,
)
from embeddebug.serial_station.ui.widgets import EmptyStateWidget


def test_build_canvas_empty_state_returns_empty_state_widget(qtbot):
    """build_canvas_empty_state 返回 EmptyStateWidget 实例。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    widget = build_canvas_empty_state(parent)
    qtbot.addWidget(widget)
    assert isinstance(widget, EmptyStateWidget)


def test_build_canvas_empty_state_sets_parent(qtbot):
    """返回的 widget 以 parent 为父级（覆盖层语义）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    widget = build_canvas_empty_state(parent)
    qtbot.addWidget(widget)
    assert widget.parent() is parent


def test_build_canvas_empty_state_title_label_text(qtbot):
    """title_label 文案是 parent.tr() 结果（翻译入口，铁律 19）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    widget = build_canvas_empty_state(parent)
    qtbot.addWidget(widget)
    # EmptyStateWidget 把 title 渲染到 _title_label。
    assert widget._title_label.text() == "画布为空"


def test_build_canvas_empty_state_description_guides_drag(qtbot):
    """desc_label 是拖拽引导文案。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    widget = build_canvas_empty_state(parent)
    qtbot.addWidget(widget)
    desc = widget._desc_label.text()
    assert "拖" in desc  # 引导用户拖入控件


def test_build_canvas_empty_state_objectname_contract(qtbot):
    """返回的 widget objectName 走 EmptyStateWidget 默认契约（QSS 覆盖）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    widget = build_canvas_empty_state(parent)
    qtbot.addWidget(widget)
    # EmptyStateWidget 默认 objectName = serialStationEmptyState。
    assert widget.objectName() == "serialStationEmptyState"


def test_build_canvas_empty_state_icon_label_objectname(qtbot):
    """icon_label / title_label / desc_label objectName 契约（QSS 覆盖）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    widget = build_canvas_empty_state(parent)
    qtbot.addWidget(widget)
    assert widget._icon_label.objectName() == "serialStationEmptyStateIcon"
    assert widget._title_label.objectName() == "serialStationEmptyStateTitle"
    assert widget._desc_label.objectName() == "serialStationEmptyStateDescription"
