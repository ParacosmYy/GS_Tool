"""可折叠卡片组件 — 带标题行（箭头 + 标题）+ 可显隐内容区。

用于把连接配置侧栏的不常用项（高级串口参数、TCP/UDP 端点）默认折叠，
释放左栏空间。点击标题行切换展开/折叠，箭头图标在 chevron-right/down 间切。

Batch 4 (B2) 改进：折叠/展开改用 ``CollapseAnimation`` 高度动画，不再硬切
``setVisible``（违反铁律 18 禁止突然出现/消失）。展开时动画 ``maximumHeight``
从 0 → 内容自然高度（OutCubic 240ms），折叠时反向。

设计要点：
- 内容区用 ``maximumHeight`` 动画显隐，**不 reparent / 不销毁** 子控件，保证
  ``findChild`` 与 action 模块契约不受影响（控件仍在 widget 树）。
- 图标走 ``IconManager``（chevron-down / chevron-right），缺图不抛异常。

约束：本模块只构建 UI 容器，不访问 controller/transport/protocol/service。
"""

from __future__ import annotations

from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.animations.collapse import CollapseAnimation
from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.theme import palette as P

_CARD = "serialStationCollapsibleCard"
_HEADER = "serialStationCollapsibleHeader"
_ARROW = "serialStationCollapsibleArrow"
_TITLE = "serialStationCollapsibleTitle"
_BODY = "serialStationCollapsibleBody"


class CollapsibleCard(QFrame):
    """可折叠卡片：标题行（点击切换）+ 内容区（高度动画显隐）。"""

    def __init__(self, parent: QWidget, title: str, expanded: bool = False) -> None:
        super().__init__(parent)
        self.setObjectName(_CARD)
        self._expanded = expanded
        self._collapse_anim = None  # 进行中的折叠/展开动画引用

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
        """切换展开/折叠（高度动画 + 箭头图标）。

        改进（Batch 4）：用 ``CollapseAnimation`` 动 ``maximumHeight``，
        不再硬切 ``setVisible``。展开目标高度取内容区 ``sizeHint().height()``，
        最小 1 防止 0 高度导致布局塌陷。
        """

        if expanded == self._expanded:
            return
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
        """同步内容区高度动画与箭头图标到当前展开状态。

        折叠态：maximumHeight 动画到 0（内容被裁切不可见，但仍 in-tree）。
        展开态：maximumHeight 动画到内容自然高度（解除裁切约束）。
        """

        # 停止进行中的动画。
        if self._collapse_anim is not None:
            try:
                self._collapse_anim.stop()
            except Exception:
                pass
            self._collapse_anim = None

        if self._expanded:
            # 展开先解除 maximumHeight 约束并 show，再从当前高度动画到自然高度。
            self._body_widget.setMaximumHeight(16777215)  # Qt 默认最大值
            self._body_widget.show()
            # 取内容自然高度作为动画终点。
            target = max(1, self._body_widget.sizeHint().height())
            self._collapse_anim = CollapseAnimation.expand(self._body_widget, target)
        else:
            # 折叠：从当前高度动画到 0。
            self._collapse_anim = CollapseAnimation.collapse(self._body_widget)

        if self._collapse_anim is not None:
            self._collapse_anim.start()

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
