"""RichTooltip 单元测试。

覆盖：
- objectName 合规（QSS 依赖）。
- title()/body() 访问器。
- sizeHint 非零（默认 ~240×80）。
- paintEvent 不抛（mock QPaintEvent，offscreen 下安全绘制）。
- install_tooltip 创建实例 + 写入 ``_installed`` 注册表。
- install_tooltip 重复挂接复用同一实例。
- uninstall_tooltip 从 ``_installed`` 移除。
- show_for 调度 QTimer 不抛。
- hide_immediately 在从未 show 时安全调用。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

from PyQt6.QtCore import QPoint, QSize
from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.controls import rich_tooltip
from embeddebug.serial_station.ui.controls.rich_tooltip import (
    RichTooltip,
    install_tooltip,
    uninstall_tooltip,
)


# ── 工具 ────────────────────────────────────────────────────────────
def _make_button(qtbot, w: int = 100, h: int = 40) -> QPushButton:
    """构造有固定几何的按钮，并映射到屏幕坐标系供 show_for 定位。"""

    btn = QPushButton("test")
    btn.setGeometry(0, 0, w, h)
    qtbot.addWidget(btn)
    return btn


def _cleanup_registry() -> None:
    """清空模块级 _installed 注册表，避免测试间残留。"""

    for tooltip in list(rich_tooltip._installed.values()):
        try:
            tooltip.hide()
            tooltip.deleteLater()
        except Exception:
            pass
    rich_tooltip._installed.clear()


# ── 基本属性 ────────────────────────────────────────────────────────
def test_rich_tooltip_objectname():
    tip = RichTooltip("标题", "正文")
    assert tip.objectName() == "serialStationRichTooltip"


def test_rich_tooltip_stores_title_body():
    tip = RichTooltip("我的标题", "我的正文")
    assert tip.title() == "我的标题"
    assert tip.body() == "我的正文"


def test_rich_tooltip_size_hint_nonzero():
    tip = RichTooltip("标题", "正文")
    hint = tip.sizeHint()
    assert isinstance(hint, QSize)
    assert hint.width() > 0
    assert hint.height() > 0


def test_rich_tooltip_icon_accessor_returns_none_by_default():
    tip = RichTooltip("标题", "正文")
    assert tip.icon() is None


# ── paintEvent 安全性 ───────────────────────────────────────────────
def test_rich_tooltip_paint_no_raise(qtbot):
    """paintEvent 应在 offscreen 下完整跑过不抛（含图标分支）。"""

    tip = RichTooltip("标题", "正文" * 20)
    qtbot.addWidget(tip)
    tip.resize(240, 80)
    mock_event = MagicMock()
    # 直接调用，模拟 Qt 触发 paint。
    tip.paintEvent(mock_event)


def test_rich_tooltip_paint_with_icon_no_raise(qtbot):
    """带图标分支也应安全绘制。"""

    from PyQt6.QtGui import QPixmap

    pixmap = QPixmap(16, 16)
    pixmap.fill()
    tip = RichTooltip("标题", "正文", icon=pixmap)
    qtbot.addWidget(tip)
    tip.resize(240, 80)
    mock_event = MagicMock()
    tip.paintEvent(mock_event)


# ── install / uninstall ─────────────────────────────────────────────
def test_install_tooltip_creates_instance(qtbot):
    _cleanup_registry()
    btn = _make_button(qtbot)

    tip = install_tooltip(btn, "标题", "正文")
    assert isinstance(tip, RichTooltip)
    assert id(btn) in rich_tooltip._installed
    assert rich_tooltip._installed[id(btn)] is tip
    _cleanup_registry()


def test_install_tooltip_reuses_existing(qtbot):
    _cleanup_registry()
    btn = _make_button(qtbot)

    tip1 = install_tooltip(btn, "T1", "B1")
    tip2 = install_tooltip(btn, "T2", "B2")
    assert tip1 is tip2, "reinstall on same widget should reuse instance"
    # 内容应被刷新。
    assert tip1.title() == "T2"
    assert tip1.body() == "B2"
    assert len(rich_tooltip._installed) == 1
    _cleanup_registry()


def test_install_tooltip_hooks_enter_leave(qtbot):
    """enter/leave 应被 patch（间接验证：调用不抛 + 行为触发）。"""

    _cleanup_registry()
    btn = _make_button(qtbot)
    install_tooltip(btn, "T", "B")

    # enterEvent 应是 patch 后的函数（不是 QPushButton 原生方法）。
    assert btn.enterEvent.__name__ == "_patched_enter"
    assert btn.leaveEvent.__name__ == "_patched_leave"

    # 模拟事件触发：不应抛异常。
    enter_event = MagicMock()
    leave_event = MagicMock()
    btn.enterEvent(enter_event)
    btn.leaveEvent(leave_event)
    _cleanup_registry()


def test_uninstall_tooltip_removes_entry(qtbot):
    _cleanup_registry()
    btn = _make_button(qtbot)
    install_tooltip(btn, "T", "B")
    assert id(btn) in rich_tooltip._installed

    uninstall_tooltip(btn)
    assert id(btn) not in rich_tooltip._installed
    _cleanup_registry()


def test_uninstall_tooltip_when_not_installed_is_safe(qtbot):
    """对未挂接的 widget 调用 uninstall 应静默返回。"""

    _cleanup_registry()
    btn = _make_button(qtbot)
    # 不应抛。
    uninstall_tooltip(btn)
    assert id(btn) not in rich_tooltip._installed
    _cleanup_registry()


# ── show_for / hide_immediately ─────────────────────────────────────
def test_show_for_does_not_raise(qtbot):
    tip = RichTooltip("标题", "正文")
    qtbot.addWidget(tip)
    btn = _make_button(qtbot)
    # 调用应调度 QTimer.singleShot，不立即 show、不抛。
    tip.show_for(btn, delay_ms=10)
    assert tip._target_widget is btn
    # 立即清理挂起定时器，避免跨测试泄漏。
    tip.hide_immediately()


def test_show_for_zero_delay_does_not_raise(qtbot):
    tip = RichTooltip("标题", "正文")
    qtbot.addWidget(tip)
    btn = _make_button(qtbot)
    tip.show_for(btn, delay_ms=0)
    tip.hide_immediately()


def test_hide_immediately_does_not_raise_when_never_shown(qtbot):
    """从未 show 过的 tooltip 调 hide_immediately 应安全（无动画、无窗口）。"""

    tip = RichTooltip("标题", "正文")
    qtbot.addWidget(tip)
    # 不应抛。
    tip.hide_immediately()
    assert not tip.isVisible()
    assert tip.windowOpacity() == 0.0


def test_hide_immediately_cancels_pending_timer(qtbot):
    """已调度但未触发的 show 应被 hide 取消。"""

    tip = RichTooltip("标题", "正文")
    qtbot.addWidget(tip)
    btn = _make_button(qtbot)
    tip.show_for(btn, delay_ms=1000)
    assert tip._pending_timer is not None
    tip.hide_immediately()
    # pending_timer 在 hide 后应被清空。
    assert tip._pending_timer is None
