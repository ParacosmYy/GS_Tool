"""富文本 tooltip。标题 + 正文 + 可选图标，fade 过渡显示。

比 ``QToolTip`` 更丰富的视觉层次，用于工具栏按钮详细说明、状态徽标详细解释。
``RichTooltip`` 作为独立顶层 ``Qt.ToolTip`` 窗口浮于父控件之上：标题（粗体
``TEXT_PRIMARY``）+ 正文（``TEXT_SECONDARY`` 自动换行）+ 可选 16×16 图标，圆角面板
底 + accent 边框 + 暗外环近似投影（避免 QGraphicsEffect 互斥），``windowOpacity``
fade in/out 对齐 ``DURATION_INSTANT``。

通过 ``install_tooltip(widget, title, body, icon)`` 一行挂接到任意 QWidget，
自动接管 enterEvent/leaveEvent（monkey-patch，与 ``micro_interactions.install_*``
同范式）。``_installed`` 字典以 ``id(widget)`` 为键，重复 ``install_tooltip`` 复用。

约束：仅依赖 PyQt6 + theme + animations，无业务层耦合。
"""

from __future__ import annotations


from PyQt6.QtCore import (
    QPoint, QRect, QSize, QTimer, QPropertyAnimation, Qt,
)
from PyQt6.QtGui import QColor, QFont, QPainter, QPaintEvent, QPixmap
from PyQt6.QtWidgets import QApplication, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P

# 布局常量。
_PADDING = 12
_SPACING_TITLE_BODY = 4
_ICON_SIZE = 16
_ICON_TEXT_GAP = 8
_CORNER_RADIUS = 6
_DEFAULT_W = 240
_DEFAULT_H = 80
_POSITION_OFFSET = 8


def _parse_color(token: str) -> QColor:
    """把 palette token（#hex 或 rgba()）解析为 QColor，失败回退透明。"""

    color = QColor(token)
    return color if color.isValid() else QColor(0, 0, 0, 0)


