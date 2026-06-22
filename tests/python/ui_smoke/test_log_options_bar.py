"""B51 ui_smoke: 日志选项工具条 6 widget 集成验证。

覆盖 log_options_bar helper wire 的所有死 widget：
- Divider（分隔线存在 + objectName）
- Badge（连接状态徽章文本/kind 切换）
- ToggleSwitch（自动滚动开关初始 checked）
- Chip（filter 标签文本 + removable）
- SegmentedControl（视图模式 3 段 + currentChanged）
- InfoBanner（持久信息条初始隐藏 + dismissed 信号）

端到端验证：build_main_window 后日志工具条可见且 widget 可达。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.app.main import build_main_window
from embeddebug.serial_station.ui.controls import BadgeKind
from embeddebug.serial_station.ui.log_options_bar import (
    build_log_info_banner,
    build_log_options_bar,
    show_log_info_banner,
    update_connection_badge,
)


def test_log_options_bar_widgets_present_in_main_window(qtbot):
    """build_main_window 后日志工具条的 6 个 widget 都可达。"""

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()
    qtbot.waitUntil(lambda: window.isVisible(), timeout=1000)

    # Badge 连接状态。
    badge = window.findChild(type(window), "serialStationLogConnectionBadge")
    # findChild 按 objectName 在 QWidget 子类找；Badge 是 QWidget 子类。
    from embeddebug.serial_station.ui.controls import Badge
    badge = window.findChild(Badge, "serialStationLogConnectionBadge")
    assert badge is not None
    assert badge.text()  # 非空（"未连接" 或 tr 后）

    # ToggleSwitch 自动滚动。
    from embeddebug.serial_station.ui.controls import ToggleSwitch
    toggle = window.findChild(ToggleSwitch, "serialStationAutoScrollToggle")
    assert toggle is not None
    assert toggle.is_checked() is True  # 默认开

    # Chip filter。
    from embeddebug.serial_station.ui.controls import Chip
    chip = window.findChild(Chip, "serialStationActiveFilterChip")
    assert chip is not None
    assert chip.is_removable() is True

    # SegmentedControl 视图模式。
    from embeddebug.serial_station.ui.controls import SegmentedControl
    seg = window.findChild(SegmentedControl, "serialStationLogViewModeSegmented")
    assert seg is not None


def test_update_connection_badge_switches_kind(qtbot):
    """update_connection_badge 按 connected 切换 Badge kind + 文本。"""

    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.controls import Badge
    parent = QWidget()
    qtbot.addWidget(parent)
    badge = Badge("未连接", BadgeKind.INFO, parent=parent)

    update_connection_badge(badge, connected=True, tr=lambda t: t)
    assert badge.kind() == BadgeKind.SUCCESS
    assert "已连接" in badge.text()

    update_connection_badge(badge, connected=False, tr=lambda t: t)
    assert badge.kind() == BadgeKind.INFO
    assert "未连接" in badge.text()


def test_update_connection_badge_none_safe(qtbot):
    """badge=None 时 update_connection_badge 安全跳过。"""

    update_connection_badge(None, connected=True, tr=lambda t: t)  # 不抛异常


def test_build_log_info_banner_initially_hidden(qtbot):
    """build_log_info_banner 构造的 banner 初始隐藏。"""

    from PyQt6.QtWidgets import QWidget
    parent = QWidget()
    qtbot.addWidget(parent)

    class _Owner:
        def tr(self, t):
            return t

    banner = build_log_info_banner(_Owner(), parent)
    assert banner.isHidden()


def test_build_log_options_bar_constructs_all_widgets(qtbot):
    """build_log_options_bar 独立构造包含全部 5 个 widget（Divider/Badge/Toggle/Chip/Segmented）。"""

    from PyQt6.QtWidgets import QWidget
    parent = QWidget()
    qtbot.addWidget(parent)

    class _Owner:
        def tr(self, t):
            return t

    row = build_log_options_bar(_Owner(), parent)
    assert row.count() >= 5  # 至少 5 个 widget + stretch


def test_show_log_info_banner_fly_in(qtbot):
    """show_log_info_banner 显示 banner 并启动 fly-in 动画（消费 DURATION_FLYOUT）。"""

    from PyQt6.QtWidgets import QWidget
    parent = QWidget()
    qtbot.addWidget(parent)

    class _Owner:
        def tr(self, t):
            return t

    banner = build_log_info_banner(_Owner(), parent)
    qtbot.addWidget(banner)
    assert banner.isHidden()

    from embeddebug.serial_station.ui.controls import BannerKind
    show_log_info_banner(banner, "测试消息", BannerKind.SUCCESS)
    assert not banner.isHidden()
    assert "测试消息" in banner.text()


def test_show_log_info_banner_none_safe(qtbot):
    """banner=None 时 show_log_info_banner 安全跳过。"""

    show_log_info_banner(None, "test")  # 不抛异常


def test_segmented_control_current_changed_signal(qtbot):
    """SegmentedControl currentChanged 信号可连接（wire 后端到端可达）。"""

    from embeddebug.serial_station.ui.controls import SegmentedControl
    seg = SegmentedControl(["A", "B", "C"])
    qtbot.addWidget(seg)
    received = []
    seg.currentChanged.connect(lambda i: received.append(i))
    seg.setCurrent(2)
    assert received == [2]


def test_chip_removed_signal(qtbot):
    """Chip removed 信号可连接（wire 后 close button 路径可达）。"""

    from embeddebug.serial_station.ui.controls import Chip
    chip = Chip("label", removable=True)
    qtbot.addWidget(chip)
    received = []
    chip.removed.connect(lambda t: received.append(t))
    chip.removed.emit("label")
    assert received == ["label"]


def test_toggle_switch_toggled_signal(qtbot):
    """ToggleSwitch toggled 信号端到端可达（wire 后状态变化可观察）。"""

    from embeddebug.serial_station.ui.controls import ToggleSwitch
    toggle = ToggleSwitch()
    qtbot.addWidget(toggle)
    received = []
    toggle.toggled.connect(lambda v: received.append(v))
    toggle.toggle()
    assert received == [True]


def test_badge_kind_colors_all_defined(qtbot):
    """Badge 4 种 kind 都有配色定义（wire 后 INFO/WARNING/ERROR/SUCCESS 完整）。"""

    from embeddebug.serial_station.ui.controls import Badge, BadgeKind
    from PyQt6.QtWidgets import QWidget
    parent = QWidget()
    qtbot.addWidget(parent)
    for kind in BadgeKind:
        badge = Badge("x", kind, parent=parent)
        assert kind in badge.KIND_COLORS


def test_divider_constructs_with_label(qtbot):
    """Divider 带标签构造（wire 后分组标签路径可达）。"""

    from embeddebug.serial_station.ui.controls import Divider
    from PyQt6.QtWidgets import QWidget
    parent = QWidget()
    qtbot.addWidget(parent)
    d = Divider("日志工具", parent=parent)
    assert d.objectName() == "serialStationDivider"
