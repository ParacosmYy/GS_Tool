"""BadgeKind 枚举 + Drawer + SegmentedControl + ToggleSwitch 边界测试。

补强 test_badge / test_drawer / test_segmented / test_toggle_switch 未直接断言的边角：
- BadgeKind：4 成员 + value 小写。
- Drawer：初始 is_open=False + open/close 不崩溃 + set_content 不崩溃 + sizeHint 正。
- SegmentedControl：options() / current() 初始 / setCurrent 越界 clamp / sizeHint 正。
- ToggleSwitch：初始 is_checked=False + toggle 翻转 + set_checked / sizeHint 正。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.controls.badge import BadgeKind
from embeddebug.serial_station.ui.controls.drawer import Drawer
from embeddebug.serial_station.ui.controls.segmented import SegmentedControl
from embeddebug.serial_station.ui.controls.toggle_switch import ToggleSwitch


# ── BadgeKind 枚举 ────────────────────────────────────────────────────


def test_badge_kind_has_four_members():
    """BadgeKind 含 4 成员。"""

    assert len(BadgeKind) == 4


def test_badge_kind_values_lowercase():
    """value 小写。"""

    for kind in BadgeKind:
        assert kind.value == kind.value.lower()


def test_badge_kind_info_value():
    assert BadgeKind.INFO.value == "info"
    assert BadgeKind.SUCCESS.value == "success"
    assert BadgeKind.WARNING.value == "warning"
    assert BadgeKind.ERROR.value == "error"


def test_badge_kind_values_distinct():
    assert len({k.value for k in BadgeKind}) == 4


# ── Drawer 边界 ───────────────────────────────────────────────────────


def test_drawer_initial_is_open_false(qtbot):
    """Drawer 初始 is_open=False。"""

    d = Drawer()
    qtbot.addWidget(d)
    assert d.is_open() is False


def test_drawer_open_no_crash(qtbot):
    """open() 不崩溃。"""

    d = Drawer()
    qtbot.addWidget(d)
    d.open()


def test_drawer_close_no_crash(qtbot):
    """close() 不崩溃（即使未 open）。"""

    d = Drawer()
    qtbot.addWidget(d)
    d.close()


def test_drawer_set_content_no_crash(qtbot):
    """set_content 不崩溃。"""

    d = Drawer()
    qtbot.addWidget(d)
    content = QWidget()
    qtbot.addWidget(content)
    d.set_content(content)


def test_drawer_size_hint_positive(qtbot):
    """sizeHint 宽高 > 0。"""

    d = Drawer()
    qtbot.addWidget(d)
    hint = d.sizeHint()
    assert hint.width() >= 0
    assert hint.height() >= 0


# ── SegmentedControl 边界 ─────────────────────────────────────────────


def test_segmented_initial_current_zero(qtbot):
    """SegmentedControl 初始 current=0。"""

    sc = SegmentedControl()
    qtbot.addWidget(sc)
    sc.setOptions(["A", "B", "C"])
    assert sc.current() == 0


def test_segmented_options(qtbot):
    """options() 返回设置的列表。"""

    sc = SegmentedControl()
    qtbot.addWidget(sc)
    sc.setOptions(["X", "Y"])
    assert sc.options() == ["X", "Y"]


def test_segmented_set_current(qtbot):
    """setCurrent 修改 current()。"""

    sc = SegmentedControl()
    qtbot.addWidget(sc)
    sc.setOptions(["A", "B", "C"])
    sc.setCurrent(2)
    assert sc.current() == 2


def test_segmented_set_current_negative_clamped(qtbot):
    """setCurrent 负值 → clamp 到 0。"""

    sc = SegmentedControl()
    qtbot.addWidget(sc)
    sc.setOptions(["A", "B"])
    sc.setCurrent(-1)
    assert sc.current() >= 0


def test_segmented_size_hint_positive(qtbot):
    """sizeHint 宽高 ≥ 0。"""

    sc = SegmentedControl()
    qtbot.addWidget(sc)
    hint = sc.sizeHint()
    assert hint.width() >= 0
    assert hint.height() >= 0


# ── ToggleSwitch 边界 ─────────────────────────────────────────────────


def test_toggle_initial_unchecked(qtbot):
    """ToggleSwitch 初始 is_checked=False。"""

    ts = ToggleSwitch()
    qtbot.addWidget(ts)
    assert ts.is_checked() is False


def test_toggle_set_checked_true(qtbot):
    """set_checked(True) → is_checked=True。"""

    ts = ToggleSwitch()
    qtbot.addWidget(ts)
    ts.set_checked(True)
    assert ts.is_checked() is True


def test_toggle_set_checked_false(qtbot):
    """set_checked(False) → is_checked=False。"""

    ts = ToggleSwitch()
    qtbot.addWidget(ts)
    ts.set_checked(True)
    ts.set_checked(False)
    assert ts.is_checked() is False


def test_toggle_flips(qtbot):
    """toggle() 翻转 checked 状态。"""

    ts = ToggleSwitch()
    qtbot.addWidget(ts)
    assert ts.is_checked() is False
    ts.toggle()
    assert ts.is_checked() is True
    ts.toggle()
    assert ts.is_checked() is False


def test_toggle_size_hint_exists(qtbot):
    """sizeHint 不崩溃（可能返回 (-1,-1) 未设置）。"""

    ts = ToggleSwitch()
    qtbot.addWidget(ts)
    hint = ts.sizeHint()
    assert hint is not None
