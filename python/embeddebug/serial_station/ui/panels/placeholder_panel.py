"""占位模式面板 — 用于尚未实现的功能模式（RTT/设置）。

显示模式名 + "即将推出" 占位，objectName 带模式前缀，便于 QSS 与后续替换。
新增真实模式时，用对应实现替换注册即可。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.app.app_controller import AppController


class PlaceholderPanel:
    """占位 ModePanel：标题 + 副标题 + 即将推出提示。"""

    def __init__(self, mode_id: str, title: str, description: str) -> None:
        self._mode_id = mode_id
        self._title = title
        self._description = description

    def build(self, app_controller: AppController) -> QWidget:
        widget = QWidget()
        widget.setObjectName(f"serialStation{self._mode_id.capitalize()}Panel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(32, 32, 32, 32)
        layout.setSpacing(10)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        title = QLabel(self._title, widget)
        title.setObjectName(f"serialStation{self._mode_id.capitalize()}Title")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(title)

        desc = QLabel(self._description, widget)
        desc.setObjectName(f"serialStation{self._mode_id.capitalize()}Description")
        desc.setAlignment(Qt.AlignmentFlag.AlignCenter)
        desc.setWordWrap(True)
        layout.addWidget(desc)
        return widget

    def on_enter(self) -> None:
        """占位模式无生命周期动作。"""

    def on_leave(self) -> None:
        """占位模式无生命周期动作。"""
