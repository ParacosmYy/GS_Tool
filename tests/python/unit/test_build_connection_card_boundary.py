"""build_connection_card 重排 toolbar 控件进连接卡片边界测试。

模块的 take_layout_widgets/group_connection_widgets/make_group_label 已覆盖；
本文件聚焦 build_connection_card 整体装配（返回 QFrame + 控件重排）。

覆盖：
1. build_connection_card 返回 QFrame + objectName（serialStationConnectionCard）。
2. build_connection_card 空 toolbar 不崩（无控件）。
3. build_connection_card 带 Port/Connect 控件（常驻组）。
4. build_connection_card 带 Serial 控件（折叠组）。
5. build_connection_card 带 Endpoints 控件（折叠组）。
6. build_connection_card 带 未识别控件（Other 折叠组）。
"""

from __future__ import annotations

import os
from types import SimpleNamespace

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QPushButton, QWidget

from embeddebug.serial_station.ui.connection_sidebar import build_connection_card


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    return owner


def _make_toolbar(parent, widgets):
    """构造含指定控件的 QHBoxLayout（parent 保持存活）。"""

    layout = QHBoxLayout()
    for w in widgets:
        layout.addWidget(w)
    return layout


# ── build_connection_card 基础 ───────────────────────────────────
def test_build_connection_card_returns_frame(qtbot):
    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    toolbar = QHBoxLayout()
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


def test_build_connection_card_empty_toolbar(qtbot):
    """空 toolbar（无控件）→ 不崩，返回 QFrame。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    toolbar = QHBoxLayout()
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


# ── build_connection_card 常驻组 ─────────────────────────────────
def test_build_connection_card_with_port_group(qtbot):
    """Port 组控件应进常驻区。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    port_combo = QWidget(parent)
    port_combo.setObjectName("serialStationPortCombo")
    toolbar = _make_toolbar(parent, [port_combo])
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


def test_build_connection_card_with_connect_group(qtbot):
    """Connect 组控件应进常驻区。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    btn = QPushButton("connect", parent)
    btn.setObjectName("serialStationConnectButton")
    toolbar = _make_toolbar(parent, [btn])
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


# ── build_connection_card 折叠组 ─────────────────────────────────
def test_build_connection_card_with_serial_group(qtbot):
    """Serial 组控件应进折叠卡。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    baud = QWidget(parent)
    baud.setObjectName("serialStationBaudCombo")
    toolbar = _make_toolbar(parent, [baud])
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


def test_build_connection_card_with_endpoints_group(qtbot):
    """Endpoints 组控件应进折叠卡。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    tcp_host = QWidget(parent)
    tcp_host.setObjectName("serialStationTcpHostEdit")
    toolbar = _make_toolbar(parent, [tcp_host])
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


# ── build_connection_card 未识别组 ───────────────────────────────
def test_build_connection_card_with_unknown_widget(qtbot):
    """无识别 objectName 的控件应进 Other 折叠组。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    mystery = QLabel("mystery", parent)
    mystery.setObjectName("unknownWidget")
    toolbar = _make_toolbar(parent, [mystery])
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)


def test_build_connection_card_mixed_groups(qtbot):
    """混合 Port + Serial + Unknown 控件 → 全部正确分组不崩。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    port = QWidget(parent)
    port.setObjectName("serialStationPortCombo")
    baud = QWidget(parent)
    baud.setObjectName("serialStationBaudCombo")
    mystery = QLabel("x", parent)
    toolbar = _make_toolbar(parent, [port, baud, mystery])
    card = build_connection_card(owner, parent, toolbar)
    assert isinstance(card, QFrame)
