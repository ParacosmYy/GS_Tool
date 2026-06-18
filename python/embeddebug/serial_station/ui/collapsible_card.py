"""可折叠卡片组件 — 带标题行（箭头 + 标题）+ 可显隐内容区。

用于把连接配置侧栏的不常用项（高级串口参数、TCP/UDP 端点）默认折叠，
释放左栏空间。点击标题行切换展开/折叠，箭头图标在 chevron-right/down 间切。

设计要点：
- 内容区用 ``setVisible`` 显隐，**不 reparent / 不销毁** 子控件，保证
  ``findChild`` 与 action 模块契约不受影响（控件只是不可见，仍在 widget 树）。
- 图标走 ``IconManager``（chevron-down / chevron-right），缺图不抛异常。

约束：本模块只构建 UI 容器，不访问 controller/transport/protocol/service。
"""

from __future__ import annotations

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.theme import palette as P
_CARD = "serialStationCollapsibleCard"
_HEADER = "serialStationCollapsibleHeader"
_ARROW = "serialStationCollapsibleArrow"
_TITLE = "serialStationCollapsibleTitle"
_BODY = "serialStationCollapsibleBody"


class CollapsibleCard(QFrame):
    """可折叠卡片：标题行（点击切换）+ 内容区（显隐）。"""

    def __init__(self, parent: QWidget, title: str, expanded: bool = False) -> None:
        super().__init__(parent)
        self.setObjectName(_CARD)
        self._expanded = expanded

        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        layout.setSpacing(4)

        self._header = self._build_header(title)
        self._header.installEventFilter(self)
        layout.addWidget(self._header)

        self._body_widget = QWidget(self)
        self._body_widget.setObjectName(_BODY)
        self._body_layout = QVBoxLayout(self._body_widget)
        self._body_layout.setContentsMargins(0, 4, 0, 0)
        self._body_layout.setSpacing(6)
        layout.addWidget(self._body_widget)

        self._apply_expanded_state()

    def body_layout(self) -> QVBoxLayout:
        """返回内容区 layout，供调用方 addWidget 装子控件。"""

        return self._body_layout

    def is_expanded(self) -> bool:
        return self._expanded

    def set_expanded(self, expanded: bool) -> None:
        """切换展开/折叠（内容区显隐 + 箭头图标）。"""

        self._expanded = expanded
        self._apply_expanded_state()

    def toggle(self) -> None:
        """切换展开状态（标题行点击槽）。"""

        self.set_expanded(not self._expanded)

    # ── 内部 ────────────────────────────────────────────────────────
    def _build_header(self, title: str) -> QWidget:
        header = QWidget(self)
        header.setObjectName(_HEADER)
        header.setCursor(Qt.CursorShape.PointingHandCursor)
        row = QHBoxLayout(header)
        row.setContentsMargins(0, 0, 0, 0)
        row.setSpacing(6)

        self._arrow = QLabel(header)
        self._arrow.setObjectName(_ARROW)
        row.addWidget(self._arrow)

        title_label = QLabel(title, header)
        title_label.setObjectName(_TITLE)
        title_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        row.addWidget(title_label)
        row.addStretch(1)
        return header

    def _apply_expanded_state(self) -> None:
        """同步内容区可见性与箭头图标到当前展开状态。"""

        self._body_widget.setVisible(self._expanded)
        icon_name = "chevron-down" if self._expanded else "chevron-right"
        icon = button_icon(icon_name, color=P.TEXT_MUTED)
        if not icon.isNull():
            self._arrow.setPixmap(icon.pixmap(14, 14))

    def eventFilter(self, watched: object, event: object) -> bool:
        """标题行点击切换折叠（仅响应 header 自身的鼠标按下）。"""

        if watched is self._header and getattr(event, "type", lambda: None)() == QEvent.Type.MouseButtonPress:
            if getattr(event, "button", None) == Qt.MouseButton.LeftButton:
                self.toggle()
                return True
        return super().eventFilter(watched, event)
