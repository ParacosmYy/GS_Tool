"""多段状态栏组件 — 底部固定高度的水平状态条。

对标 MobaXterm 多段状态栏（连接/字节/FPS/编码/时钟）和 VOFA+ 连接状态指示。
每段独立更新，段间用竖线分隔。自包含 QWidget，不依赖 controller/transport。

用法::

    bar = StatusBar(parent)
    bar.set_section("connection", "COM3 @ 115200", icon="cable")
    bar.set_section("rx_tx", "RX: 1.2KB  TX: 256B")
    bar.set_section("fps", "30 fps")
    bar.clear_section("fps")  # 移除 fps 段

约束：只依赖 PyQt6 + theme + icons。无业务逻辑。
"""

from __future__ import annotations

from typing import Dict

from PyQt6.QtWidgets import QHBoxLayout, QLabel, QFrame, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


class StatusBar(QWidget):
    """多段底部状态栏。

    每段由 key 标识，可动态增删。段间用 1px 竖线分隔。
    高度固定 24px，背景 BG_PANEL，文字 TEXT_SECONDARY。
    """

    FIXED_HEIGHT = 24

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationStatusBar")
        self.setFixedHeight(self.FIXED_HEIGHT)

        self._sections: Dict[str, QLabel] = {}
        self._dividers: list[QFrame] = []

        self._layout = QHBoxLayout(self)
        self._layout.setContentsMargins(
            T.SPACING_INT_LG, 0, T.SPACING_INT_LG, 0
        )
        self._layout.setSpacing(T.SPACING_INT_LG)

        self._add_stretch_end()

    def set_section(self, key: str, text: str) -> None:
        """更新或新增一段。key 已存在则更新文本，否则追加到末尾（stretch 前）。"""

        if key in self._sections:
            self._sections[key].setText(text)
            return

        # 插入到 stretch 之前。
        label = QLabel(text, self)
        label.setObjectName(f"serialStationStatusBar_{key}")
        label.setStyleSheet(
            f"color: {P.TEXT_SECONDARY}; background: transparent; "
            f"font-size: {T.FONT_XS};"
        )
        # 在 stretch 前插入：先移除 stretch，加 label + divider，再加回 stretch。
        self._layout.takeAt(self._layout.count() - 1)  # 移除尾部 stretch

        # 段间分隔线（首段前不加）。
        if self._sections:
            divider = QFrame(self)
            divider.setFixedWidth(1)
            divider.setStyleSheet(
                f"background-color: {P.BORDER}; border: none;"
            )
            self._layout.addWidget(divider)
            self._dividers.append(divider)

        self._layout.addWidget(label)
        self._sections[key] = label
        self._add_stretch_end()

    def clear_section(self, key: str) -> None:
        """移除一段。不存在时静默跳过。"""

        if key not in self._sections:
            return

        label = self._sections.pop(key)
        self._layout.removeWidget(label)
        label.deleteLater()

        # 移除多余的分隔线（段数 == 分隔线数 + 1，所以删一段删一线）。
        if self._dividers:
            divider = self._dividers.pop()
            self._layout.removeWidget(divider)
            divider.deleteLater()

        # 如果删的是第一段，还需移除新的第一段前的分隔线。
        if self._sections and self._dividers:
            # 检查布局第一个 widget 是否是 divider（不该出现在最前）。
            first = self._layout.itemAt(0)
            if first and first.widget() and first.widget().isWidgetType():
                w = first.widget()
                if w not in self._sections.values():
                    self._dividers.pop(0)
                    self._layout.removeWidget(w)
                    w.deleteLater()

    def section_text(self, key: str) -> str:
        """返回指定段的当前文本。不存在返回空串。"""

        label = self._sections.get(key)
        return label.text() if label else ""

    def section_count(self) -> int:
        """返回当前段数。"""

        return len(self._sections)

    def _add_stretch_end(self) -> None:
        """在布局末尾添加 stretch（右对齐效果）。"""

        self._layout.addStretch(1)
