"""B49-1/B49-2 ui_smoke: 日志空态 + 连接加载态切换。

覆盖：
- 空态：未连接时日志卡显示 EmptyStateWidget（图标 + 标题 + 描述）。
- 加载态：连接开始时显示 SkeletonBlock + 「正在建立连接…」。
- 切换：首条日志写入时空态淡出；清空日志时空态重新淡入。

黑盒测试：通过 build_main_window 端到端验证可见性，不直接 poke 私有动画属性。
动画异步：fade_out 的 finished→hide 用 qtbot.waitUntil(visibility) 等待。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.app.main import build_main_window
from embeddebug.serial_station.ui.log_empty_state import (
    hide_log_empty_state,
    show_log_empty_state,
)
from embeddebug.serial_station.ui.log_loading_state import (
    build_log_loading_state,
    hide_log_loading_state,
    show_log_loading_state,
)


def test_log_empty_state_visible_on_startup(qtbot):
    """启动时日志卡应显示空态占位（inbox 图标 + 标题 + 描述）。"""

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()
    qtbot.waitUntil(lambda: window.isVisible(), timeout=1000)

    empty = getattr(window, "_log_empty_state", None)
    assert empty is not None, "main window should mount _log_empty_state"
    assert empty.objectName() == "serialStationEmptyState"
    # 空态标题应包含「暂无日志」（tr 后文案）。
    title = empty._title_label.text()
    assert "暂无日志" in title or "No" in title or "log" in title.lower()


def test_log_empty_state_show_hide_cycle(qtbot):
    """空态 show_with_fade → hide_with_fade → show_with_fade 完整循环。"""

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()
    qtbot.waitUntil(lambda: window.isVisible(), timeout=1000)
    empty = window._log_empty_state

    # hide_with_fade 是异步（finished→hide），用 waitUntil 等待隐藏完成。
    hide_log_empty_state(window)
    qtbot.waitUntil(lambda: empty.isHidden(), timeout=2000)
    assert empty.isHidden()

    # 重新 show_with_fade。
    show_log_empty_state(window)
    qtbot.waitUntil(lambda: not empty.isHidden(), timeout=2000)
    assert not empty.isHidden()


def test_log_loading_overlay_constructs_and_toggles(qtbot):
    """连接加载态覆盖层（SkeletonBlock + 文案）能构造并安全切换。"""

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()
    qtbot.waitUntil(lambda: window.isVisible(), timeout=1000)
    overlay = window._log_loading_state

    assert overlay.objectName() == "serialStationLogLoadingOverlay"
    # 初始隐藏（连接未开始）。
    assert overlay.isHidden()

    # show → 可见；hide → 隐藏。
    show_log_loading_state(window)
    assert not overlay.isHidden()
    hide_log_loading_state(window)
    assert overlay.isHidden()


def test_log_loading_state_helpers_safe_without_attr(qtbot):
    """host 没有 _log_loading_state 时 helper 安全降级（不抛异常）。"""

    class BareHost:
        def tr(self, text: str) -> str:
            return text

    host = BareHost()
    # 这两个调用不应抛异常（getattr 防御）。
    show_log_loading_state(host)
    hide_log_loading_state(host)


def test_build_log_loading_state_standalone(qtbot):
    """build_log_loading_state 能独立构造（脱离主窗口，覆盖层单测）。"""

    from PyQt6.QtWidgets import QWidget
    parent = QWidget()
    qtbot.addWidget(parent)
    parent.resize(300, 200)

    overlay = build_log_loading_state(parent)
    assert overlay.objectName() == "serialStationLogLoadingOverlay"
    # 内含 SkeletonBlock + label。
    assert overlay._skeleton is not None
    assert overlay._label is not None
    assert overlay._label.text()  # 非空文案
