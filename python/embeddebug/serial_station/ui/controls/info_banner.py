"""可关闭信息条。

info/warning/error/success 四种 kind，顶部 accent 条颜色随 kind 变化。点关闭按钮发
dismissed 信号并 slide-out 动画。用于 OTA 状态、连接错误、协议校验失败等场景的状态
提示。

设计要点：
- 左侧 4px accent 条 + 半透明 kind 软底色 + 居中文本，色相对齐 palette 状态色 token。
- 右侧 × 关闭按钮 hover 切换 accent 高亮，点击 emit dismissed(text) 后启动 slide-up
  动画（高度归零 + 透明度归零），结束后 hide + deleteLater。
- 文本/颜色全部走 palette 常量，无散落硬编码（参见 05-ui-standard §颜色集中管理）。

约束：只依赖 PyQt6 + theme + animations。
"""

from __future__ import annotations

import enum

from PyQt6.QtCore import (
    QEvent,
    QPointF,
    QPropertyAnimation,
    QRect,
    QRectF,
    QSize,
    Qt,
    QTimer,
    pyqtSignal,
)
from PyQt6.QtGui import QColor, QMouseEvent, QPaintEvent, QPainter, QPen
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


# INFO soft 底色常量（palette 未提供 INFO_SOFT，模块顶层集中声明，避免逻辑内散落）。
_INFO_SOFT_BG = "rgba(34, 211, 238, 0.10)"
# WARNING/ERROR/SUCCESS soft 底色复用 palette 的 *_SOFT token。


class BannerKind(enum.Enum):
    """信息条类型枚举。"""

    INFO = "info"
    WARNING = "warning"
    ERROR = "error"
    SUCCESS = "success"


