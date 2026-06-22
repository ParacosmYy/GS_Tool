"""Batch 49-1: 日志区空态占位 helper。

独立模块，保持 ``sections.py`` ≤ 260 行（architecture test 守护）。

提供：
- ``build_log_empty_state(parent)``：在日志卡 body 构造 EmptyStateWidget 覆盖层。
- ``hide_log_empty_state(host)`` / ``show_log_empty_state(host)``：
  从 host 上挂载的 ``_log_empty_state`` 安全切换显隐（getattr 防御，
  调用方无需保证属性一定存在）。

触发点：
- 首条日志 append → hide_log_empty_state（log_actions.append_log_entry）。
- clear_log / disconnect → show_log_empty_state（session_actions.clear_log）。

约束：只依赖 PyQt6 + widgets + animations，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.widgets import EmptyStateWidget


class _LogEmptyStateHost(Protocol):
    """host 上访问 ``_log_empty_state`` 所需的最小表面（getattr 防御）。"""

    _log_empty_state: EmptyStateWidget | None


def build_log_empty_state(parent: QWidget) -> EmptyStateWidget:
    """构建日志区空态占位（inbox 图标 + 标题 + 描述）。

    Args:
        parent: 日志卡 body 容器（QPlainTextEdit 的兄弟 widget 容器）。

    Returns:
        配置好的 EmptyStateWidget，调用方负责 addWidget / show_with_fade。
    """

    return EmptyStateWidget(
        icon_name="inbox",
        title=parent.tr("暂无日志"),
        description=parent.tr("连接设备后将在此显示收发数据"),
        parent=parent,
    )


def hide_log_empty_state(host: _LogEmptyStateHost) -> None:
    """淡出隐藏日志空态（首条日志写入时调用）。"""

    empty = getattr(host, "_log_empty_state", None)
    if empty is None:
        return
    empty.hide_with_fade()


def show_log_empty_state(host: _LogEmptyStateHost) -> None:
    """淡入显示日志空态（清空日志 / 断开连接时调用）。"""

    empty = getattr(host, "_log_empty_state", None)
    if empty is None:
        return
    empty.show_with_fade()
