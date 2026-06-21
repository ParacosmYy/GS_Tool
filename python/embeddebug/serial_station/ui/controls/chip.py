"""Material Design 风格 Chip 标签。

圆角胶囊形标签，可选可关闭。支持 selected toggle、click signal、removed signal。
用于标签筛选、命令历史标签、协议筛选。

实现要点：
- 自绘 QWidget（不依赖 QSS），paintEvent 根据 _selected / _hovered 状态切换
  背景色（默认 / hover / 选中）。
- 文本居中显示，右侧保留 close button 区域（10×10），可选关闭（_removable）。
- 可选状态（_selectable）：点击 chip 主体切换 _selected，发射 toggled 信号；
  否则只发射 clicked。
- 鼠标移动追踪 close button 悬停状态，绘制 × 颜色随 _close_hovered 变化。

约束：只依赖 PyQt6 + theme + animations。无业务逻辑、无定时器、无线程。
"""

from __future__ import annotations

from PyQt6.QtCore import QEvent, QRectF, QSize, Qt, pyqtSignal
from PyQt6.QtGui import QColor, QFontMetrics, QPainter, QPaintEvent, QMouseEvent
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


# 固定布局常量（chip 高度、左右 padding、close button 区域宽度）。
_CHIP_HEIGHT = 28
_CHIP_PADDING_H = 12
_CLOSE_BUTTON_SIZE = 10
_CLOSE_AREA_WIDTH = 20
_BORDER_RADIUS = _CHIP_HEIGHT // 2


