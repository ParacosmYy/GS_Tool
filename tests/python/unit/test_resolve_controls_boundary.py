"""waveform_cursor_interactions _resolve + StatusBar + ValueDisplay 边界测试。

补强 test_status_bar / test_controls_displays 未直接断言的边角：
- _resolve：callable 返回结果 / 非 callable 返回自身 / None 返回 None / callable 抛异常返回 None。
- StatusBar：section_count 初始 0 / set_section 创建+更新 / clear_section 删除 / section_text 未知名空串。
- ValueDisplay：value() 初始 0.0 / set_value 后读取 / set_label/set_unit 不崩溃。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.status_bar import StatusBar
from embeddebug.serial_station.ui.controls.value_display import ValueDisplay
from embeddebug.serial_station.ui.waveform_cursor_interactions import _resolve


# ── _resolve ──────────────────────────────────────────────────────────


class _FakeManager:
    """模拟 cursor_manager 对象。"""

    pass


def test_resolve_callable_returns_result():
    """callable ref → 返回 ref() 的结果。"""

    manager = _FakeManager()
    ref = lambda: manager
    assert _resolve(ref) is manager


def test_resolve_non_callable_returns_self():
    """非 callable ref → 返回 ref 自身。"""

    manager = _FakeManager()
    assert _resolve(manager) is manager


def test_resolve_none_returns_none():
    """None ref → None。"""

    assert _resolve(None) is None


def test_resolve_callable_raises_returns_none():
    """callable ref 抛异常 → None。"""

    def bad_ref():
        raise RuntimeError("boom")

    assert _resolve(bad_ref) is None


def test_resolve_callable_returns_none():
    """callable ref 返回 None → None。"""

    assert _resolve(lambda: None) is None


# ── StatusBar 边界 ────────────────────────────────────────────────────


def test_status_bar_initial_count_zero(qtbot):
    """新建 StatusBar section_count=0。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    assert bar.section_count() == 0


def test_status_bar_set_section_creates(qtbot):
    """set_section 新 key → section_count+1。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("connection", "COM3")
    assert bar.section_count() == 1
    assert bar.section_text("connection") == "COM3"


def test_status_bar_set_section_updates_existing(qtbot):
    """set_section 已有 key → 更新值，count 不增。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("connection", "COM3")
    bar.set_section("connection", "COM5")
    assert bar.section_count() == 1
    assert bar.section_text("connection") == "COM5"


def test_status_bar_clear_section(qtbot):
    """clear_section 删除 key → count-1。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.set_section("connection", "COM3")
    bar.clear_section("connection")
    assert bar.section_count() == 0
    assert bar.section_text("connection") == ""


def test_status_bar_section_text_unknown_empty(qtbot):
    """section_text 未知名 → 空串。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    assert bar.section_text("ghost") == ""


def test_status_bar_clear_unknown_no_crash(qtbot):
    """clear_section 未知名 → 不崩溃。"""

    bar = StatusBar()
    qtbot.addWidget(bar)
    bar.clear_section("ghost")  # 不抛


# ── ValueDisplay 边界 ─────────────────────────────────────────────────


def test_value_display_initial_value_zero(qtbot):
    """ValueDisplay 初始 value=0.0。"""

    vd = ValueDisplay()
    qtbot.addWidget(vd)
    assert vd.value() == 0.0


def test_value_display_set_value(qtbot):
    """set_value 后 value() 读取。"""

    vd = ValueDisplay()
    qtbot.addWidget(vd)
    vd.set_value(42.5)
    assert vd.value() == 42.5


def test_value_display_set_negative(qtbot):
    """set_value 负数。"""

    vd = ValueDisplay()
    qtbot.addWidget(vd)
    vd.set_value(-10.0)
    assert vd.value() == -10.0


def test_value_display_set_label(qtbot):
    """set_label 不崩溃。"""

    vd = ValueDisplay()
    qtbot.addWidget(vd)
    vd.set_label("温度")


def test_value_display_set_unit(qtbot):
    """set_unit 不崩溃。"""

    vd = ValueDisplay()
    qtbot.addWidget(vd)
    vd.set_unit("°C")


def test_value_display_set_label_and_unit_no_crash(qtbot):
    """同时 set_label + set_unit 不崩溃。"""

    vd = ValueDisplay()
    qtbot.addWidget(vd)
    vd.set_label("电压")
    vd.set_unit("V")
    vd.set_value(3.3)
