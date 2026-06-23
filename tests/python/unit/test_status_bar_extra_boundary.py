"""StatusBar set_section/clear_section/section_count 边界扩展测试。

test_status_bar 覆盖基础；本文件补 section_count 初始 + 多 section 顺序 +
clear_all + set_section 空文本 + section_text 多键查询。

覆盖：
1. section_count 初始 0。
2. set_section 空字符串不崩。
3. 多 section 有序（conn/clock/status）。
4. clear 所有 section 后 section_count 0。
5. section_text 查询多键。
6. set_section 同键多次更新不增加 count。
7. clear 后 section_text 返回空。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.status_bar import StatusBar


def test_section_count_initial_zero(qtbot):
    bar = StatusBar()
    qtbot.addWidget(bar)
    assert bar.section_count() == 0


def test_set_section_empty_string(qtbot):
    """set_section 空字符串不崩。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("empty", "")
    assert bar.section_text("empty") == ""


def test_multiple_sections_ordered(qtbot):
    """多 section 有序创建。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("conn", "COM3")
    bar.set_section("clock", "12:00:00")
    bar.set_section("status", "Connected")
    assert bar.section_count() == 3
    assert bar.section_text("conn") == "COM3"
    assert bar.section_text("clock") == "12:00:00"
    assert bar.section_text("status") == "Connected"


def test_clear_all_sections(qtbot):
    """逐个 clear 所有 section 后 count 0。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("a", "1")
    bar.set_section("b", "2")
    bar.clear_section("a")
    bar.clear_section("b")
    assert bar.section_count() == 0


def test_set_section_same_key_no_count_increase(qtbot):
    """同键多次更新不增加 count。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("x", "1")
    bar.set_section("x", "2")
    bar.set_section("x", "3")
    assert bar.section_count() == 1
    assert bar.section_text("x") == "3"


def test_section_text_after_clear_returns_empty(qtbot):
    """clear 后 section_text 返回空。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("temp", "value")
    bar.clear_section("temp")
    assert bar.section_text("temp") == ""


def test_clear_and_recreate_section(qtbot):
    """clear 后重新 set_section 创建新 section。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("key", "old")
    bar.clear_section("key")
    assert bar.section_count() == 0
    bar.set_section("key", "new")
    assert bar.section_count() == 1
    assert bar.section_text("key") == "new"
