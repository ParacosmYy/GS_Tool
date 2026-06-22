"""滑入式侧边抽屉面板。

左 / 右 / 上 / 下 四向滑入，带半透明遮罩层。点击遮罩区域关闭。用于设置面板、
筛选面板、详情面板等需要临时聚焦的侧边内容。

设计要点：
- 抽屉本体是覆盖父区域的遮罩 QWidget，paintEvent 自绘半透明 overlay（``BG_OVERLAY``
  基色，alpha 随 ``_overlay_alpha`` 调制）。内部 ``_panel`` 子控件承载内容
  （``set_content`` 替换），由 ``open`` / ``close`` 通过 ``QPropertyAnimation`` 在
  ``geometry`` 上滑动入场 / 离场。
- 动画 ``finished`` 连接**绑定方法**而非闭包——PyQt 在 receiver 被 C++ 析构时自动
  断开绑定方法连接，避免访问冲突（与 ``info_banner._on_anim_finished`` 同一模式）。
- ``close`` 完成后用 ``QTimer.singleShot(0, self.deleteLater)`` 推迟自销毁。
- 颜色全部走 palette 常量（``BG_OVERLAY`` / ``BG_PANEL`` / ``ACCENT``）。
"""

from __future__ import annotations

from PyQt6.QtCore import (
    QEvent,
    QPointF,
    QPropertyAnimation,
    QRect,
    QSize,
    Qt,
    QTimer,
    QVariantAnimation,
    pyqtSignal,
)
from PyQt6.QtGui import QColor, QMouseEvent, QPaintEvent, QPainter
from PyQt6.QtWidgets import QVBoxLayout, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


# 抽屉面板默认尺寸：左右边 280px 宽，上下边 200px 高。
_DEFAULT_HORIZONTAL_EXTENT = 280
_DEFAULT_VERTICAL_EXTENT = 200


