"""Batch 5 (C1) 测试：EmptyStateWidget / SkeletonWidget / PlaceholderPanel。

覆盖三个新建/重做组件：
1. EmptyStateWidget —— 图标+标题+描述+可选 CTA，objectName 合规。
2. SkeletonWidget —— shimmer 动画启动/停止，SkeletonBlock 多行组合。
3. PlaceholderPanel —— 重做后用 EmptyStateWidget + 入场动画。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLabel, QPushButton

from embeddebug.serial_station.ui.panels.placeholder_panel import PlaceholderPanel
from embeddebug.serial_station.ui.widgets import EmptyStateWidget, SkeletonWidget
from embeddebug.serial_station.ui.widgets.skeleton import SkeletonBlock


# ── EmptyStateWidget ──────────────────────────────────────────────
def test_empty_state_has_objectname(qtbot):
    w = EmptyStateWidget(title="空", description="无数据")
    qtbot.addWidget(w)
    assert w.objectName() == "serialStationEmptyState"


def test_empty_state_renders_title_and_description(qtbot):
    w = EmptyStateWidget(title="暂无日志", description="连接设备后显示")
    qtbot.addWidget(w)
    assert w._title_label.text() == "暂无日志"
    assert w._desc_label.text() == "连接设备后显示"


def test_empty_state_set_title_updates(qtbot):
    w = EmptyStateWidget(title="A", description="B")
    qtbot.addWidget(w)
    w.set_title("C")
    assert w._title_label.text() == "C"


def test_empty_state_with_cta_creates_button(qtbot):
    clicked: list = []
    w = EmptyStateWidget(title="T", description="D", cta_text="重试", on_cta=lambda: clicked.append(1))
    qtbot.addWidget(w)
    assert isinstance(w._cta_button, QPushButton)
    w._cta_button.click()
    assert clicked == [1]


def test_empty_state_without_cta_no_button(qtbot):
    w = EmptyStateWidget(title="T", description="D")
    qtbot.addWidget(w)
    assert w._cta_button is None


def test_empty_state_emoji_fallback_when_no_icon(qtbot):
    w = EmptyStateWidget(title="T", description="D", emoji="📭")
    qtbot.addWidget(w)
    assert w._icon_label.text() == "📭"


# ── SkeletonWidget ────────────────────────────────────────────────
def test_skeleton_has_objectname(qtbot):
    sk = SkeletonWidget(height=20)
    qtbot.addWidget(sk)
    assert sk.objectName() == "serialStationSkeleton"


def test_skeleton_starts_shimmer(qtbot):
    sk = SkeletonWidget(height=20)
    qtbot.addWidget(sk)
    assert sk._shimmer_anim is not None
    assert sk._shimmer_anim.loopCount() == -1  # 无限循环


def test_skeleton_stop_shimmer(qtbot):
    sk = SkeletonWidget(height=20)
    qtbot.addWidget(sk)
    sk.stop_shimmer()
    assert sk._shimmer_anim is None


def test_skeleton_block_creates_multiple_rows(qtbot):
    block = SkeletonBlock(rows=4, with_title=True)
    qtbot.addWidget(block)
    # 4 正文行 + 1 标题行 = 5 个骨架。
    assert len(block._skeletons) == 5


def test_skeleton_block_stop_all(qtbot):
    block = SkeletonBlock(rows=3)
    qtbot.addWidget(block)
    block.stop_all()
    for sk in block._skeletons:
        assert sk._shimmer_anim is None


# ── PlaceholderPanel（重做后） ────────────────────────────────────
def test_placeholder_panel_builds_empty_state(qtbot):
    panel = PlaceholderPanel("rtt", "RTT 调试", "SEGGER RTT", icon_name="activity")
    widget = panel.build(app_controller=None)
    qtbot.addWidget(widget)
    # 应包含 EmptyStateWidget 子控件。
    empty = widget.findChild(EmptyStateWidget)
    assert empty is not None
    assert empty._title_label.text() == "RTT 调试"


def test_placeholder_panel_on_enter_starts_animation(qtbot):
    panel = PlaceholderPanel("rtt", "RTT", "desc")
    widget = panel.build(app_controller=None)
    qtbot.addWidget(widget)
    panel.on_enter()
    # on_enter 应启动入场动画（card_enter 返回非空列表）。
    assert len(panel._enter_anims) > 0


def test_placeholder_panel_on_leave_stops_animation(qtbot):
    panel = PlaceholderPanel("rtt", "RTT", "desc")
    widget = panel.build(app_controller=None)
    qtbot.addWidget(widget)
    panel.on_enter()
    panel.on_leave()
    assert panel._enter_anims == []