class Chip(QWidget):
    """Material Design 风格 Chip 标签控件。

    圆角胶囊形标签，可选可关闭，支持悬停与选中态自绘。
    常用于标签筛选、命令历史标签、协议筛选场景。

    信号：
        clicked: chip 主体被点击（任意按钮）。
        toggled(bool): 选中状态变化（仅 selectable 模式有效）。
        removed(str): close button 被点击，参数为 chip 当前文本。
    """

    clicked = pyqtSignal()
    toggled = pyqtSignal(bool)
    removed = pyqtSignal(str)

    def __init__(
        self,
        text: str = "",
        parent=None,
        removable: bool = True,
        selectable: bool = False,
        selected: bool = False,
    ) -> None:
        """初始化 Chip。

        参数：
            text: chip 显示的文本。
            parent: 父控件。
            removable: 是否显示 close button 并允许 removed 信号。
            selectable: 是否允许点击切换 _selected。
            selected: 初始 _selected 状态。
        """

        super().__init__(parent)
        self.setObjectName("serialStationChip")
        self.setMouseTracking(True)
        self.setCursor(Qt.CursorShape.PointingHandCursor)

        self._text: str = text
        self._removable: bool = removable
        self._selectable: bool = selectable
        self._selected: bool = bool(selected) if selectable else False
        self._hovered: bool = False
        self._close_hovered: bool = False

        # 固定高度，宽度随 sizeHint 自适应。
        self.setFixedHeight(_CHIP_HEIGHT)

    # ── 公共 API ─────────────────────────────────────────────────────
    def text(self) -> str:
        """返回当前文本。"""

        return self._text

    def set_text(self, text: str) -> None:
        """设置 chip 文本，触发重绘与重新布局。"""

        self._text = text
        self.updateGeometry()
        self.update()

    def is_removable(self) -> bool:
        """是否可关闭（显示 close button）。"""

        return self._removable

    def set_removable(self, removable: bool) -> None:
        """切换 removable 状态。"""

        self._removable = removable
        self.updateGeometry()
        self.update()

    def is_selected(self) -> bool:
        """返回当前选中状态。"""

        return self._selected

    def set_selected(self, selected: bool) -> None:
        """设置 _selected 状态。

        仅在 _selectable=True 时生效；若状态发生变化则发射 toggled 信号。
        """

        if not self._selectable:
            return
        new_state = bool(selected)
        if new_state == self._selected:
            return
        self._selected = new_state
        self.toggled.emit(self._selected)
        self.update()

    def set_selectable(self, selectable: bool) -> None:
        """切换 selectable 能力（关闭时 _selected 强制为 False）。"""

        self._selectable = selectable
        if not selectable and self._selected:
            self._selected = False
            self.update()

    # ── 尺寸 ─────────────────────────────────────────────────────────
    def sizeHint(self) -> QSize:
        """基于文本宽度 + close button 区域返回建议尺寸。"""

        fm = QFontMetrics(self.font())
        text_w = fm.horizontalAdvance(self._text)
        right = _CLOSE_AREA_WIDTH if self._removable else _CHIP_PADDING_H
        width = _CHIP_PADDING_H + text_w + right
        return QSize(max(width, _CHIP_HEIGHT * 2), _CHIP_HEIGHT)

    # ── 绘制 ─────────────────────────────────────────────────────────
    def paintEvent(self, event: QPaintEvent) -> None:
        """根据状态绘制圆角胶囊背景、文本与 close button。"""

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        rect = QRectF(self.rect()).adjusted(0.5, 0.5, -0.5, -0.5)

        # 背景色：选中 > hover > 默认。
        bg_color = self._resolve_bg_color()
        painter.setBrush(bg_color)
        painter.setPen(self._resolve_border_pen())
        painter.drawRoundedRect(rect, _BORDER_RADIUS, _BORDER_RADIUS)

        # 文本。
        text_rect = self._text_rect()
        text_color = self._resolve_text_color()
        painter.setPen(text_color)
        painter.drawText(
            text_rect,
            int(Qt.AlignmentFlag.AlignVCenter | Qt.AlignmentFlag.AlignLeft),
            self._text,
        )

        # close button（可选）。
        if self._removable:
            self._paint_close_button(painter)

    def _resolve_bg_color(self) -> QColor:
        """根据 _selected / _hovered 选择背景色。"""

        if self._selected:
            return QColor(P.ACCENT_SOFT)
        if self._hovered:
            return QColor(P.BG_PANEL_RAISED)
        return QColor(P.BG_PANEL_RAISED)

    def _resolve_text_color(self) -> QColor:
        """根据 _selected 选择文本色（选中态用 ACCENT_HOVER 强调）。"""

        if self._selected:
            return QColor(P.ACCENT_HOVER)
        if self._hovered:
            return QColor(P.TEXT_PRIMARY)
        return QColor(P.TEXT_SECONDARY)

    def _resolve_border_pen(self) -> QColor:
        """边框色：选中态用 ACCENT_BORDER，否则用淡灰描边。"""

        if self._selected:
            return QColor(P.ACCENT_BORDER)
        return QColor("rgba(255, 255, 255, 28)")

    def _paint_close_button(self, painter: QPainter) -> None:
        """绘制 close button（× 图标），颜色随 _close_hovered 变化。"""

        close_rect = self._close_button_rect()
        color = QColor(P.TEXT_PRIMARY) if self._close_hovered else QColor(P.TEXT_SECONDARY)
        painter.setPen(color)
        # 简单 × ：两条对角线。
        pad = 2.0
        r = close_rect.adjusted(pad, pad, -pad, -pad)
        painter.drawLine(r.topLeft(), r.bottomRight())
        painter.drawLine(r.topRight(), r.bottomLeft())

    # ── 几何辅助 ─────────────────────────────────────────────────────
    def _close_button_rect(self) -> QRectF:
        """close button 命中区域（右侧 20×高度，居中 10×10）。"""

        if not self._removable:
            return QRectF()
        x = self.width() - _CLOSE_AREA_WIDTH
        # 在 close_area 内部居中放置 _CLOSE_BUTTON_SIZE × _CLOSE_BUTTON_SIZE 方块。
        offset = (_CLOSE_AREA_WIDTH - _CLOSE_BUTTON_SIZE) / 2.0
        y = (self.height() - _CLOSE_BUTTON_SIZE) / 2.0
        return QRectF(x + offset, y, _CLOSE_BUTTON_SIZE, _CLOSE_BUTTON_SIZE)

    def _text_rect(self) -> QRectF:
        """文本绘制区域：左侧 padding 起，到 close button 左边（或右侧 padding）。"""

        fm = QFontMetrics(self.font())
        text_w = fm.horizontalAdvance(self._text)
        if self._removable:
            right_limit = self.width() - _CLOSE_AREA_WIDTH
        else:
            right_limit = self.width() - _CHIP_PADDING_H
        left = _CHIP_PADDING_H
        width = max(0, right_limit - left)
        return QRectF(left, 0, min(width, text_w), self.height())

    # ── 事件 ─────────────────────────────────────────────────────────
    def mousePressEvent(self, event: QMouseEvent) -> None:
        """处理点击：close button 命中 → removed；否则切换/只发射 clicked。"""

        if event.button() == Qt.MouseButton.LeftButton:
            pos = event.position()
            if self._removable and self._close_button_rect().contains(pos):
                # close button 命中：发射 removed 并关闭自身。
                self.removed.emit(self._text)
                self.close()
                event.accept()
                return
            if self._selectable:
                # 主体命中且可选：切换 _selected（set_selected 会发射 toggled）。
                self.set_selected(not self._selected)
            self.clicked.emit()
        event.accept()

    def enterEvent(self, event: QEvent) -> None:
        """鼠标进入：更新 _hovered 并重绘。"""

        self._hovered = True
        self.update()
        super().enterEvent(event)

    def leaveEvent(self, event: QEvent) -> None:
        """鼠标离开：清除 _hovered / _close_hovered 并重绘。"""

        self._hovered = False
        if self._close_hovered:
            self._close_hovered = False
        self.update()
        super().leaveEvent(event)

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        """跟踪 close button 悬停状态，变化时重绘。"""

        if not self._removable:
            return
        pos = event.position()
        inside = self._close_button_rect().contains(pos)
        if inside != self._close_hovered:
            self._close_hovered = inside
            self.update()
        event.accept()
