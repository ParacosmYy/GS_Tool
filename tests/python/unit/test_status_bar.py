"""StatusBar 多段状态栏测试（合并到 test_animations_controls.py 域不可行，放此文件）。

覆盖：
- objectName 合规
- set_section 新增 + 更新
- clear_section 移除
- section_text 查询
- section_count
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from embeddebug.serial_station.ui.controls.status_bar import StatusBar


def test_status_bar_objectname(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    assert bar.objectName() == "serialStationStatusBar"


def test_status_bar_fixed_height(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    assert bar.FIXED_HEIGHT == 24


def test_set_section_creates(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("conn", "COM3 @ 115200")
    assert bar.section_count() == 1
    assert bar.section_text("conn") == "COM3 @ 115200"


def test_set_section_updates_existing(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("conn", "COM3")
    bar.set_section("conn", "COM5 @ 9600")
    assert bar.section_count() == 1
    assert bar.section_text("conn") == "COM5 @ 9600"


def test_multiple_sections(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("conn", "Connected")
    bar.set_section("rx", "RX: 1KB")
    bar.set_section("tx", "TX: 256B")
    assert bar.section_count() == 3


def test_clear_section(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("conn", "COM3")
    bar.set_section("fps", "30fps")
    bar.clear_section("fps")
    assert bar.section_count() == 1
    assert bar.section_text("fps") == ""
    assert bar.section_text("conn") == "COM3"


def test_clear_nonexistent_noop(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.clear_section("nonexistent")
    assert bar.section_count() == 0


def test_section_text_nonexistent_empty(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    assert bar.section_text("nope") == ""
