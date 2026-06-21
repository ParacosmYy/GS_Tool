"""Drawer 抽屉控件单元测试。

覆盖：
- objectName 合规（QSS 依赖）。
- 默认 side 为 LeftEdge。
- set_content 替换内部控件（旧控件 reparent + 新控件接入）。
- open() 发射 opened 信号且 is_open() 为 True。
- close() 发射 closed 信号且 is_open() 为 False。
- paintEvent 不抛异常（repaint 模式，覆盖四种 side + overlay 淡入路径）。
- sizeHint 非零。
- open → close 完整循环不触发访问冲突（绑定方法模式验证）。
- 点击遮罩区域（面板外）触发 close。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, QPoint, QPointF, Qt
from PyQt6.QtGui import QMouseEvent
from PyQt6.QtWidgets import QLabel

from embeddebug.serial_station.ui.controls.drawer import Drawer


def _make_drawer(qtbot, side: Qt.Edge = Qt.Edge.LeftEdge) -> Drawer:
    """构造一个 resize 到 400×300 的 Drawer，注册到 qtbot。"""

    drawer = Drawer(side=side)
    drawer.resize(400, 300)
    qtbot.addWidget(drawer)
    return drawer


def _left_press_at(widget, x: float, y: float) -> QMouseEvent:
    """构造左键 press 事件（6 参重载，对齐 test_chip 的 6 参形式）。

    PyQt6 6.11 已废弃 7 参重载；本函数使用 (type, localPos, globalPos, button,
    buttons, modifiers) 6 参形式。
    """

    return QMouseEvent(
        QEvent.Type.MouseButtonPress,
        QPointF(x, y),
        QPointF(widget.mapToGlobal(QPoint(int(x), int(y)))),
        Qt.MouseButton.LeftButton,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
    )


# ── 基础属性 ─────────────────────────────────────────────────────────
def test_drawer_objectname(qtbot):
    """objectName 必须为 serialStationDrawer（QSS 选择器依赖）。"""

    drawer = _make_drawer(qtbot)
    assert drawer.objectName() == "serialStationDrawer"


def test_drawer_default_side_left(qtbot):
    """未指定 side 时默认为 LeftEdge。"""

    drawer = Drawer()
    qtbot.addWidget(drawer)
    assert drawer._side == Qt.Edge.LeftEdge


def test_drawer_size_hint_nonzero(qtbot):
    """sizeHint 宽高均非零。"""

    drawer = _make_drawer(qtbot)
    hint = drawer.sizeHint()
    assert hint.width() > 0 and hint.height() > 0


def test_drawer_is_open_default_false(qtbot):
    """初始状态 is_open 为 False。"""

    drawer = _make_drawer(qtbot)
    assert not drawer.is_open()


# ── set_content ─────────────────────────────────────────────────────
def test_drawer_set_content_replaces(qtbot):
    """set_content 替换内部控件：新控件 parent 为 _panel，旧控件被替换。"""

    drawer = _make_drawer(qtbot)
    label1 = QLabel("first")
    drawer.set_content(label1)
    assert drawer._content is label1
    assert label1.parent() is drawer._panel

    label2 = QLabel("second")
    drawer.set_content(label2)
    assert drawer._content is label2
    assert label2.parent() is drawer._panel


# ── paintEvent ──────────────────────────────────────────────────────
def test_drawer_paint_does_not_raise(qtbot):
    """paintEvent 在未 open / overlay 满 alpha / 四种 side 下均不抛异常。"""

    # 未 open 状态下 repaint。
    drawer = _make_drawer(qtbot)
    drawer.repaint()

    # 模拟 open 后 overlay 满 alpha。
    drawer._overlay_alpha = 1.0
    drawer._panel.setGeometry(drawer._panel_target_rect())
    drawer.repaint()

    # 覆盖四种 side 方向（accent 高亮线四条分支）。
    for side in (Qt.Edge.LeftEdge, Qt.Edge.RightEdge, Qt.Edge.TopEdge, Qt.Edge.BottomEdge):
        d = _make_drawer(qtbot, side=side)
        d._overlay_alpha = 1.0
        d._panel.setGeometry(d._panel_target_rect())
        d.repaint()


# ── open / close 信号 ──────────────────────────────────────────────
def test_drawer_open_emits_opened_and_sets_is_open(qtbot):
    """open() 发射 opened 信号且 is_open() 为 True。"""

    drawer = _make_drawer(qtbot)
    with qtbot.waitSignal(drawer.opened, timeout=2000):
        drawer.open()
    assert drawer.is_open()


def test_drawer_close_emits_closed_and_sets_not_open(qtbot):
    """close() 同步置 is_open False，动画完成后发射 closed 信号。

    is_open 在 close() 调用时同步翻转（不等动画），故在 waitSignal 体内心断言；
    closed 信号在滑出动画结束后发射，随后 deleteLater 被推迟到下一轮事件循环。
    """

    drawer = _make_drawer(qtbot)
    with qtbot.waitSignal(drawer.opened, timeout=2000):
        drawer.open()
    assert drawer.is_open()

    with qtbot.waitSignal(drawer.closed, timeout=2000):
        drawer.close()
        # close() 同步置 _is_open=False；widget 此时仍存活（动画进行中）。
        assert not drawer.is_open()
    # closed 已发射即证明 close 周期完成；此后 deleteLater 可能已执行，不再访问 drawer。


# ── open → close 完整循环 ───────────────────────────────────────────
def test_drawer_open_close_cycle_no_violation(qtbot):
    """open → close 完整循环不触发访问冲突（绑定方法 finished 连接验证）。

    若 _on_open_finished / _on_close_finished 为闭包而非绑定方法，在 pytest-qt
    跨用例 processEvents 时会触发 Windows 访问冲突。到达此处即证明绑定方法模式有效。
    """

    drawer = _make_drawer(qtbot)
    with qtbot.waitSignal(drawer.opened, timeout=2000):
        drawer.open()
    assert drawer.is_open()
    with qtbot.waitSignal(drawer.closed, timeout=2000):
        drawer.close()


# ── 遮罩点击关闭 ────────────────────────────────────────────────────
def test_drawer_click_overlay_closes(qtbot):
    """点击遮罩区域（面板外）触发 close → is_open False + closed 信号。

    LeftEdge 面板宽 280，点击 x=350（面板外）应触发关闭。
    """

    drawer = _make_drawer(qtbot, side=Qt.Edge.LeftEdge)
    with qtbot.waitSignal(drawer.opened, timeout=2000):
        drawer.open()
    assert drawer.is_open()

    # 点击面板外区域（遮罩）→ close。
    drawer.mousePressEvent(_left_press_at(drawer, 350.0, 150.0))
    assert not drawer.is_open()
    with qtbot.waitSignal(drawer.closed, timeout=2000):
        pass


def test_drawer_click_inside_panel_does_not_close(qtbot):
    """点击面板内部不应触发关闭（仅遮罩区域关闭）。"""

    drawer = _make_drawer(qtbot, side=Qt.Edge.LeftEdge)
    with qtbot.waitSignal(drawer.opened, timeout=2000):
        drawer.open()
    assert drawer.is_open()

    # 点击面板内（x=50 在 280 宽面板内）→ 不关闭。
    drawer.mousePressEvent(_left_press_at(drawer, 50.0, 150.0))
    assert drawer.is_open()

    # 清理：停止动画以防 deleteLater 在后续用例前触发。
    if drawer._slide is not None:
        drawer._slide.stop()
        drawer._slide = None


# ── side 方向覆盖 ───────────────────────────────────────────────────
def test_drawer_open_right_edge(qtbot):
    """RightEdge 方向 open → opened 信号正常发射。"""

    drawer = _make_drawer(qtbot, side=Qt.Edge.RightEdge)
    with qtbot.waitSignal(drawer.opened, timeout=2000):
        drawer.open()
    assert drawer.is_open()
    # 清理：停止动画避免 deleteLater。
    if drawer._slide is not None:
        drawer._slide.stop()
        drawer._slide = None
