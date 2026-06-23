"""CursorManager remove/clear/cursor_values 边界扩展测试。

test_waveform_core 覆盖基础 add/remove/clear/cursor_values；本文件补中间删除 +
混合 X/Y + cursor_values 部分清空 + clear 返回值 + 多次 clear。

覆盖：
1. remove_cursor 从 3 条 X 中间删除（保留首尾）。
2. remove_cursor 混合 X/Y（删 X 不影响 Y）。
3. cursor_values 部分清空后正确。
4. clear 返回 None。
5. clear 多次不崩。
6. cursor_values 空 manager 返回 ([], [])。
7. add_x/add_y 交替后 cursor_values 分离 X/Y。
8. remove_cursor 删除 Y 游标。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pyqtgraph as pg

from embeddebug.serial_station.ui.waveform_cursors import CursorManager


def _make_manager(qtbot):
    """构造带 plot 的 CursorManager。"""

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    return CursorManager(plot)


# ── remove_cursor 中间删除 ───────────────────────────────────────
def test_remove_cursor_middle_of_three(qtbot):
    """从 3 条 X 中删中间一条，保留首尾。"""

    manager = _make_manager(qtbot)
    c1 = manager.add_x_cursor(1.0)
    c2 = manager.add_x_cursor(2.0)
    c3 = manager.add_x_cursor(3.0)
    assert manager.remove_cursor(c2) is True
    assert len(manager.x_cursors) == 2
    # 首尾保留。
    assert manager.x_cursors[0] is c1
    assert manager.x_cursors[1] is c3


# ── remove_cursor 混合 X/Y ───────────────────────────────────────
def test_remove_x_does_not_affect_y(qtbot):
    """删 X 游标不影响 Y 游标。"""

    manager = _make_manager(qtbot)
    x = manager.add_x_cursor(1.0)
    manager.add_y_cursor(5.0)
    manager.remove_cursor(x)
    assert len(manager.x_cursors) == 0
    assert len(manager.y_cursors) == 1


def test_remove_y_cursor(qtbot):
    """删 Y 游标。"""

    manager = _make_manager(qtbot)
    y = manager.add_y_cursor(3.0)
    assert manager.remove_cursor(y) is True
    assert len(manager.y_cursors) == 0


# ── cursor_values 部分清空 ───────────────────────────────────────
def test_cursor_values_after_partial_remove(qtbot):
    """部分删除后 cursor_values 正确。"""

    manager = _make_manager(qtbot)
    c1 = manager.add_x_cursor(10.0)
    manager.add_x_cursor(20.0)
    manager.remove_cursor(c1)
    xs, ys = manager.cursor_values()
    assert xs == [20.0]
    assert ys == []


# ── clear 返回值 + 多次 ──────────────────────────────────────────
def test_clear_returns_none(qtbot):
    manager = _make_manager(qtbot)
    manager.add_x_cursor(1.0)
    result = manager.clear()
    assert result is None


def test_clear_multiple_times_no_crash(qtbot):
    manager = _make_manager(qtbot)
    manager.add_x_cursor(1.0)
    manager.clear()
    manager.clear()  # 空 manager 再 clear
    manager.clear()


# ── cursor_values 空 manager ─────────────────────────────────────
def test_cursor_values_empty_manager(qtbot):
    """空 manager cursor_values 返回 ([], [])。"""

    manager = _make_manager(qtbot)
    xs, ys = manager.cursor_values()
    assert xs == []
    assert ys == []


# ── add_x/add_y 交替 ─────────────────────────────────────────────
def test_add_x_y_alternating_cursor_values_separated(qtbot):
    """交替添加 X/Y 后 cursor_values 正确分离。"""

    manager = _make_manager(qtbot)
    manager.add_x_cursor(1.0)
    manager.add_y_cursor(2.0)
    manager.add_x_cursor(3.0)
    manager.add_y_cursor(4.0)
    xs, ys = manager.cursor_values()
    assert xs == [1.0, 3.0]
    assert ys == [2.0, 4.0]
