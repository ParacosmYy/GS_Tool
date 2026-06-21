"""ToggleSwitch 滑动开关测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.toggle_switch import ToggleSwitch


def test_toggle_objectname(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    assert t.objectName() == "serialStationToggleSwitch"


def test_toggle_default_unchecked(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    assert not t.is_checked()


def test_toggle_set_checked(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    t.set_checked(True)
    assert t.is_checked()


def test_toggle_click_toggles(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    t.toggle()
    assert t.is_checked()
    t.toggle()
    assert not t.is_checked()


def test_toggle_emits_signal(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    received = []
    t.toggled.connect(lambda v: received.append(v))
    t.toggle()
    assert received == [True]


def test_toggle_set_checked_no_signal(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    received = []
    t.toggled.connect(lambda v: received.append(v))
    t.set_checked(True)
    assert received == []


def test_toggle_paint_no_raise(qtbot):
    t = ToggleSwitch()
    qtbot.addWidget(t)
    t.set_checked(True)
    t.repaint()
