"""分隔线组件 — 带可选标签的水平分隔线。

用于面板内分组（如设置面板区分「连接设置」「显示选项」「快捷键」）。
对标 MobaXterm / VOFA+ 的分组分隔线。

用法::

    divider = Divider("连接设置", parent)
    divider2 = Divider(parent=parent)  # 无标签纯线

约束：只依赖 PyQt6 + theme。无业务逻辑。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QHBoxLayout, QLabel, QFrame, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


class Divider(QWidget):
    """水平分隔线，可带左侧标签。"""

    def __init__(self, label: str = "", parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationDivider")
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, T.SPACING_INT_SM, 0, T.SPACING_INT_SM)
        layout.setSpacing(T.SPACING_INT_SM)

        if label:
            lbl = QLabel(label, self)
            lbl.setObjectName("serialStationDividerLabel")
            lbl.setStyleSheet(
                f"color: {P.TEXT_MUTED}; background: transparent; "
                f"font-size: {T.FONT_XS}; font-weight: {T.FONT_WEIGHT_SEMIBOLD};"
            )
            layout.addWidget(lbl)

        line = QFrame(self)
        line.setFrameShape(QFrame.Shape.HLine)
        line.setStyleSheet(f"color: {P.BORDER}; border: none;")
        layout.addWidget(line, 1)