class InfoBanner(QWidget):
    """可关闭信息条控件。

    顶部 accent 条颜色随 ``BannerKind`` 变化，右侧 × 按钮发 ``dismissed`` 信号后
    启动 slide-up 动画并自销毁。
    """

    # 关闭时发出，payload 为当前文本（方便父面板记录是哪条提示被关闭）。
    dismissed = pyqtSignal(str)

    # kind -> (accent_color, soft_bg_color)。accent 用于左侧条与 hover 高亮，
    # soft_bg 用于整体半透明底色。优先复用 palette 常量。
    KIND_COLORS: dict[BannerKind, tuple[str, str]] = {
        BannerKind.INFO: (P.ACCENT, _INFO_SOFT_BG),
        BannerKind.WARNING: (P.WARNING, P.WARNING_SOFT),
        BannerKind.ERROR: (P.ERROR, P.ERROR_SOFT),
        BannerKind.SUCCESS: (P.SUCCESS, P.SUCCESS_SOFT),
    }

    def __init__(
        self,
        text: str = "",
        kind: BannerKind = BannerKind.INFO,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationInfoBanner")

        # 启用鼠标跟踪以驱动 hover 状态切换（mouseMoveEvent 无按下也会触发）。
        self.setMouseTracking(True)
        # 透明背景——paintEvent 自绘 kind 软底色，避免默认 QWidget 底破坏色相。
        self.setAttribute(Qt.WidgetAttribute.WA_StyledBackground, False)

        self._text: str = text
        self._kind: BannerKind = kind
        self._close_hovered: bool = False
        # slide-out 动画目标高度（0），与初始尺寸共同决定动画起止。
        self._max_height: int = 40
        self._anim_out: QPropertyAnimation | None = None

        # 默认尺寸（与 sizeHint 一致）。
        self.setFixedSize(self.sizeHint())

    # ── 属性 getter/setter ──────────────────────────────────────────
    def set_text(self, text: str) -> None:
        """更新提示文本并触发重绘。"""

        self._text = text
        self.update()

    def text(self) -> str:
        """返回当前提示文本。"""

        return self._text

    def set_kind(self, kind: BannerKind) -> None:
        """更新 kind（决定 accent/底色）并触发重绘。"""

        self._kind = kind
        self.update()

    def kind(self) -> BannerKind:
        """返回当前 kind。"""

        return self._kind

    def sizeHint(self) -> QSize:
        """默认提示尺寸：宽 360、高 40。"""

        return QSize(360, 40)

    # ── 几何辅助 ────────────────────────────────────────────────────
    def _close_button_rect(self) -> QRectF:
        """返回关闭按钮命中区域（右侧 12×12，垂直居中）。"""

        right_pad = 10.0
        size = 12.0
        y = (self.height() - size) / 2.0
        return QRectF(self.width() - right_pad - size, y, size, size)

    # ── 绘制 ────────────────────────────────────────────────────────
    def paintEvent(self, event: QPaintEvent) -> None:
        """自绘：底色 + 左 accent 条 + 居中文本 + 右 × 按钮。"""

        accent_color, bg_color = self.KIND_COLORS.get(
            self._kind, (P.ACCENT, _INFO_SOFT_BG)
        )
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        rect = QRectF(self.rect())

        # 1. 半透明 kind 软底色。
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QColor(bg_color))
        painter.drawRoundedRect(rect, 6, 6)

        # 2. 左侧 4px accent 条（全高，圆角与底色对齐视觉一体）。
        accent_w = 4.0
        accent_rect = QRectF(rect.left(), rect.top(), accent_w, rect.height())
        painter.setBrush(QColor(accent_color))
        painter.drawRoundedRect(accent_rect, 2, 2)

        # 3. 文本（垂直居中，左留 accent + padding）。
        text_pen = QPen(QColor(P.TEXT_PRIMARY))
        painter.setPen(text_pen)
        text_rect = QRectF(
            rect.left() + accent_w + 12,
            rect.top(),
            rect.width() - accent_w - 12 - 28,  # 右侧为 × 按钮预留区
            rect.height(),
        )
        painter.drawText(
            text_rect,
            int(Qt.AlignmentFlag.AlignVCenter | Qt.AlignmentFlag.AlignLeft),
            self._text,
        )

        # 4. 右侧 × 关闭按钮（hover 时切 accent 高亮）。
        self._draw_close_button(painter, accent_color)

    def _draw_close_button(self, painter: QPainter, accent_color: str) -> None:
        """绘制右侧 × 关闭图标（hover 时颜色切 accent）。"""

        btn_rect = self._close_button_rect()
        color = QColor(accent_color) if self._close_hovered else QColor(P.TEXT_SECONDARY)
        pen = QPen(color)
        pen.setWidthF(1.6)
        pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)

        inset = 2.0
        x1 = btn_rect.left() + inset
        y1 = btn_rect.top() + inset
        x2 = btn_rect.right() - inset
        y2 = btn_rect.bottom() - inset
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2))
        painter.drawLine(QPointF(x2, y1), QPointF(x1, y2))

    # ── 鼠标交互 ────────────────────────────────────────────────────
    def mousePressEvent(self, event: QMouseEvent) -> None:
        """点击关闭按钮 → emit dismissed(text) → _animate_out。"""

        if event.button() == Qt.MouseButton.LeftButton:
            pos = QPointF(event.position())
            if self._close_button_rect().contains(pos):
                self.dismissed.emit(self._text)
                self._animate_out()
                event.accept()
                return
        event.accept()

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        """跟踪 × 按钮 hover 状态，变化时触发重绘。"""

        pos = QPointF(event.position())
        hovered = self._close_button_rect().contains(pos)
        if hovered != self._close_hovered:
            self._close_hovered = hovered
            self.update()
        event.accept()

    def enterEvent(self, event: QEvent) -> None:
        """进入控件时无操作（hover 由 mouseMove 精确驱动）。"""

        super().enterEvent(event)

    def leaveEvent(self, event: QEvent) -> None:
        """离开控件时复位 hover。"""

        if self._close_hovered:
            self._close_hovered = False
            self.update()
        super().leaveEvent(event)

    # ── slide-out 动画 ─────────────────────────────────────────────
    def _animate_out(self) -> None:
        """slide-up + fade-out 动画；结束后 hide + deleteLater。

        生命周期安全（关键）：``finished`` 信号连接到**绑定方法**而非闭包。
        PyQt 在 receiver（``self``）被 C++ 析构时会自动断开绑定方法连接，
        从而避免 ``finished`` 在已删除对象上分发导致 Windows 访问冲突。
        闭包（普通函数）不在 sip 的生命周期追踪范围内，会在 pytestqt 跨用例
        processEvents 时触发访问冲突。
        """

        if self._anim_out is not None:
            # 已在动画中，避免重复调度。
            return

        start_rect = QRect(self.geometry())
        end_rect = QRect(
            start_rect.x(),
            start_rect.y(),
            start_rect.width(),
            0,  # 高度归零
        )
        anim = QPropertyAnimation(self, b"geometry", self)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        anim.setStartValue(start_rect)
        anim.setEndValue(end_rect)
        anim.setEasingCurve(AnimationTokens.EASE_IN)

        # 绑定方法：PyQt 自动追踪 receiver 生命周期，self 被删除时自动 disconnect。
        anim.finished.connect(self._on_anim_finished)
        self._anim_out = anim
        anim.start()

    def _on_anim_finished(self) -> None:
        """``_animate_out`` 动画完成回调：hide + 推迟 deleteLater 到下一轮事件循环。

        用 ``try/except RuntimeError`` 兜底处理 widget 已被外部 deleteLater 删除
        的边缘情况（PyQt wrapper 仍在但 C++ 已删）。``QTimer.singleShot(0, ...)``
        把 ``deleteLater`` 推到 ``finished`` 分发路径之外，避免在信号回调内同步
        触发二次删除。
        """

        try:
            self.hide()
            QTimer.singleShot(0, self.deleteLater)
        except RuntimeError:
            # wrapped C/C++ object of type InfoBanner has been deleted.
            pass