class RichTooltip(QWidget):
    """富文本 tooltip：标题 + 正文 + 可选图标，fade 过渡。

    用 ``setWindowFlags(ToolTip | FramelessWindowHint)`` 作为顶层窗口浮出，
    不抢焦点。``show_for(widget)`` 在 300ms 延迟后定位到目标右上方并淡入；
    ``hide_immediately()`` 立即触发淡出（或未显示时直接放弃）。
    """

    def __init__(
        self,
        title: str,
        body: str,
        icon: QPixmap | None = None,
        parent=None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationRichTooltip")
        self.setWindowFlags(
            Qt.WindowType.ToolTip | Qt.WindowType.FramelessWindowHint
        )
        self._title: str = title
        self._body: str = body
        self._icon: QPixmap | None = icon
        self._opacity_anim: QPropertyAnimation | None = None
        self._target_widget: QWidget | None = None
        self._pending_timer: QTimer | None = None
        self.setWindowOpacity(0.0)
        self.resize(self.sizeHint())

    # ── 访问器 ────────────────────────────────────────────────────
    def title(self) -> str:
        """返回标题文本。"""
        return self._title

    def body(self) -> str:
        """返回正文文本。"""
        return self._body

    def icon(self) -> QPixmap | None:
        """返回图标（可能为 None）。"""
        return self._icon

    def sizeHint(self) -> QSize:
        """基于标题/正文/图标估算默认尺寸（~240×80）。"""
        return QSize(_DEFAULT_W, _DEFAULT_H)

    # ── 绘制 ──────────────────────────────────────────────────────
    def paintEvent(self, event: QPaintEvent) -> None:
        """自绘圆角面板 + 外环近似投影 + 标题 + 正文 + 可选图标。"""

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        rect = self.rect()

        # 外环（略暗、略大圆角矩形）近似投影，避免 QGraphicsEffect 互斥。
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(_parse_color(P.SHADOW_POPOVER))
        painter.drawRoundedRect(rect.adjusted(0, 1, -1, -1), _CORNER_RADIUS + 1, _CORNER_RADIUS + 1)

        # 主体面板底（轻微透出底层，营造玻璃浮层感）。
        bg = _parse_color(P.BG_PANEL_RAISED)
        if bg.alpha() == 255:
            bg.setAlpha(245)
        painter.setBrush(bg)
        border = _parse_color(P.ACCENT_BORDER)
        if border.alpha() == 0:
            border = _parse_color(P.ACCENT)
            border.setAlpha(80)
        painter.setPen(border)
        painter.drawRoundedRect(rect.adjusted(0, 0, -1, -1), _CORNER_RADIUS, _CORNER_RADIUS)

        # 文本 X 起点：有图标则预留图标宽度 + 间距，否则从内边距起。
        text_x = _PADDING
        if self._icon is not None and not self._icon.isNull():
            text_x += _ICON_SIZE + _ICON_TEXT_GAP
            painter.drawPixmap(
                QRect(_PADDING, _PADDING, _ICON_SIZE, _ICON_SIZE),
                self._icon, self._icon.rect(),
            )

        text_w = rect.width() - text_x - _PADDING
        if text_w <= 0:
            return

        # 标题：粗体 TEXT_PRIMARY，顶部对齐。
        title_font = QFont(self.font())
        title_font.setBold(True)
        painter.setFont(title_font)
        painter.setPen(_parse_color(P.TEXT_PRIMARY))
        title_rect = QRect(text_x, _PADDING, text_w, painter.fontMetrics().height())
        painter.drawText(title_rect, Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignTop, self._title)

        # 正文：常规 TEXT_SECONDARY，标题下方，自动换行。
        painter.setFont(QFont(self.font()))
        painter.setPen(_parse_color(P.TEXT_SECONDARY))
        body_top = title_rect.bottom() + _SPACING_TITLE_BODY
        body_rect = QRect(text_x, body_top, text_w, rect.height() - body_top - _PADDING)
        flags = Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignTop | Qt.TextFlag.TextWordWrap
        painter.drawText(body_rect, flags, self._body)

    # ── 显示 / 隐藏 ───────────────────────────────────────────────
    def show_for(self, widget: QWidget, delay_ms: int = 300) -> None:
        """为目标 widget 安排一次显示：延迟 delay_ms 后定位并淡入。"""

        self._target_widget = widget
        if self._pending_timer is not None:
            self._pending_timer.stop()
        timer = QTimer(self)
        timer.setSingleShot(True)
        timer.timeout.connect(self._do_show)
        self._pending_timer = timer
        timer.start(delay_ms)

    def _do_show(self) -> None:
        """延迟到期后实际显示：定位到目标右上、show、淡入。"""

        self._pending_timer = None
        target = self._target_widget
        if target is None:
            return
        top_right = target.mapToGlobal(QPoint(target.width(), 0))
        pos = QPoint(
            top_right.x() + _POSITION_OFFSET,
            top_right.y() - self.height() - _POSITION_OFFSET,
        )
        # 越界回退：向右超出屏幕改放到控件左上方；上方越界改放下方。
        screen = QApplication.primaryScreen()
        if screen is not None:
            avail = screen.availableGeometry()
            if pos.x() + self.width() > avail.right():
                pos.setX(
                    target.mapToGlobal(QPoint(0, 0)).x()
                    - self.width()
                    - _POSITION_OFFSET
                )
            if pos.y() < avail.top():
                pos.setY(top_right.y() + _POSITION_OFFSET)
        self.move(pos)
        self.show()
        self._fade(1.0, AnimationTokens.DURATION_INSTANT)

    def hide_immediately(self) -> None:
        """立即触发淡出；若尚未 show 则取消挂起的定时器即可。"""

        if self._pending_timer is not None:
            self._pending_timer.stop()
            self._pending_timer = None
        if not self.isVisible():
            self.setWindowOpacity(0.0)
            return
        self._fade(0.0, AnimationTokens.DURATION_INSTANT)

    def _fade(
        self, target_opacity: float, duration_ms: int
    ) -> QPropertyAnimation:
        """驱动 ``windowOpacity`` 到 target_opacity。

        target==0 时完成会调用 ``self.hide()``，避免留下不可见却仍在
        焦点链中的窗口（影响下一轮 show 的初始状态）。
        """

        if self._opacity_anim is not None:
            self._opacity_anim.stop()
        anim = QPropertyAnimation(self, b"windowOpacity", self)
        anim.setDuration(duration_ms)
        anim.setStartValue(self.windowOpacity())
        anim.setEndValue(target_opacity)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        if target_opacity <= 0.0:
            anim.finished.connect(self.hide)
        self._opacity_anim = anim
        anim.start()
        return anim


# ── 模块级 install/uninstall ────────────────────────────────────────
# 以 id(widget) 为键的注册表，重复 install 复用同一 RichTooltip 实例。
_installed: dict[int, RichTooltip] = {}


def install_tooltip(
    widget: QWidget,
    title: str,
    body: str,
    icon: QPixmap | None = None,
) -> RichTooltip:
    """为 widget 挂接 RichTooltip，enter 显示、leave 隐藏。

    与 ``micro_interactions.install_*`` 同范式：monkey-patch widget 的
    ``enterEvent`` / ``leaveEvent``，无需继承或替换类。
    重复调用同一 widget 复用已存在的实例，仅刷新 title/body/icon。
    返回挂接的 RichTooltip（可用于后续 ``show_for`` 手动触发）。
    """

    key = id(widget)
    existing = _installed.get(key)
    if existing is not None:
        existing._title = title
        existing._body = body
        existing._icon = icon
        existing.resize(existing.sizeHint())
        existing.update()
        return existing

    tooltip = RichTooltip(title, body, icon)
    _installed[key] = tooltip

    original_enter = widget.enterEvent
    original_leave = widget.leaveEvent

    def _patched_enter(event):
        try:
            original_enter(event)
        except Exception:
            pass
        tooltip.show_for(widget)

    def _patched_leave(event):
        try:
            original_leave(event)
        except Exception:
            pass
        tooltip.hide_immediately()

    widget.enterEvent = _patched_enter  # type: ignore[assignment]
    widget.leaveEvent = _patched_leave  # type: ignore[assignment]
    widget.destroyed.connect(lambda _obj=None: uninstall_tooltip(widget))
    return tooltip


def uninstall_tooltip(widget: QWidget) -> None:
    """移除 widget 上挂接的 RichTooltip（隐藏、删除、清注册）。"""

    key = id(widget)
    tooltip = _installed.get(key)
    if tooltip is None:
        return
    try:
        if tooltip._pending_timer is not None:
            tooltip._pending_timer.stop()
        tooltip.hide()
    except Exception:
        pass
    tooltip.deleteLater()
    _installed.pop(key, None)
