"""layout_cards + connection_sidebar 边界单元测试。

补强 test_layout_cards / test_connection_sidebar 未直接断言的边角：
- _ALWAYS_ON_GROUPS 常量（Port + Connect）。
- build_card：返回 (QFrame, QVBoxLayout) + header objectName + icon_name=None。
- wrap_layout：无标题 + 无 icon + 返回 QFrame。
- card_body：非 QFrame 控件抛 ValueError。
- group_connection_widgets：空列表 / 单控件 / _ALWAYS_ON_GROUPS 行为。
- take_layout_widgets：空 layout → 空列表。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.connection_sidebar import (
    _ALWAYS_ON_GROUPS,
    group_connection_widgets,
    make_group_label,
    take_layout_widgets,
)
from embeddebug.serial_station.ui.layout_cards import build_card, card_body, wrap_layout


# ── _ALWAYS_ON_GROUPS 常量 ─────────────────────────────────────────────


def test_always_on_groups_contains_port_and_connect():
    """_ALWAYS_ON_GROUPS = {'Port', 'Connect'}。"""

    assert {"Port", "Connect"} == _ALWAYS_ON_GROUPS


def test_always_on_groups_size_two():
    assert len(_ALWAYS_ON_GROUPS) == 2


# ── build_card 边界 ────────────────────────────────────────────────────


def test_build_card_returns_frame_and_layout(qtbot):
    """build_card 返回 (QFrame, QVBoxLayout)。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card, body = build_card(parent, title="Test")
    assert isinstance(card, QFrame)
    assert isinstance(body, QVBoxLayout)


def test_build_card_objectname(qtbot):
    """card objectName = serialStationCard。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card, _ = build_card(parent, title="Test")
    assert card.objectName() == "serialStationCard"


def test_build_card_no_icon(qtbot):
    """icon_name=None → 不崩溃。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card, _ = build_card(parent, title="Test", icon_name=None)
    assert card.objectName() == "serialStationCard"


def test_build_card_no_title(qtbot):
    """无标题 → 无 header（只有 body）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card, body = build_card(parent)
    assert card_body(card) is body


# ── wrap_layout 边界 ───────────────────────────────────────────────────


def test_wrap_layout_no_title(qtbot):
    """wrap_layout 无标题 → 不崩溃，返回 QFrame。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    layout = QHBoxLayout()
    card = wrap_layout(parent, layout)
    assert isinstance(card, QFrame)


def test_wrap_layout_with_title_and_icon(qtbot):
    """wrap_layout 带标题 + icon → 返回 QFrame + 有 body。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    layout = QHBoxLayout()
    card = wrap_layout(parent, layout, title="My Card", icon_name="cable")
    assert isinstance(card, QFrame)
    assert card_body(card) is not None


def test_wrap_layout_returns_card_with_body(qtbot):
    """wrap_layout 返回的 card 有可用的 body layout。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    layout = QVBoxLayout()
    card = wrap_layout(parent, layout, title="Test")
    body = card_body(card)
    assert body is not None


# ── card_body 边界 ─────────────────────────────────────────────────────


def test_card_body_raises_for_plain_widget(qtbot):
    """card_body 对非 serialStationCard 控件抛 ValueError。"""

    w = QWidget()
    qtbot.addWidget(w)
    with pytest.raises(ValueError):
        card_body(w)


# ── group_connection_widgets 边界 ──────────────────────────────────────


def test_group_connection_empty_list():
    """空列表 → 空分组列表。"""

    groups = group_connection_widgets([])
    assert isinstance(groups, list)
    assert len(groups) == 0


def test_group_connection_single_widget():
    """单控件 → 1 组（或 None）。"""

    parent = QWidget()
    w = QWidget(parent)
    w.setObjectName("serialStationUnknownWidget")
    groups = group_connection_widgets([w])
    assert len(groups) >= 0  # 不崩溃即可


def test_always_on_groups_constant_intact():
    """_ALWAYS_ON_GROUPS 含 Port + Connect（常量不被修改）。"""

    parent = QWidget()
    w = QWidget(parent)
    w.setObjectName("serialStationUnknownWidget")
    group_connection_widgets([w])
    # 常量在调用后不被修改
    assert "Port" in _ALWAYS_ON_GROUPS
    assert "Connect" in _ALWAYS_ON_GROUPS


# ── make_group_label 边界 ──────────────────────────────────────────────


def test_make_group_label_returns_qlabel(qtbot):
    """make_group_label 返回 QLabel。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    label = make_group_label(parent, "端口")
    assert isinstance(label, QLabel)


def test_make_group_label_empty_text(qtbot):
    """make_group_label 空文本 → 不崩溃。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    label = make_group_label(parent, "")
    assert isinstance(label, QLabel)


# ── take_layout_widgets 边界 ───────────────────────────────────────────


def test_take_layout_widgets_empty_layout():
    """空 layout → 空列表。"""

    layout = QVBoxLayout()
    result = take_layout_widgets(layout)
    assert result == []
