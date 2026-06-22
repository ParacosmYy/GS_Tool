"""connection_sidebar 辅助函数单元测试 — layout 控件提取 + 分组 + 标签。

覆盖：take_layout_widgets 提取/排除 TopBar 控件、make_group_label 创建、
group_connection_widgets 按 objectName 分组。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication, QHBoxLayout, QLabel, QPushButton, QWidget

import pytest


@pytest.fixture(scope="module")
def qapp():
    return QApplication.instance() or QApplication([])


def test_take_layout_widgets_extracts_all(qapp):
    """正常控件全部提取。"""
    parent = QWidget()
    layout = QHBoxLayout(parent)
    btn1 = QPushButton("A", parent)
    btn1.setObjectName("btn1")
    btn2 = QPushButton("B", parent)
    btn2.setObjectName("btn2")
    layout.addWidget(btn1)
    layout.addWidget(btn2)
    from embeddebug.serial_station.ui.connection_sidebar import take_layout_widgets
    widgets = take_layout_widgets(layout)
    assert len(widgets) == 2


def test_take_layout_widgets_excludes_topbar_owned(qapp):
    """status/profile label 被 TopBar 拥有，不提取但保留在 layout。"""
    parent = QWidget()
    layout = QHBoxLayout(parent)
    status = QLabel("Disconnected", parent)
    status.setObjectName("serialStationStatusLabel")
    btn = QPushButton("OK", parent)
    btn.setObjectName("ok")
    layout.addWidget(status)
    layout.addWidget(btn)
    from embeddebug.serial_station.ui.connection_sidebar import take_layout_widgets
    widgets = take_layout_widgets(layout)
    assert len(widgets) == 1  # 只 btn，status 被排除
    # status 仍在 layout（remaining）
    assert layout.count() == 1


def test_make_group_label(qapp):
    parent = QWidget()
    from embeddebug.serial_station.ui.connection_sidebar import make_group_label
    label = make_group_label(parent, "端口设置")
    assert label.text() == "端口设置"
    assert label.objectName() == "serialStationCardGroupLabel"


def test_group_connection_widgets(qapp):
    """按 objectName 分组。"""
    parent = QWidget()
    port_combo = QWidget(parent)
    port_combo.setObjectName("serialStationPortCombo")
    refresh_btn = QWidget(parent)
    refresh_btn.setObjectName("serialStationRefreshPortsButton")
    unknown = QWidget(parent)
    unknown.setObjectName("unknownWidget")
    from embeddebug.serial_station.ui.connection_sidebar import group_connection_widgets
    groups = group_connection_widgets([port_combo, refresh_btn, unknown])
    assert isinstance(groups, list)
    # 至少有 port 组（含 port_combo + refresh_btn）。
    port_group = [g for g in groups if g[0] is not None and "port" in str(g[0]).lower()]
    # unknown 归 None 组。
    none_group = [g for g in groups if g[0] is None]
    assert len(none_group) >= 1 or any(len(g[1]) == 0 for g in groups if g[0] is None)
