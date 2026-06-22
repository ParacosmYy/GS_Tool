"""Settings 强调色选择行 helper 单元测试（_accent_row）。

Batch 10 抽出的设置页强调色选择行模块，守 settings_panel.py ≤ 300 行。
- build_accent_row(panel, parent)：构建 7 色点选择行。
- _sync_checked(panel, accent_id)：同步色点 checked 态。

覆盖：
- build_accent_row：返回 QHBoxLayout + 7 个 QToolButton 色点 + 第一个 label。
- 色点 objectName 序列 serialStationAccentSwatch{0..6}。
- 当前激活 accent 的色点 checked=True。
- _sync_checked：切换 checked 到指定 accent，其他取消。
- 色点 inline style 含 background-color（填色契约）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace

from PyQt6.QtWidgets import QLabel, QToolButton, QWidget

from embeddebug.serial_station.ui.panels._accent_row import (
    _sync_checked,
    build_accent_row,
)
from embeddebug.serial_station.ui.theme.accents import ACCENTS, get_active_accent_id


def _make_panel(qtbot) -> SimpleNamespace:
    """构造最小 SettingsPanel 替身（只需 _widget + _accent_swatches）。"""

    widget = QWidget()
    qtbot.addWidget(widget)
    return SimpleNamespace(_widget=widget, _accent_swatches=[])


def test_build_accent_row_creates_seven_swatches(qtbot):
    """build_accent_row 创建 7 个色点（ACCENTS 变体数）。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    row = build_accent_row(panel, parent)
    assert len(panel._accent_swatches) == len(ACCENTS)
    assert all(isinstance(s, QToolButton) for s in panel._accent_swatches)
    # 返回 QHBoxLayout（含 label + 7 色点 + stretch）。
    assert row.count() >= len(ACCENTS) + 1


def test_build_accent_row_first_widget_is_field_label(qtbot):
    """行首是「强调色」字段标签，objectName 走 settings field 契约。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    row = build_accent_row(panel, parent)
    label_item = row.itemAt(0)
    assert label_item is not None
    label = label_item.widget()
    assert isinstance(label, QLabel)
    assert label.objectName() == "serialStationSettingsFieldLabel"


def test_build_accent_row_swatches_have_sequential_objectnames(qtbot):
    """色点 objectName 序列 serialStationAccentSwatch{0..6}（QSS 契约）。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    for index, swatch in enumerate(panel._accent_swatches):
        assert swatch.objectName() == f"serialStationAccentSwatch{index}"


def test_build_accent_row_active_swatch_checked(qtbot):
    """当前激活 accent 的色点 checked=True，其他 False。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    active_id = get_active_accent_id()
    for index, variant in enumerate(ACCENTS):
        swatch = panel._accent_swatches[index]
        assert swatch.isChecked() == (variant.id == active_id)


def test_build_accent_row_swatches_have_inline_background(qtbot):
    """色点 inline style 含 background-color（填色契约，QSS 不覆盖）。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    for swatch in panel._accent_swatches:
        style = swatch.styleSheet()
        assert "background-color" in style
        assert "border-radius: 12px" in style


def test_build_accent_row_swatches_are_checkable_and_pointing_cursor(qtbot):
    """色点 setCheckable(True) + 指针光标（可达性）。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    for swatch in panel._accent_swatches:
        assert swatch.isCheckable() is True


def test_sync_checked_sets_target_accent_checked(qtbot):
    """_sync_checked 把目标 accent 色点置 checked，其他取消。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    # 选第一个非 active accent 作为目标。
    active_id = get_active_accent_id()
    target = next(v for v in ACCENTS if v.id != active_id)
    _sync_checked(panel, target.id)
    for index, variant in enumerate(ACCENTS):
        swatch = panel._accent_swatches[index]
        assert swatch.isChecked() == (variant.id == target.id)


def test_sync_checked_is_idempotent(qtbot):
    """重复 _sync_checked 同一 accent 结果一致。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    target_id = ACCENTS[0].id
    _sync_checked(panel, target_id)
    _sync_checked(panel, target_id)
    assert panel._accent_swatches[0].isChecked() is True
    for index in range(1, len(ACCENTS)):
        assert panel._accent_swatches[index].isChecked() is False


def test_sync_checked_handles_unknown_accent(qtbot):
    """未知 accent_id：所有色点 checked=False（无匹配）。"""

    panel = _make_panel(qtbot)
    parent = QWidget()
    qtbot.addWidget(parent)
    build_accent_row(panel, parent)
    _sync_checked(panel, "nonexistent-accent")
    for swatch in panel._accent_swatches:
        assert swatch.isChecked() is False
