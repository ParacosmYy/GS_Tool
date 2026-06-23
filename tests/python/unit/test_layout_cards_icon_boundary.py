"""layout_cards build_card icon_name + wrap_layout icon 边界测试。

test_layout_cards_helpers 覆盖基础 build_card/wrap_layout；
本文件补 icon_name 参数 + card_body objectName + _CARD constants。

覆盖：
1. build_card 带 icon_name 不崩。
2. build_card 返回的 card 有 objectName。
3. build_card 返回的 body 有 objectName。
4. build_card 带 title + icon_name 创建 header。
5. wrap_layout 带 icon_name 不崩。
6. _CARD_OBJECT_NAME 常量。
7. _CARD_BODY_OBJECT_NAME 常量。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QFrame, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.layout_cards import (
    _CARD_BODY_OBJECT_NAME,
    _CARD_OBJECT_NAME,
    build_card,
    card_body,
    wrap_layout,
)


def test_build_card_with_icon_name(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    card, _body = build_card(parent, title="Test", icon_name="cable")
    assert isinstance(card, QFrame)


def test_build_card_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    card, _body = build_card(parent, title="Test")
    assert card.objectName() == _CARD_OBJECT_NAME


def test_build_card_body_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    _, body = build_card(parent, title="Test")
    assert body.objectName() == _CARD_BODY_OBJECT_NAME


def test_build_card_title_and_icon(qtbot):
    """build_card 带 title + icon_name 创建 header。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card, _body = build_card(parent, title="My Card", icon_name="settings")
    # card 应有 header 子控件（title label）。
    from PyQt6.QtWidgets import QLabel

    labels = card.findChildren(QLabel)
    assert any("My Card" in l.text() for l in labels)


def test_wrap_layout_with_icon(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    layout = QVBoxLayout()
    card = wrap_layout(parent, layout, title="Wrapped", icon_name="cable")
    assert isinstance(card, QFrame)


def test_card_object_name_constant():
    assert _CARD_OBJECT_NAME.startswith("serialStation")


def test_card_body_object_name_constant():
    assert _CARD_BODY_OBJECT_NAME.startswith("serialStation")


def test_card_body_extracts_from_card(qtbot):
    """card_body 从 build_card 返回的 card 中提取 body layout。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card, original_body = build_card(parent, title="Test")
    extracted = card_body(card)
    assert extracted is original_body
