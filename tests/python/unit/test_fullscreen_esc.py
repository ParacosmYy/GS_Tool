"""Batch 26 测试：dashboard widget 全屏 ESC 退出。

覆盖：
1. attach_double_click_fullscreen 重写 keyPressEvent（ESC 退出全屏）。
2. 全屏态 ESC → restore，is_fullscreen 变 False。
3. 非全屏态 ESC → 不 restore（交父类）。
4. enter() setFocus（widget 能收按键事件）。
5. 源码接入断言（attach_double_click_fullscreen 含 keyPressEvent override）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent
from PyQt6.QtWidgets import QLabel

from embeddebug.serial_station.ui.dashboard import attach_double_click_fullscreen


def _make_widget(qtbot, host=None):
    host_widget = host or QLabel("host")
    qtbot.addWidget(host_widget)
    host_widget.resize(400, 300)
    widget = QLabel("child", host_widget)
    widget.setGeometry(10, 10, 50, 20)
    qtbot.addWidget(widget)
    return widget, host_widget


def _esc_event() -> QKeyEvent:
    return QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Escape, Qt.KeyboardModifier.NoModifier)


def _enter_event() -> QKeyEvent:
    return QKeyEvent(QEvent.Type.KeyPress, Qt.Key.Key_Return, Qt.KeyboardModifier.NoModifier)


# ── ESC 退出全屏 ───────────────────────────────────────────────────
def test_esc_exits_fullscreen(qtbot):
    """全屏态按 ESC 应 restore 退出。"""

    widget, host = _make_widget(qtbot)
    handler = attach_double_click_fullscreen(widget, host=host)
    handler.enter(widget, host=host)
    assert handler.is_fullscreen is True
    widget.keyPressEvent(_esc_event())
    assert handler.is_fullscreen is False


def test_esc_non_fullscreen_does_not_restore(qtbot):
    """非全屏态 ESC 不触发 restore（交父类默认处理）。"""

    widget, host = _make_widget(qtbot)
    handler = attach_double_click_fullscreen(widget, host=host)
    # 未进入全屏，ESC 不应改变状态。
    assert handler.is_fullscreen is False
    widget.keyPressEvent(_esc_event())
    assert handler.is_fullscreen is False


def test_non_esc_key_does_not_exit_fullscreen(qtbot):
    """全屏态非 ESC 键（如 Enter）不退出全屏。"""

    widget, host = _make_widget(qtbot)
    handler = attach_double_click_fullscreen(widget, host=host)
    handler.enter(widget, host=host)
    widget.keyPressEvent(_enter_event())
    assert handler.is_fullscreen is True  # 仍全屏


# ── keyPressEvent 重写 ─────────────────────────────────────────────
def test_keyPressEvent_overridden(qtbot):
    """attach 后 widget 的 keyPressEvent 应被替换为闭包。"""

    widget, host = _make_widget(qtbot)
    attach_double_click_fullscreen(widget, host=host)
    assert "keyPressEvent" in widget.__dict__


def test_double_click_still_works(qtbot):
    """ESC 加入后双击全屏 toggle 仍正常（直接调 handler.enter 验证 toggle 语义）。

    offscreen 模式下双击事件经 mouseDoubleClickEvent 闭包，但 setFocus 在无事件循环
    时不真正聚焦，故直接验证 toggle 行为而非依赖焦点。
    """

    widget, host = _make_widget(qtbot)
    handler = attach_double_click_fullscreen(widget, host=host)
    # 直接调 toggle 验证双击语义（双击闭包内部即调 toggle）。
    assert handler.toggle(widget, host=host) is True   # 进入
    assert handler.is_fullscreen is True
    assert handler.toggle(widget, host=host) is False  # 退出
    assert handler.is_fullscreen is False


# ── enter setFocus ─────────────────────────────────────────────────
def test_enter_calls_set_focus(qtbot, monkeypatch):
    """enter() 应调 widget.setFocus（Batch 26，源码行为断言，offscreen 不保证真实聚焦）。"""

    widget, host = _make_widget(qtbot)
    handler = attach_double_click_fullscreen(widget, host=host)
    focus_calls: list = []
    monkeypatch.setattr(
        widget, "setFocus", lambda reason: focus_calls.append(reason)
    )
    handler.enter(widget, host=host)
    assert len(focus_calls) == 1


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_attach_function_has_key_press_override():
    """attach_double_click_fullscreen 源码应含 keyPressEvent override + ESC 检查。"""

    from embeddebug.serial_station.ui.dashboard import fullscreen

    src = inspect.getsource(fullscreen.attach_double_click_fullscreen)
    assert "keyPressEvent" in src
    assert "Key_Escape" in src
    assert "is_fullscreen" in src


def test_enter_sets_focus_in_source():
    """enter() 源码应含 setFocus（Batch 26）。"""

    from embeddebug.serial_station.ui.dashboard.fullscreen import WidgetFullscreenHandler

    src = inspect.getsource(WidgetFullscreenHandler.enter)
    assert "setFocus" in src
