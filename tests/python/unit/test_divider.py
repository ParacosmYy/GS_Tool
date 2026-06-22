"""Divider 分隔线测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.divider import Divider


def test_divider_objectname(qtbot):
    d = Divider()
    qtbot.addWidget(d)
    assert d.objectName() == "serialStationDivider"


def test_divider_no_label(qtbot):
    d = Divider()
    qtbot.addWidget(d)
    labels = d.findChildren(type(d), "serialStationDividerLabel")
    assert len(labels) == 0


def test_divider_with_label(qtbot):
    d = Divider("连接设置")
    qtbot.addWidget(d)
    d.findChild(type(d), "serialStationDividerLabel")
    # QLabel is not Divider type; check via findChildren with QLabel
    from PyQt6.QtWidgets import QLabel
    labels = d.findChildren(QLabel)
    assert len(labels) == 1
    assert labels[0].text() == "连接设置"


def test_divider_paint_no_raise(qtbot):
    d = Divider("测试")
    qtbot.addWidget(d)
    d.repaint()
