"""键盘快捷键徽章组件 — 小型药丸标签显示快捷键提示。

对标 VS Code / MobaXterm / Linear 的快捷键提示样式：等宽字体、圆角药丸、
弱色调。用于按钮右侧、菜单项尾部、tooltip 内嵌。

用法::

    hint = KeyboardShortcut("Ctrl+K", parent)
    hint2 = KeyboardShortcut("Ctrl+Shift+P")

约束：只依赖 PyQt6 + theme。无业务逻辑。
"""

from __future__ import annotations

from PyQt6.QtGui import QMouseEvent
from PyQt6.QtWidgets import QLabel, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


class KeyboardShortcut(QLabel):
    """键盘快捷键徽章（药丸标签）。

    自带样式（等宽字体 + 圆角 + 弱色调），无需额外 QSS。
    setObjectName serialStationKeyHint 供全局 QSS 覆盖。
    """

    def __init__(self, text: str, parent: QWidget | None = None) -> None:
        super().__init__(text, parent)
        self.setObjectName("serialStationKeyHint")
        self._apply_style()

    def _apply_style(self) -> None:
        """应用药丸样式。"""
        self.setStyleSheet(
            f"""
            QLabel#serialStationKeyHint {{
                background-color: {P.BG_PANEL_RAISED};
                color: {P.TEXT_MUTED};
                border: {T.BORDER_THIN} solid {P.BORDER};
                border-radius: 4px;
                padding: 1px 5px;
                font-family: {T.FONT_FAMILY_MONO};
                font-size: {T.FONT_XS};
            }}
            """
        )

    def mousePressEvent(self, event: QMouseEvent) -> None:
        """徽章不可点击，吞掉事件防止穿透到父控件。"""
        event.accept()