class Drawer(QWidget):
    """滑入式侧边抽屉面板控件。

    四向滑入 + 半透明遮罩，点击遮罩关闭。承载任意内容控件（``set_content``）。

    信号：
        opened: 抽屉完全展开后发出（滑入动画结束）。
        closed: 抽屉完全收起后发出（滑出动画结束，随后自销毁）。
    """

    opened = pyqtSignal()
    closed = pyqtSignal()

    def __init__(
        self,
        side: Qt.Edge = Qt.Edge.LeftEdge,
        parent: QWidget | None = None,
    ) -> None:
        """初始化抽屉。

        参数：
            side: 滑入方向（Left / Right / Top / Bottom Edge）。
            parent: 父控件；为 None 时作为顶层窗口显示（遮罩全屏）。
        """

        super().__init__(parent)
        self.setObjectName("serialStationDrawer")
        self.setMouseTracking(True)
        # 透明背景——paintEvent 自绘 overlay，避免默认 QWidget 底破坏遮罩色相。
        self.setAttribute(Qt.WidgetAttribute.WA_StyledBackground, False)

        self._side: Qt.Edge = side
        self._is_open: bool = False
        # overlay 透明度 0.0~1.0，paintEvent 据此调制 BG_OVERLAY 的 alpha。
        self._overlay_alpha: float = 0.0

        # 内容容器（承载 set_content 的控件，参与 geometry 滑动动画）。
        self._panel = QWidget(self)
        self._panel.setObjectName("serialStationDrawerPanel")
        self._panel_layout = QVBoxLayout(self._panel)
        self._panel_layout.setContentsMargins(0, 0, 0, 0)
        self._panel_layout.setSpacing(0)
        self._content: QWidget | None = None

        # 活动动画引用（持有以防 GC 中断动画）。
        self._slide: QPropertyAnimation | None = None
        self._fade: QVariantAnimation | None = None

    # ── 公共 API ─────────────────────────────────────────────────────
    def set_content(self, widget: QWidget) -> None:
        """替换抽屉内容控件。旧控件被 reparent + deleteLater。"""

        if self._content is not None:
            self._panel_layout.removeWidget(self._content)
            self._content.setParent(None)
            self._content.deleteLater()
        self._content = widget
        widget.setParent(self._panel)
        self._panel_layout.addWidget(widget)

    def open(self) -> None:
        """显示遮罩 + 滑入面板；动画结束后发射 ``opened``。"""

        if self._is_open:
            return
        self._is_open = True
        self.show()
        self.raise_()

        target = self._panel_target_rect()
        offscreen = self._panel_offscreen_rect()
        self._panel.setGeometry(offscreen)
        self._panel.show()
        self._panel.raise_()

        # overlay 从透明淡入。
        self._overlay_alpha = 0.0
        self.update()
        self._run_fade(0.0, 1.0)

        # 面板 geometry 滑入（finished 连绑定方法，避免访问冲突）。
        # Batch 52: 消费预留 token DURATION_DRAWER（抽屉专用滑出时长 300ms，
        # 比 DURATION_NORMAL 更长，匹配 Material 抽屉观感）。
        if self._slide is not None:
            self._slide.stop()
        slide = QPropertyAnimation(self._panel, b"geometry", self)
        slide.setDuration(AnimationTokens.DURATION_DRAWER)
        slide.setStartValue(offscreen)
        slide.setEndValue(target)
        slide.setEasingCurve(AnimationTokens.EASE_OUT)
        slide.finished.connect(self._on_open_finished)
        self._slide = slide
        slide.start()

    def close(self) -> None:
        """滑出面板 + 淡出遮罩；动画结束后发射 ``closed`` 并自销毁。

        ``_is_open`` 同步置 False，``closed`` 信号在滑出动画完成后异步发射，
        随后 ``QTimer.singleShot(0, self.deleteLater)`` 推迟自销毁。

        生命周期安全（关键）：``finished`` 连接到绑定方法 ``_on_close_finished``；
        ``QTimer.singleShot(0, ...)`` 把 ``deleteLater`` 推到 ``finished`` 分发路径
        之外，避免在信号回调内同步触发二次删除（与 ``info_banner`` 同一模式）。
        """

        if not self._is_open:
            return
        self._is_open = False

        offscreen = self._panel_offscreen_rect()
        current = QRect(self._panel.geometry())

        # overlay 淡出。
        self._run_fade(self._overlay_alpha, 0.0)

        if self._slide is not None:
            self._slide.stop()
        slide = QPropertyAnimation(self._panel, b"geometry", self)
        slide.setDuration(AnimationTokens.DURATION_NORMAL)
        slide.setStartValue(current)
        slide.setEndValue(offscreen)
        slide.setEasingCurve(AnimationTokens.EASE_IN)
        slide.finished.connect(self._on_close_finished)
        self._slide = slide
        slide.start()

    def is_open(self) -> bool:
        """返回抽屉当前是否处于展开状态。"""

        return self._is_open

    # ── 几何计算 ─────────────────────────────────────────────────────
    def _is_horizontal(self) -> bool:
        """左右边为水平抽屉（宽度固定），上下边为垂直（高度固定）。"""

        return self._side in (Qt.Edge.LeftEdge, Qt.Edge.RightEdge)

    def _panel_extent(self) -> int:
        """返回抽屉面板的固定尺寸（水平=宽，垂直=高）。"""

        return (
            _DEFAULT_HORIZONTAL_EXTENT
            if self._is_horizontal()
            else _DEFAULT_VERTICAL_EXTENT
        )

    def _panel_target_rect(self) -> QRect:
        """面板展开后的目标矩形（贴对应边）。"""

        ext = self._panel_extent()
        if self._side == Qt.Edge.LeftEdge:
            return QRect(0, 0, ext, self.height())
        if self._side == Qt.Edge.RightEdge:
            return QRect(self.width() - ext, 0, ext, self.height())
        if self._side == Qt.Edge.TopEdge:
            return QRect(0, 0, self.width(), ext)
        return QRect(0, self.height() - ext, self.width(), ext)

    def _panel_offscreen_rect(self) -> QRect:
        """面板离屏矩形（滑入起点 / 滑出终点）。"""

        ext = self._panel_extent()
        if self._side == Qt.Edge.LeftEdge:
            return QRect(-ext, 0, ext, self.height())
        if self._side == Qt.Edge.RightEdge:
            return QRect(self.width(), 0, ext, self.height())
        if self._side == Qt.Edge.TopEdge:
            return QRect(0, -ext, self.width(), ext)
        return QRect(0, self.height(), self.width(), ext)

    # ── 尺寸 ─────────────────────────────────────────────────────────
    def sizeHint(self) -> QSize:
        """建议尺寸：基于默认水平 / 垂直 extent，非零。"""

        return QSize(_DEFAULT_HORIZONTAL_EXTENT, _DEFAULT_VERTICAL_EXTENT)

    # ── 绘制 ─────────────────────────────────────────────────────────
    def paintEvent(self, event: QPaintEvent) -> None:
        """绘制半透明遮罩 + 面板底色 + 贴边 accent 高亮线。"""

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, False)

        # 1. 半透明 overlay（alpha 随 _overlay_alpha 调制 BG_OVERLAY 基色）。
        overlay = QColor(P.BG_OVERLAY)
        overlay.setAlpha(int(self._overlay_alpha * overlay.alpha()))
        painter.fillRect(self.rect(), overlay)

        # 2. 面板底色（绘制在 _panel.geometry() 区域，面板本身透明，内容控件叠在上层）。
        panel_rect = self._panel.geometry()
        if not panel_rect.isNull() and panel_rect.intersects(self.rect()):
            painter.fillRect(panel_rect, QColor(P.BG_PANEL))
            # accent 高亮线（贴边一侧），强调抽屉朝向。
            painter.setPen(QColor(P.ACCENT))
            if self._side == Qt.Edge.LeftEdge:
                painter.drawLine(panel_rect.topRight(), panel_rect.bottomRight())
            elif self._side == Qt.Edge.RightEdge:
                painter.drawLine(panel_rect.topLeft(), panel_rect.bottomLeft())
            elif self._side == Qt.Edge.TopEdge:
                painter.drawLine(panel_rect.bottomLeft(), panel_rect.bottomRight())
            else:
                painter.drawLine(panel_rect.topLeft(), panel_rect.topRight())

    # ── 鼠标交互 ─────────────────────────────────────────────────────
    def mousePressEvent(self, event: QMouseEvent) -> None:
        """点击遮罩区域（面板外）→ 关闭抽屉。"""

        if event.button() == Qt.MouseButton.LeftButton:
            pos = QPointF(event.position())
            if not self._panel.geometry().contains(pos.toPoint()):
                self.close()
        event.accept()

    # ── overlay 淡入淡出 ─────────────────────────────────────────────
    def _run_fade(self, frm: float, to: float) -> None:
        """运行 overlay alpha 过渡动画（QVariantAnimation，bound method 回调）。"""

        if self._fade is not None:
            self._fade.stop()
        fade = QVariantAnimation(self)
        fade.setStartValue(frm)
        fade.setEndValue(to)
        fade.setDuration(AnimationTokens.DURATION_NORMAL)
        fade.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        fade.valueChanged.connect(self._on_fade_value)
        self._fade = fade
        fade.start()

    def _on_fade_value(self, value) -> None:
        """overlay alpha 更新 → 触发重绘。绑定方法，self 删除时自动断开。"""

        try:
            self._overlay_alpha = float(value)
            self.update()
        except RuntimeError:
            # wrapped C/C++ object has been deleted.
            pass

    # ── 动画完成回调（绑定方法，避免访问冲突） ───────────────────────
    def _on_open_finished(self) -> None:
        """open 滑入动画完成：清理引用，发射 ``opened``。"""

        try:
            self._slide = None
            self.opened.emit()
        except RuntimeError:
            pass

    def _on_close_finished(self) -> None:
        """close 滑出动画完成：隐藏、发射 ``closed``、推迟自销毁。"""

        try:
            self._slide = None
            self.hide()
            self.closed.emit()
            QTimer.singleShot(0, self.deleteLater)
        except RuntimeError:
            # wrapped C/C++ object has been deleted.
            pass
