"""占位模式面板 — 用于尚未完全实现的功能模式。

Batch 5 (C1) 重做：用 EmptyStateWidget 替代朴素的双 QLabel 占位。
诊断报告指出占位面板朴素到只有两个 QLabel（标题+描述），无图标无 CTA，
用户落到占位页得不到任何动作指引。现在渲染模式 icon + 醒目标题 + 描述 +
可选 CTA，并带入场淡入动画（不突兀）。

objectName 带模式前缀，便于 QSS 与后续替换为真实面板。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.panel_animations import card_enter
from embeddebug.serial_station.ui.widgets import EmptyStateWidget


class PlaceholderPanel:
    """占位 ModePanel：EmptyStateWidget（图标 + 标题 + 描述 + 可选 CTA）。

    Args:
        mode_id: 模式标识（用于 objectName 前缀）。
        title: 醒目标题。
        description: 次要描述。
        icon_name: lucide 图标名（用于 EmptyStateWidget 渲染）。
        emoji: 图标缺失时的 emoji 兜底。
    """

    def __init__(
        self,
        mode_id: str,
        title: str,
        description: str,
        icon_name: str = "",
        emoji: str = "🚧",
    ) -> None:
        self._mode_id = mode_id
        self._title = title
        self._description = description
        self._icon_name = icon_name
        self._emoji = emoji
        self._widget: QWidget | None = None
        self._enter_anims: list = []

    def build(self, app_controller: AppController) -> QWidget:
        widget = QWidget()
        widget.setObjectName(f"serialStation{self._mode_id.capitalize()}Panel")
        from PyQt6.QtWidgets import QVBoxLayout

        layout = QVBoxLayout(widget)
        layout.setContentsMargins(0, 0, 0, 0)

        empty = EmptyStateWidget(
            icon_name=self._icon_name,
            title=self._title,
            description=self._description,
            emoji=self._emoji,
            parent=widget,
        )
        layout.addWidget(empty)
        self._widget = widget
        return widget

    def on_enter(self) -> None:
        """进入占位模式时触发淡入 + 上滑入场动画。"""

        if self._widget is None:
            return
        # 停止上一组动画（连续切换时）。
        for anim in self._enter_anims:
            try:
                anim.stop()
            except Exception:
                pass
        self._enter_anims = card_enter(self._widget)
        for anim in self._enter_anims:
            anim.start()

    def on_leave(self) -> None:
        """离开占位模式，停止入场动画。"""

        for anim in self._enter_anims:
            try:
                anim.stop()
            except Exception:
                pass
        self._enter_anims = []
