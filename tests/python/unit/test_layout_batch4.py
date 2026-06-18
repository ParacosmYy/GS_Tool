"""Batch 4 (B2) 布局修复测试：响应式 AppShell 模式 + 折叠卡高度动画。

覆盖三个修复：
1. ResponsiveLayout.attach_to_top_level —— 监听顶层窗口 resize（AppShell 模式生效）。
2. ResponsiveLayout 断点折叠/展开 + 信号。
3. CollapsibleCard 折叠/展开走 CollapseAnimation（不再硬切 setVisible）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QMainWindow, QSplitter, QWidget

from embeddebug.serial_station.ui.collapsible_card import CollapsibleCard
from embeddebug.serial_station.ui.responsive_layout import (
    BREAKPOINT_COLLAPSE,
    BREAKPOINT_EXPAND,
    ResponsiveLayout,
)


# ── ResponsiveLayout ──────────────────────────────────────────────
def _make_splitter_with_three_zones(parent: QWidget) -> QSplitter:
    """构建一个三区 QSplitter（左/中/右各一个 QWidget）。"""

    splitter = QSplitter(Qt.Orientation.Horizontal, parent)
    for name in ("serialStationLeftZone", "serialStationCenterZone", "serialStationRightZone"):
        w = QWidget(splitter)
        w.setObjectName(name)
        w.setMinimumWidth(50)
    splitter.setSizes([300, 620, 260])
    return splitter


def test_responsive_collapse_below_breakpoint(qtbot):
    window = QMainWindow()
    qtbot.addWidget(window)
    splitter = _make_splitter_with_three_zones(window)
    responsive = ResponsiveLayout(splitter, window)
    collapsed_signals: list = []
    responsive.sidebar_collapsed.connect(lambda: collapsed_signals.append(True))

    responsive.on_window_resized(BREAKPOINT_COLLAPSE - 10)
    assert responsive.is_collapsed is True
    assert collapsed_signals == [True]
    # 左区应被设为 0 宽度。
    assert splitter.sizes()[0] == 0


def test_responsive_expand_above_breakpoint(qtbot):
    window = QMainWindow()
    qtbot.addWidget(window)
    splitter = _make_splitter_with_three_zones(window)
    responsive = ResponsiveLayout(splitter, window)

    # 先折叠。
    responsive.on_window_resized(BREAKPOINT_COLLAPSE - 10)
    assert responsive.is_collapsed is True
    # 再展开（需超过 EXPAND 断点，滞后防抖）。
    expanded_signals: list = []
    responsive.sidebar_expanded.connect(lambda: expanded_signals.append(True))
    responsive.on_window_resized(BREAKPOINT_EXPAND + 10)
    assert responsive.is_collapsed is False
    assert expanded_signals == [True]


def test_responsive_hysteresis_no_flicker(qtbot):
    """折叠与展开断点之间（滞后区）不应反复触发。"""

    window = QMainWindow()
    qtbot.addWidget(window)
    splitter = _make_splitter_with_three_zones(window)
    responsive = ResponsiveLayout(splitter, window)

    collapse_count: list[int] = []
    expand_count: list[int] = []
    responsive.sidebar_collapsed.connect(lambda: collapse_count.append(1))
    responsive.sidebar_expanded.connect(lambda: expand_count.append(1))
    responsive.on_window_resized(BREAKPOINT_COLLAPSE - 10)
    assert responsive.is_collapsed is True
    assert len(collapse_count) == 1
    # 在滞后区（COLLAPSE 到 EXPAND 之间）resize，应保持折叠不抖动。
    responsive.on_window_resized((BREAKPOINT_COLLAPSE + BREAKPOINT_EXPAND) // 2)
    assert responsive.is_collapsed is True
    assert len(expand_count) == 0


def test_responsive_attach_to_top_level_does_not_crash(qtbot):
    """attach_to_top_level 应成功安装事件过滤器且不崩溃。

    Batch 4 核心修复：旧版只靠 SerialStationMainWindow.resizeEvent 驱动，
    AppShell 多模式 shell 下内容被 reparent 后失效。新版监听顶层窗口。
    事件过滤器的实际 resize 投递依赖窗口 show + 事件循环，这里只验证安装健壮性，
    断点逻辑由 test_responsive_collapse/expand 直接覆盖。
    """

    window = QMainWindow()
    qtbot.addWidget(window)
    splitter = _make_splitter_with_three_zones(window)
    window.setCentralWidget(splitter)
    responsive = ResponsiveLayout(splitter, window)
    # 应可对任意 QWidget 安装事件过滤器不崩溃。
    responsive.attach_to_top_level(window)
    # 手动转发 resize（模拟事件过滤器捕获后的调用路径）仍应正确驱动断点。
    responsive.on_window_resized(BREAKPOINT_COLLAPSE - 50)
    assert responsive.is_collapsed is True


# ── CollapsibleCard ───────────────────────────────────────────────
def test_collapsible_card_default_state(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    card = CollapsibleCard(parent, "Settings", expanded=False)
    assert card.is_expanded() is False


def test_collapsible_card_toggle(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    card = CollapsibleCard(parent, "Settings", expanded=False)
    assert card.is_expanded() is False
    card.toggle()
    assert card.is_expanded() is True
    card.toggle()
    assert card.is_expanded() is False


def test_collapsible_card_expand_starts_animation(qtbot):
    """展开应启动 CollapseAnimation（不再硬切 setVisible）。

    Batch 4 改进：旧版用 setVisible 硬切，违反铁律 18。
    """

    parent = QWidget()
    qtbot.addWidget(parent)
    card = CollapsibleCard(parent, "Settings", expanded=False)
    # 展开后应持有 _collapse_anim（CollapseAnimation.expand 返回的动画）。
    card.set_expanded(True)
    assert card._collapse_anim is not None


def test_collapsible_card_collapse_sets_animation(qtbot):
    """折叠也应走动画。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card = CollapsibleCard(parent, "Settings", expanded=True)
    card.set_expanded(False)
    assert card._collapse_anim is not None


def test_collapsible_card_body_layout_accessible(qtbot):
    """body_layout() 应可被调用方 addWidget 装子控件。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    card = CollapsibleCard(parent, "Settings", expanded=True)
    body = card.body_layout()
    assert body is not None
