"""卡片化面板与三区分栏布局的单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QFrame, QLabel, QLineEdit, QPlainTextEdit, QSplitter, QWidget

from embeddebug.serial_station.ui.layout_cards import build_card, card_body, wrap_layout
from embeddebug.serial_station.ui.main_window import SerialStationMainWindow


def test_build_card_creates_frame_with_objectname(qtbot):
    from PyQt6.QtWidgets import QWidget

    parent = QWidget()
    qtbot.addWidget(parent)
    card, body = build_card(parent, title="Test", icon_name="plug")
    assert isinstance(card, QFrame)
    assert card.objectName() == "serialStationCard"
    assert card_body(card) is body


def test_build_card_without_title_has_only_body(qtbot):
    from PyQt6.QtWidgets import QWidget

    parent = QWidget()
    qtbot.addWidget(parent)
    card, body = build_card(parent)
    assert card_body(card) is body


def test_card_body_raises_for_non_card(qtbot):
    from PyQt6.QtWidgets import QWidget

    parent = QWidget()
    qtbot.addWidget(parent)
    try:
        card_body(parent)
        raise AssertionError("expected ValueError")
    except ValueError:
        pass


def test_wrap_layout_wraps_layout_into_card(qtbot):
    from PyQt6.QtWidgets import QHBoxLayout, QWidget

    parent = QWidget()
    qtbot.addWidget(parent)
    layout = QHBoxLayout()
    edit = QLineEdit(parent)
    layout.addWidget(edit)
    card = wrap_layout(parent, layout, title="Wrapped", icon_name="send")
    assert card.objectName() == "serialStationCard"
    assert card_body(card) is not None


def test_three_zone_layout_creates_splitter_with_three_zones(qtbot):
    from PyQt6.QtCore import Qt

    window = SerialStationMainWindow()
    qtbot.addWidget(window)

    splitter = window.findChild(QSplitter, "serialStationMainSplitter")
    assert splitter is not None
    assert splitter.orientation() == Qt.Orientation.Horizontal
    # 三区：left / center / right。
    assert splitter.count() == 3
    assert window.findChild(QWidget, "serialStationLeftZone") is not None
    assert window.findChild(QWidget, "serialStationCenterZone") is not None
    assert window.findChild(QWidget, "serialStationRightZone") is not None


def test_three_zone_layout_has_at_least_three_cards(qtbot):
    window = SerialStationMainWindow()
    qtbot.addWidget(window)
    cards = window.findChildren(QFrame, "serialStationCard")
    # Connection + Command + Waveform + Log + Log Tools + Profile = 至少 5 张卡。
    assert len(cards) >= 5


def test_three_zone_layout_preserves_critical_objectnames(qtbot):
    """冒烟测试硬断言的 9 个 objectName 在三区布局下仍可达。"""

    window = SerialStationMainWindow()
    qtbot.addWidget(window)
    from PyQt6.QtWidgets import QComboBox, QPushButton

    critical = {
        ("QPushButton", "serialStationConnectButton"),
        ("QLineEdit", "serialStationSendEdit"),
        ("QComboBox", "serialStationCommandHistoryCombo"),
        ("QPushButton", "serialStationSendButton"),
        ("QLineEdit", "serialStationInjectEdit"),
        ("QPushButton", "serialStationInjectButton"),
        ("QPushButton", "serialStationClearButton"),
        ("QPlainTextEdit", "serialStationLogView"),
        ("QLabel", "serialStationStatusLabel"),
    }
    type_map = {
        "QPushButton": QPushButton,
        "QLineEdit": QLineEdit,
        "QComboBox": QComboBox,
        "QPlainTextEdit": QPlainTextEdit,
        "QLabel": QLabel,
    }
    for type_name, object_name in critical:
        widget = window.findChild(type_map[type_name], object_name)
        assert widget is not None, f"missing critical objectName: {object_name}"


def test_three_zone_layout_log_view_inside_center_card(qtbot):
    """log_view 必须落在中区日志卡主体内。"""

    window = SerialStationMainWindow()
    qtbot.addWidget(window)
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    center = window.findChild(QWidget, "serialStationCenterZone")  # type: ignore[arg-type]
    assert log_view is not None
    assert center is not None
    # log_view 的祖先链应包含中区。
    ancestor = log_view.parent()
    found_center = False
    while ancestor is not None:
        if ancestor is center:
            found_center = True
            break
        ancestor = ancestor.parent()
    assert found_center, "log_view not nested under center zone"


def test_three_zone_splitter_has_three_resizable_children(qtbot):
    """三区 splitter 有 3 个可拉伸子区，且中区 widget 持有日志与波形卡。"""

    window = SerialStationMainWindow()
    qtbot.addWidget(window)
    splitter = window.findChild(QSplitter, "serialStationMainSplitter")
    assert splitter is not None
    assert splitter.count() == 3
    # 子区均非 collapsible（setChildrenCollapsible(False)）。
    assert splitter.childrenCollapsible() is False
    # 中区应同时包含波形面板和日志视图。
    center = window.findChild(QWidget, "serialStationCenterZone")
    assert center is not None
    assert window.findChild(QWidget, "serialStationWaveformPanel") is not None
    assert window.findChild(QPlainTextEdit, "serialStationLogView") is not None
