"""SegmentedControl 分段控件单元测试。

覆盖：
- objectName / 默认 current。
- setCurrent 钳制、信号发射（重复 setCurrent 同索引不发射）。
- setOptions 重置 current。
- sizeHint 随选项数变宽。
- 鼠标点击切换 current。
- paintEvent 不抛异常。
- 指示器动画：setCurrent 创建动画，手动推进到 duration 后指示器到达目标段。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


from PyQt6.QtCore import QPointF, Qt
from PyQt6.QtGui import QMouseEvent
from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.controls.segmented import SegmentedControl


def _make_seg(qtbot, options: tuple[str, ...] = ("A", "B", "C")) -> SegmentedControl:
    """构造一个固定尺寸的 SegmentedControl（200x32）用于测试。"""

    seg = SegmentedControl(list(options))
    seg.resize(200, 32)
    qtbot.addWidget(seg)
    return seg


def _click_at(seg: SegmentedControl, x: float) -> None:
    """在 seg 局部坐标 x 处模拟左键按下。"""

    pos = QPointF(x, seg.height() / 2.0)
    event = QMouseEvent(
        QMouseEvent.Type.MouseButtonPress,
        pos,
        pos,
        Qt.MouseButton.LeftButton,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
    )
    seg.mousePressEvent(event)


# ── 基础属性 ────────────────────────────────────────────────────────
def test_seg_objectname(qtbot):
    seg = _make_seg(qtbot)
    assert seg.objectName() == "serialStationSegmentedControl"


def test_seg_default_current_zero(qtbot):
    seg = _make_seg(qtbot)
    assert seg.current() == 0


# ── setCurrent 信号与幂等 ───────────────────────────────────────────
def test_seg_set_current_emits_signal(qtbot):
    seg = _make_seg(qtbot)
    with qtbot.waitSignal(seg.currentChanged, timeout=2000) as blocker:
        seg.setCurrent(2)
    assert blocker.args == [2]
    assert seg.current() == 2


def test_seg_set_current_same_index_no_emit(qtbot):
    seg = _make_seg(qtbot)
    seg.setCurrent(2)
    emitted: list[int] = []
    seg.currentChanged.connect(lambda i: emitted.append(i))
    # 重复 setCurrent(2) 不应发射。
    seg.setCurrent(2)
    assert emitted == []


# ── setCurrent 钳制 ─────────────────────────────────────────────────
def test_seg_set_current_clamps(qtbot):
    seg = _make_seg(qtbot)
    seg.setCurrent(-1)
    assert seg.current() == 0
    seg.setCurrent(99)
    assert seg.current() == len(seg.options()) - 1


# ── setOptions 重置 current ─────────────────────────────────────────
def test_seg_set_options_resets_current(qtbot):
    seg = _make_seg(qtbot)
    seg.setCurrent(2)
    assert seg.current() == 2
    # 新选项更少，current(2) 越界 → 钳到 0。
    seg.setOptions(["X", "Y"])
    assert seg.current() == 0
    assert seg.options() == ["X", "Y"]


def test_seg_set_options_empty(qtbot):
    seg = _make_seg(qtbot)
    seg.setOptions([])
    assert seg.options() == []
    assert seg.current() == -1


# ── sizeHint ────────────────────────────────────────────────────────
def test_seg_size_hint_grows_with_options(qtbot):
    seg_small = _make_seg(qtbot, ("A", "B"))
    seg_big = _make_seg(qtbot, ("A", "B", "C", "D"))
    w_small = seg_small.sizeHint().width()
    w_big = seg_big.sizeHint().width()
    assert w_big > w_small
    # 高度固定 32px。
    assert seg_small.sizeHint().height() == 32
    assert seg_big.sizeHint().height() == 32


# ── 鼠标点击 ────────────────────────────────────────────────────────
def test_seg_click_changes_current(qtbot):
    seg = _make_seg(qtbot)
    assert seg.current() == 0
    # 3 段、宽 200 → 每段 ~66.67px。点击段 1 的中心 ~100px。
    _click_at(seg, 100.0)
    assert seg.current() == 1


def test_seg_click_out_of_range_ignored(qtbot):
    seg = _make_seg(qtbot)
    seg.setCurrent(0)
    # 右键应被忽略，current 不变。
    pos = QPointF(100.0, seg.height() / 2.0)
    event = QMouseEvent(
        QMouseEvent.Type.MouseButtonPress,
        pos,
        pos,
        Qt.MouseButton.RightButton,
        Qt.MouseButton.RightButton,
        Qt.KeyboardModifier.NoModifier,
    )
    seg.mousePressEvent(event)
    assert seg.current() == 0


# ── paintEvent 不抛异常 ─────────────────────────────────────────────
def test_seg_paint_does_not_raise(qtbot):
    seg = _make_seg(qtbot)
    seg.setCurrent(1)

    class _MockEvent:
        pass

    # paintEvent 在无真实 QPaintEvent 时也应能跑完（参数被忽略）。
    seg.paintEvent(_MockEvent())  # type: ignore[arg-type]


# ── 指示器动画 ──────────────────────────────────────────────────────
def test_seg_indicator_animates_on_change(qtbot):
    seg = _make_seg(qtbot)
    seg.show()
    qtbot.waitExposed(seg)
    QApplication.processEvents()

    seg.setCurrent(1)
    # 切换后应存在活跃动画。
    assert len(seg._active_anims) >= 1, "setCurrent should start an indicator animation"
    anim = seg._active_anims[-1]

    # 手动推进到 duration 终点，指示器应到达段 1 的目标矩形。
    anim.setCurrentTime(anim.duration())
    target = seg._segment_rect(1)
    got = seg._indicator_rect
    assert abs(got.x() - target.x()) < 0.5, (got.x(), target.x())
    assert abs(got.width() - target.width()) < 0.5
    assert abs(got.y() - target.y()) < 0.5
    assert abs(got.height() - target.height()) < 0.5
