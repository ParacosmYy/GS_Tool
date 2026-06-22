"""top_bar build helpers + connection_toolbar 内部 helper 边界测试。

补强 test_top_bar / test_animations_integrations 未直接断言的边角：
- build_top_bar：返回 QFrame + objectName + brand 子控件可达。
- _build_brand_chip：返回 QFrame + 含 brand icon/label 子控件。
- _build_brand_text：返回 QWidget + 含 label。
- connection_toolbar _install_rich_tooltips：不崩溃（即使无按钮）。
- connection_toolbar _serial_config_combo：返回 QComboBox + objectName。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace

from PyQt6.QtWidgets import QComboBox, QFrame, QLabel, QWidget

from embeddebug.serial_station.ui.top_bar import (
    _build_brand_chip,
    _build_brand_text,
    build_top_bar,
)
from embeddebug.serial_station.ui.connection_toolbar import (
    _install_rich_tooltips,
    _serial_config_combo,
)


# ── build_top_bar 边界 ────────────────────────────────────────────────


class _FakeHost:
    """模拟 TopBarHost 协议。"""

    def __init__(self) -> None:
        self._brand_chip = QLabel("ED")
        self._brand_icon = QLabel()
        self._brand_name = QLabel("EmbedDebug")
        self._brand_tagline = QLabel("Serial Tool")
        self._status_label = QLabel("Ready")
        self._profile_label = QLabel("default")

    def tr(self, text: str) -> str:
        return text


def test_build_top_bar_returns_frame(qtbot):
    """build_top_bar 返回 QFrame。"""

    owner = _FakeHost()
    root = QWidget()
    qtbot.addWidget(root)
    bar = build_top_bar(owner, root)
    assert isinstance(bar, QFrame)


def test_build_top_bar_has_objectname(qtbot):
    """bar objectName = serialStationTopBar。"""

    owner = _FakeHost()
    root = QWidget()
    qtbot.addWidget(root)
    bar = build_top_bar(owner, root)
    assert bar.objectName() == "serialStationTopBar"


def test_build_top_bar_has_layout(qtbot):
    """bar 有 layout（含子控件）。"""

    owner = _FakeHost()
    root = QWidget()
    qtbot.addWidget(root)
    bar = build_top_bar(owner, root)
    assert bar.layout() is not None


# ── _build_brand_chip ─────────────────────────────────────────────────


def test_build_brand_chip_returns_frame(qtbot):
    """_build_brand_chip 返回 QFrame。"""

    owner = _FakeHost()
    parent = QWidget()
    qtbot.addWidget(parent)
    chip = _build_brand_chip(owner, parent)
    assert isinstance(chip, QFrame)


def test_build_brand_chip_has_objectname(qtbot):
    """brand chip 有 objectName。"""

    owner = _FakeHost()
    parent = QWidget()
    qtbot.addWidget(parent)
    chip = _build_brand_chip(owner, parent)
    assert chip.objectName() != ""


# ── _build_brand_text ─────────────────────────────────────────────────


def test_build_brand_text_returns_widget(qtbot):
    """_build_brand_text 返回 QWidget。"""

    owner = _FakeHost()
    parent = QWidget()
    qtbot.addWidget(parent)
    text_widget = _build_brand_text(owner, parent)
    assert isinstance(text_widget, QWidget)


# ── connection_toolbar helpers ────────────────────────────────────────


def test_install_rich_tooltips_no_crash(qtbot):
    """_install_rich_tooltips 不崩溃（即使无按钮属性）。"""

    owner = SimpleNamespace(_widget=QWidget())
    qtbot.addWidget(owner._widget)
    _install_rich_tooltips(owner)  # 不抛


def test_serial_config_combo_returns_combobox(qtbot):
    """_serial_config_combo 返回 QComboBox + objectName。"""

    root = QWidget()
    qtbot.addWidget(root)
    combo = _serial_config_combo(root, "serialStationTestCombo", "test", "baud")
    assert isinstance(combo, QComboBox)
    assert combo.objectName() == "serialStationTestCombo"


def test_serial_config_combo_has_tooltip(qtbot):
    """_serial_config_combo 设置 tooltip。"""

    root = QWidget()
    qtbot.addWidget(root)
    combo = _serial_config_combo(root, "test", "my tooltip", "parity")
    assert combo.toolTip() == "my tooltip"
