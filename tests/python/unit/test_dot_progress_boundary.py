"""DotState 枚举 + StatusDot flash helpers + ProgressRing clamp/indeterminate 边界测试。

补强 test_status_dot / test_controls_displays 未直接断言的边角：
- DotState：5 成员 + value 小写 + _STATE_COLORS 5 键。
- StatusDot._get_flash/_set_flash：flash 属性 round-trip + 初始 0。
- ProgressRing：_clamp_value 超出 range clamp + setIndeterminate/isIndeterminate +
  setValue 负值/超 max clamp + setRange。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.status_dot import DotState, StatusDot
from embeddebug.serial_station.ui.controls.progress_ring import ProgressRing


# ── DotState 枚举 ─────────────────────────────────────────────────────


def test_dot_state_has_five_members():
    """DotState 含 5 成员。"""

    assert len(DotState) == 5


def test_dot_state_values_lowercase():
    """value 小写。"""

    for state in DotState:
        assert state.value == state.value.lower()


def test_dot_state_values_distinct():
    assert len({s.value for s in DotState}) == 5


def test_dot_state_known_values():
    assert DotState.OFF.value == "off"
    assert DotState.GREEN.value == "green"
    assert DotState.YELLOW.value == "yellow"
    assert DotState.RED.value == "red"
    assert DotState.BLUE.value == "blue"


# ── StatusDot _get_flash / _set_flash ─────────────────────────────────


def test_status_dot_flash_initial_zero(qtbot):
    """_get_flash 初始 = 0。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    assert dot._get_flash() == 0.0


def test_status_dot_flash_set_get(qtbot):
    """_set_flash / _get_flash round-trip。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot._set_flash(0.5)
    assert 0.49 < dot._get_flash() < 0.51


def test_status_dot_flash_zero(qtbot):
    """_set_flash(0) → _get_flash=0。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot._set_flash(0.0)
    assert dot._get_flash() == 0.0


# ── ProgressRing _clamp_value ─────────────────────────────────────────


def test_progress_ring_clamp_above_max(qtbot):
    """setValue > max → clamp 到 max。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setRange(0, 100)
    ring.setValue(200)
    assert ring.value() == 100


def test_progress_ring_clamp_below_min(qtbot):
    """setValue < min → clamp 到 min。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setRange(50, 100)
    ring.setValue(0)
    assert ring.value() == 50


def test_progress_ring_negative_clamp(qtbot):
    """负值 clamp 到 min。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setRange(0, 100)
    ring.setValue(-10)
    assert ring.value() == 0


def test_progress_ring_normal_value(qtbot):
    """正常值不 clamp。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setRange(0, 100)
    ring.setValue(50)
    assert ring.value() == 50


# ── ProgressRing indeterminate ────────────────────────────────────────


def test_progress_ring_indeterminate_default_false(qtbot):
    """初始 isIndeterminate=False。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    assert ring.isIndeterminate() is False


def test_progress_ring_set_indeterminate_true(qtbot):
    """setIndeterminate(True) → isIndeterminate=True。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setIndeterminate(True)
    assert ring.isIndeterminate() is True


def test_progress_ring_set_indeterminate_toggle(qtbot):
    """setIndeterminate 翻转。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setIndeterminate(True)
    assert ring.isIndeterminate() is True
    ring.setIndeterminate(False)
    assert ring.isIndeterminate() is False


# ── ProgressRing setRange ─────────────────────────────────────────────


def test_progress_ring_set_range(qtbot):
    """setRange 修改 min/max。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setRange(10, 90)
    assert ring.minimum() == 10
    assert ring.maximum() == 90


def test_progress_ring_set_minimum(qtbot):
    """setMinimum。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setMinimum(5)
    assert ring.minimum() == 5


def test_progress_ring_set_maximum(qtbot):
    """setMaximum。"""

    ring = ProgressRing()
    qtbot.addWidget(ring)
    ring.setMaximum(200)
    assert ring.maximum() == 200
