"""layout_cards 辅助函数单元测试 — build_card + card_body + wrap_layout。

覆盖：build_card 创建卡片 + body layout、card_body 按 objectName 提取、
wrap_layout 包装现成 layout。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication, QHBoxLayout, QLabel, QWidget

import pytest


@pytest.fixture(scope="module")
def qapp():
    return QApplication.instance() or QApplication([])


def test_build_card_returns_frame_and_body(qapp):
    parent = QWidget()
    from embeddebug.serial_station.ui.layout_cards import build_card
    card, body = build_card(parent, title="Test")
    assert card.objectName() == "serialStationCard"
    assert body is not None


def test_build_card_no_title(qapp):
    parent = QWidget()
    from embeddebug.serial_station.ui.layout_cards import build_card
    card, _ = build_card(parent)
    assert card.objectName() == "serialStationCard"


def test_card_body_extracts_layout(qapp):
    """card_body 从已建卡片提取 body layout。"""
    parent = QWidget()
    from embeddebug.serial_station.ui.layout_cards import build_card, card_body
    card, original_body = build_card(parent, title="X")
    extracted = card_body(card)
    assert extracted is original_body


def test_card_body_no_layout_raises(qapp):
    """无 layout 的 widget 抛 ValueError。"""
    parent = QWidget()
    from embeddebug.serial_station.ui.layout_cards import card_body
    bare = QWidget(parent)
    with pytest.raises(ValueError, match="has no layout"):
        card_body(bare)


def test_wrap_layout_returns_card(qapp):
    parent = QWidget()
    layout = QHBoxLayout()
    layout.addWidget(QLabel("x", parent))
    from embeddebug.serial_station.ui.layout_cards import wrap_layout
    card = wrap_layout(parent, layout, title="Wrapped")
    assert card.objectName() == "serialStationCard"
