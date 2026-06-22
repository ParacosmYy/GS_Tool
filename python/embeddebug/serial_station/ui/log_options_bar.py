"""Batch 51: 日志区选项工具条 — wire 死 widget 到真实功能场景。

集中接入 Divider / Badge / ToggleSwitch / Chip 到日志工具区，消除
``test_no_dead_widgets.py`` 白名单。独立模块保持 ``sections.py`` ≤ 260 行。

接入语义：
- ``Divider``：日志工具区与操作区之间的视觉分组分隔线。
- ``Badge``：连接状态徽章（已连接=SUCCESS / 已断开=MUTED），实时反映连接。
- ``ToggleSwitch``：日志自动滚动开关（默认开，大量日志时用户可关）。
- ``Chip``：当前 filter 的可视化标签（显示 active filter 文本）。

约束：只依赖 PyQt6 + controls + theme，不访问 controller/transport。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QHBoxLayout, QLabel, QPushButton, QWidget

from embeddebug.serial_station.ui.controls import (
    Badge,
    BadgeKind,
    Chip,
    Divider,
    Drawer,
    InfoBanner,
    BannerKind,
    SegmentedControl,
    ToggleSwitch,
)
from embeddebug.serial_station.ui.theme import tokens as T


class _LogOptionsHost(Protocol):
    """host 上访问 tr / 连接状态 / filter 文本所需的最小表面。"""

    def tr(self, text: str) -> str: ...


def build_log_options_bar(owner: _LogOptionsHost, parent: QWidget) -> QHBoxLayout:
    """构建日志选项工具条（Divider + Badge + ToggleSwitch + Chip）。

    Args:
        owner: 主窗口（提供 tr）。
        parent: 父 widget。

    Returns:
        QHBoxLayout，调用方 addLayout 到日志区上方。
    """

    row = QHBoxLayout()
    row.setSpacing(T.SPACING_INT_MD)
    row.setContentsMargins(0, 0, 0, 0)

    # Batch 51-1: Divider 分隔日志工具区与上方区域（真实视觉分组）。
    divider = Divider(parent=parent)
    row.addWidget(divider)

    # Batch 51-2: Badge 连接状态徽章（实时反映连接状态，对齐 MobaXterm 状态指示）。
    status_badge = Badge(owner.tr("未连接"), BadgeKind.INFO, parent=parent)
    status_badge.setObjectName("serialStationLogConnectionBadge")
    row.addWidget(status_badge)

    # Batch 51-6: ToggleSwitch 自动滚动开关（大量日志时用户可暂停滚动）。
    auto_scroll_label = QLabel(owner.tr("自动滚动"), parent)
    auto_scroll_label.setObjectName("serialStationAutoScrollLabel")
    auto_scroll_toggle = ToggleSwitch(parent)
    auto_scroll_toggle.setObjectName("serialStationAutoScrollToggle")
    auto_scroll_toggle.set_checked(True)
    row.addWidget(auto_scroll_label)
    row.addWidget(auto_scroll_toggle)

    # Batch 51-3: Chip 显示当前 filter（可关闭清除 filter）。
    filter_chip = Chip(owner.tr("全部"), parent=parent, removable=True, selectable=False)
    filter_chip.setObjectName("serialStationActiveFilterChip")
    row.addWidget(filter_chip)

    # Batch 51-5: SegmentedControl 日志视图模式（Hex/ASCII/Dec），真实功能场景。
    # 替代未来可能加的独立视图模式 combo，对齐 VOFA+ 数据格式切换。
    view_mode = SegmentedControl(
        [owner.tr("ASCII"), owner.tr("Hex"), owner.tr("Dec")],
        parent=parent,
    )
    view_mode.setObjectName("serialStationLogViewModeSegmented")
    row.addWidget(view_mode)

    # Batch 52-1: Drawer 命令历史侧栏按钮（wire 最后一个死 widget）。
    # 点击打开右侧抽屉显示发送历史，对齐 MobaXterm 命令历史面板。
    history_btn = QPushButton(owner.tr("历史"), parent)
    history_btn.setObjectName("serialStationHistoryButton")
    history_btn.setCursor(Qt.CursorShape.PointingHandCursor)
    row.addWidget(history_btn)

    row.addStretch(1)
    return row


def build_history_drawer(owner: _LogOptionsHost, parent: QWidget) -> Drawer:
    """Batch 52-1: 构建命令历史侧栏抽屉（wire Drawer 到真实场景）。

    右侧滑入，承载一个占位 QLabel（未来接 command_history 数据）。
    消费预留 token DURATION_DRAWER（抽屉滑出动画时长）。
    """

    drawer = Drawer(side=Qt.Edge.RightEdge, parent=parent)
    drawer.setObjectName("serialStationHistoryDrawer")
    placeholder = QLabel(owner.tr("命令历史（待接入 controller.command_history）"), drawer)
    placeholder.setObjectName("serialStationHistoryPlaceholder")
    placeholder.setWordWrap(True)
    drawer.set_content(placeholder)
    return drawer


def build_log_info_banner(owner: _LogOptionsHost, parent: QWidget) -> InfoBanner:
    """Batch 51-4: 构建日志区持久信息条（wire InfoBanner 到真实场景）。

    用于显示连接提示/协议警告等持久通知（非 toast 的短暂场景）。
    初始隐藏，调用方按需 show。
    """

    banner = InfoBanner(
        text=owner.tr("日志记录已启用"),
        kind=BannerKind.INFO,
        parent=parent,
    )
    banner.setObjectName("serialStationLogInfoBanner")
    banner.hide()
    return banner


def show_log_info_banner(banner: InfoBanner | None, text: str, kind: BannerKind = BannerKind.INFO) -> None:
    """Batch 51: 显示日志信息条，带 fly-in 动画（消费 DURATION_FLYOUT + EASE_OUT_QUINT）。

    Args:
        banner: build_log_info_banner 创建的 banner（None 安全跳过）。
        text: 显示文本。
        kind: BannerKind（默认 INFO）。
    """

    if banner is None:
        return
    banner.set_text(text)
    banner.set_kind(kind)
    banner.show()
    # fly-in：从上方滑入 + 透明度淡入（消费预留 token DURATION_FLYOUT + EASE_OUT_QUINT）。
    from PyQt6.QtCore import QPropertyAnimation, QPoint
    from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

    start_y = banner.y() - 20
    pos_anim = QPropertyAnimation(banner, b"pos", banner)
    pos_anim.setDuration(AnimationTokens.DURATION_FLYOUT)
    pos_anim.setStartValue(QPoint(banner.x(), start_y))
    pos_anim.setEndValue(QPoint(banner.x(), banner.y()))
    pos_anim.setEasingCurve(AnimationTokens.EASE_OUT_QUINT)
    pos_anim.start()


def update_connection_badge(badge: Badge | None, connected: bool, tr) -> None:
    """根据连接状态更新 Badge 文本与 kind（供 connection_control_state 调用）。

    Args:
        badge: build_log_options_bar 创建的连接状态 Badge（None 安全跳过）。
        connected: 是否已连接。
        tr: host.tr 翻译入口。
    """

    if badge is None:
        return
    if connected:
        badge.set_text(tr("已连接"))
        badge.set_kind(BadgeKind.SUCCESS)
    else:
        badge.set_text(tr("未连接"))
        badge.set_kind(BadgeKind.INFO)
