"""Chip 控件单元测试。

覆盖：
- objectName 合规（QSS 依赖）。
- 文本 get/set round-trip。
- 默认未选中、selectable 模式 toggle 信号去重。
- clicked 信号发射。
- removable close button 点击 → removed 信号 + 文本参数。
- removable=False 不绘制 close button（内部状态/几何断言）。
- sizeHint 随文本变宽。
- paintEvent 不抛异常。
- enter/leave 切换 _hovered 并调用 update。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, QPoint, QPointF, Qt
from PyQt6.QtGui import QEnterEvent, QMouseEvent

from embeddebug.serial_station.ui.controls.chip import Chip


def _make_chip(qtbot, text: str = "hello", **kwargs) -> Chip:
    """构造一个已 resize 到 sizeHint 的 Chip，注册到 qtbot。"""

    chip = Chip(text=text, **kwargs)
    hint = chip.sizeHint()
    chip.resize(hint)
    qtbot.addWidget(chip)
    return chip


def _left_press_at(widget, x: float, y: float) -> QMouseEvent:
    """构造左键 press 事件（6 参重载，globalPos 必须为 QPointF，对齐 test_segmented）。"""

    return QMouseEvent(
        QEvent.Type.MouseButtonPress,
        QPointF(x, y),
        QPointF(widget.mapToGlobal(QPoint(int(x), int(y)))),
        Qt.MouseButton.LeftButton,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
    )


# ── 基础属性 ─────────────────────────────────────────────────────────
def test_chip_objectname(qtbot):
    chip = _make_chip(qtbot)
    assert chip.objectName() == "serialStationChip"


def test_chip_text_getset(qtbot):
    chip = _make_chip(qtbot, text="initial")
    assert chip.text() == "initial"
    chip.set_text("updated")
    assert chip.text() == "updated"


def test_chip_default_not_selected(qtbot):
    chip = _make_chip(qtbot)
    assert not chip.is_selected()


# ── selectable / toggled ───────────────────────────────────────────
def test_chip_selectable_toggle_emits_signal(qtbot):
    chip = _make_chip(qtbot, selectable=True)
    assert not chip.is_selected()

    with qtbot.waitSignal(chip.toggled, timeout=1000) as blocker:
        chip.set_selected(True)
    assert blocker.args == [True]
    assert chip.is_selected()

    # 再次 set_selected(True) 是 no-op，不应发射。
    received: list[bool] = []
    chip.toggled.connect(lambda v: received.append(v))
    chip.set_selected(True)
    assert received == [], "set_selected with same value should not emit toggled"
    assert chip.is_selected()


def test_chip_non_selectable_set_selected_noop(qtbot):
    """不可选时 set_selected 不应改变状态、不发射信号。"""

    chip = _make_chip(qtbot, selectable=False)
    received: list[bool] = []
    chip.toggled.connect(lambda v: received.append(v))
    chip.set_selected(True)
    assert not chip.is_selected()
    assert received == []


# ── clicked ────────────────────────────────────────────────────────
def test_chip_clicked_signal(qtbot):
    chip = _make_chip(qtbot)
    cx, cy = chip.width() / 2, chip.height() / 2
    with qtbot.waitSignal(chip.clicked, timeout=1000):
        chip.mousePressEvent(_left_press_at(chip, cx, cy))


def test_chip_right_button_does_not_emit_clicked(qtbot):
    """右键不应触发 clicked（mousePressEvent 仅在 LeftButton 分支内 emit）。"""

    chip = _make_chip(qtbot)
    received: list[bool] = []
    chip.clicked.connect(lambda: received.append(True))
    event = QMouseEvent(
        QEvent.Type.MouseButtonPress,
        QPointF(chip.width() / 2, chip.height() / 2),
        QPointF(chip.width() / 2, chip.height() / 2),
        Qt.MouseButton.RightButton,
        Qt.MouseButton.RightButton,
        Qt.KeyboardModifier.NoModifier,
    )
    chip.mousePressEvent(event)
    assert received == []


# ── removable / removed ───────────────────────────────────────────
def test_chip_removable_emits_removed_on_close_click(qtbot):
    chip = _make_chip(qtbot, text="tag1", removable=True)
    close_rect = chip._close_button_rect()
    center = close_rect.center()
    with qtbot.waitSignal(chip.removed, timeout=1000) as blocker:
        chip.mousePressEvent(_left_press_at(chip, center.x(), center.y()))
    assert blocker.args == ["tag1"]


def test_chip_non_removable_close_button_rect_empty(qtbot):
    """removable=False 时不绘制 close button，_close_button_rect 返回空矩形。"""

    chip = _make_chip(qtbot, removable=False)
    assert chip._close_button_rect().isNull()


def test_chip_non_removable_click_does_not_remove(qtbot):
    """removable=False 时点击右侧区域不应发射 removed（走 clicked 路径）。"""

    chip = _make_chip(qtbot, removable=False)
    received: list[str] = []
    chip.removed.connect(lambda t: received.append(t))
    # 点右侧 close_area 区域位置，但 removable=False，不应触发 removed。
    chip.mousePressEvent(_left_press_at(chip, chip.width() - 5, chip.height() / 2))
    assert received == []


# ── sizeHint ───────────────────────────────────────────────────────
def test_chip_size_hint_grows_with_text(qtbot):
    short = _make_chip(qtbot, text="ab")
    long = _make_chip(qtbot, text="a-much-longer-chip-label-text")
    assert long.sizeHint().width() > short.sizeHint().width()
    assert short.sizeHint().height() == long.sizeHint().height() == 28


def test_chip_size_hint_height_constant(qtbot):
    chip = _make_chip(qtbot)
    assert chip.sizeHint().height() == 28


# ── paintEvent ─────────────────────────────────────────────────────
def test_chip_paint_does_not_raise(qtbot):
    chip = _make_chip(qtbot, text="paint-test", selectable=True, selected=True)
    chip.set_selected(True)
    from PyQt6.QtGui import QPaintEvent

    event = QPaintEvent(chip.rect())
    # 直接调用 paintEvent 不应抛异常（覆盖 selected / hover 两条路径）。
    chip.paintEvent(event)
    # 也覆盖一下 removable=False 的绘制分支。
    chip.set_removable(False)
    chip.paintEvent(QPaintEvent(chip.rect()))


def test_chip_paint_hover_path(qtbot):
    """模拟 _hovered=True 走 paintEvent hover 分支，不抛异常。"""

    chip = _make_chip(qtbot, text="hover")
    from PyQt6.QtGui import QPaintEvent

    chip._hovered = True
    chip._close_hovered = True
    chip.paintEvent(QPaintEvent(chip.rect()))


# ── hover 状态 ─────────────────────────────────────────────────────
def test_chip_hover_state_updates(qtbot):
    chip = _make_chip(qtbot)
    assert not chip._hovered

    center = QPointF(chip.width() / 2, chip.height() / 2)
    enter = QEnterEvent(center, center, center)
    chip.enterEvent(enter)
    assert chip._hovered is True

    leave = QEvent(QEvent.Type.Leave)
    chip.leaveEvent(leave)
    assert chip._hovered is False
    assert chip._close_hovered is False


def test_chip_close_hover_tracked_on_move(qtbot):
    """mouseMoveEvent 跟踪 close button 悬停状态。"""

    chip = _make_chip(qtbot, removable=True)
    close_rect = chip._close_button_rect()
    center = close_rect.center()
    move_inside = QMouseEvent(
        QEvent.Type.MouseMove,
        QPointF(center.x(), center.y()),
        QPointF(center.x(), center.y()),
        Qt.MouseButton.NoButton,
        Qt.MouseButton.NoButton,
        Qt.KeyboardModifier.NoModifier,
    )
    chip.mouseMoveEvent(move_inside)
    assert chip._close_hovered is True

    move_outside = QMouseEvent(
        QEvent.Type.MouseMove,
        QPointF(2.0, 2.0),
        QPointF(2.0, 2.0),
        Qt.MouseButton.NoButton,
        Qt.MouseButton.NoButton,
        Qt.KeyboardModifier.NoModifier,
    )
    chip.mouseMoveEvent(move_outside)
    assert chip._close_hovered is False


def test_chip_non_removable_move_is_noop(qtbot):
    """removable=False 时 mouseMoveEvent 不应改变 _close_hovered。"""

    chip = _make_chip(qtbot, removable=False)
    move = QMouseEvent(
        QEvent.Type.MouseMove,
        QPointF(chip.width() - 5, chip.height() / 2),
        QPointF(chip.width() - 5, chip.height() / 2),
        Qt.MouseButton.NoButton,
        Qt.MouseButton.NoButton,
        Qt.KeyboardModifier.NoModifier,
    )
    chip.mouseMoveEvent(move)
    assert chip._close_hovered is False
