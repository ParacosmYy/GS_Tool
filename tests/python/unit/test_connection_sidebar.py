"""connection_sidebar 辅助函数单元测试 — layout 控件提取 + 分组 + 标签。

覆盖：take_layout_widgets 提取/排除 TopBar 控件、make_group_label 创建、
group_connection_widgets 按 objectName 分组。
"""

from __future__ import annotations

import os
from types import SimpleNamespace

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication, QFrame, QHBoxLayout, QLabel, QPushButton, QWidget

import pytest


@pytest.fixture(scope="module")
def qapp():
    return QApplication.instance() or QApplication([])


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    return owner


def _make_toolbar(widgets):
    layout = QHBoxLayout()
    for widget in widgets:
        layout.addWidget(widget)
    return layout


def _card_label_texts(card):
    return [label.text() for label in card.findChildren(QLabel)]


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
    # unknown 归 None 组。
    none_group = [g for g in groups if g[0] is None]
    assert len(none_group) >= 1 or any(len(g[1]) == 0 for g in groups if g[0] is None)


def test_build_connection_card_empty_toolbar_returns_shared_card(qapp):
    """空 toolbar 不崩，返回共享卡片容器。"""
    parent = QWidget()
    from embeddebug.serial_station.ui.connection_sidebar import build_connection_card
    card = build_connection_card(_make_owner(), parent, QHBoxLayout())
    assert isinstance(card, QFrame)
    assert card.objectName() == "serialStationCard"
    assert card.findChildren(QLabel, "serialStationCardGroupLabel") == []


@pytest.mark.parametrize(
    ("object_name", "expected_label"),
    [
        ("serialStationPortCombo", "Port"),
        ("serialStationConnectButton", "Connect"),
    ],
)
def test_build_connection_card_keeps_common_groups_always_visible(qapp, object_name, expected_label):
    parent = QWidget()
    widget = QPushButton(expected_label, parent)
    widget.setObjectName(object_name)
    from embeddebug.serial_station.ui.connection_sidebar import build_connection_card
    card = build_connection_card(_make_owner(), parent, _make_toolbar([widget]))
    assert isinstance(card, QFrame)
    assert expected_label in _card_label_texts(card)
    assert card.findChild(QWidget, widget.objectName()) is widget


@pytest.mark.parametrize(
    ("object_name", "expected_title"),
    [
        ("serialStationBaudCombo", "Serial"),
        ("serialStationTcpHostEdit", "Endpoints"),
        ("unknownWidget", "Other"),
    ],
)
def test_build_connection_card_puts_low_frequency_groups_in_collapsible_cards(
    qapp, object_name, expected_title
):
    parent = QWidget()
    widget = QLabel(expected_title, parent)
    widget.setObjectName(object_name)
    from embeddebug.serial_station.ui.connection_sidebar import build_connection_card
    card = build_connection_card(_make_owner(), parent, _make_toolbar([widget]))
    collapsibles = card.findChildren(QFrame, "serialStationCollapsibleCard")
    assert len(collapsibles) == 1
    assert expected_title in _card_label_texts(card)
    assert collapsibles[0].findChild(QWidget, widget.objectName()) is widget


def test_build_connection_card_handles_mixed_groups(qapp):
    parent = QWidget()
    port = QWidget(parent)
    port.setObjectName("serialStationPortCombo")
    baud = QWidget(parent)
    baud.setObjectName("serialStationBaudCombo")
    mystery = QLabel("x", parent)
    mystery.setObjectName("unknownWidget")
    from embeddebug.serial_station.ui.connection_sidebar import build_connection_card
    card = build_connection_card(_make_owner(), parent, _make_toolbar([port, baud, mystery]))
    assert {"Port", "Serial", "Other"}.issubset(_card_label_texts(card))
    assert len(card.findChildren(QFrame, "serialStationCollapsibleCard")) == 2
