"""空状态组件 — 图标 + 标题 + 描述 + 可选 CTA。

用于列表/日志/面板在无数据时的友好占位，替代朴素的「只有两个 QLabel」占位。
对齐 Linear/Arc/VOFA+ 的空状态语言：大图标 + 醒目标题 + 次要描述 + 可选行动按钮。

Batch 5 (C1) 新建。诊断报告指出全仓无 EmptyStateWidget，占位面板朴素到只有
两个 QLabel，用户落到占位页得不到任何动作指引。

设计要点：
- 图标走 IconManager（lucide），缺失图标时用 emoji 兜底（不空屏）。
- 标题用主文本色 + 较大字号，描述用次文本色 + 较小字号。
- 可选 CTA 按钮（如「返回串口」「查看文档」）。
- 入场带淡入动画（panel_animations.fade_in），不突兀。

约束：只依赖 PyQt6 + icons + theme + animations。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QLabel, QPushButton, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.micro_interactions import install_hover_lift
from embeddebug.serial_station.ui.theme import palette as P


class EmptyStateWidget(QWidget):
    """空状态占位：图标 + 标题 + 描述 + 可选 CTA。

    用法::

        empty = EmptyStateWidget(
            icon_name="inbox",
            title="暂无日志",
            description="连接设备后将在此显示收发数据",
        )
        layout.addWidget(empty)

    Args:
        icon_name: lucide 图标名（缺失时用 emoji 兜底）。
        title: 醒目标题（主文本色，较大字号）。
        description: 次要描述（次文本色，较小字号，可换行）。
        cta_text: 可选行动按钮文字（不传则不显示按钮）。
        on_cta: CTA 按钮回调。
        emoji: 图标缺失时的 emoji 兜底（默认 "📭"）。
    """

    def __init__(
        self,
        icon_name: str = "",
        title: str = "",
        description: str = "",
        cta_text: str | None = None,
        on_cta=None,
        emoji: str = "📭",
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationEmptyState")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(32, 32, 32, 32)
        layout.setSpacing(12)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # 图标（优先 lucide，缺失用 emoji）。
        self._icon_label = QLabel(self)
        self._icon_label.setObjectName("serialStationEmptyStateIcon")
        self._icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        icon = button_icon(icon_name, color=P.TEXT_MUTED) if icon_name else None
        if icon is not None and not icon.isNull():
            self._icon_label.setPixmap(icon.pixmap(56, 56))
        else:
            self._icon_label.setText(emoji)
            font = self._icon_label.font()
            font.setPointSize(40)
            self._icon_label.setFont(font)
        layout.addWidget(self._icon_label)

        # 标题。
        self._title_label = QLabel(title, self)
        self._title_label.setObjectName("serialStationEmptyStateTitle")
        self._title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._title_label.setWordWrap(True)
        title_font = self._title_label.font()
        title_font.setPointSize(13)
        title_font.setBold(True)
        self._title_label.setFont(title_font)
        layout.addWidget(self._title_label)

        # 描述。
        self._desc_label = QLabel(description, self)
        self._desc_label.setObjectName("serialStationEmptyStateDescription")
        self._desc_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._desc_label.setWordWrap(True)
        desc_font = self._desc_label.font()
        desc_font.setPointSize(10)
        self._desc_label.setFont(desc_font)
        layout.addWidget(self._desc_label)

        # 可选 CTA 按钮（装 hover lift + 按压动画，Batch 5）。
        if cta_text:
            self._cta_button = QPushButton(cta_text, self)
            self._cta_button.setObjectName("serialStationEmptyStateCta")
            self._cta_button.setCursor(Qt.CursorShape.PointingHandCursor)
            # CTA 是空状态的主操作，装完整微交互（hover 上浮 + accent tint 阴影）。
            install_hover_lift(self._cta_button)
            if on_cta is not None:
                self._cta_button.clicked.connect(on_cta)
            layout.addWidget(self._cta_button)
        else:
            self._cta_button = None

    def set_title(self, title: str) -> None:
        self._title_label.setText(title)

    def set_description(self, description: str) -> None:
        self._desc_label.setText(description)
