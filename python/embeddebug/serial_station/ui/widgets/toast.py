"""Toast 气泡控件 — 单条非模态通知的渲染 + 入场/离场动画。

对齐 Linear/Slack/Vercel 的 toast 语言：右上角浮层卡片，从右滑入（SlideAnimation），
超时或手动关闭时淡出（FadeTransition），生命周期由 AnimationController 统一管理。

Batch 11 新建：填补通知子系统（notifications/）只有逻辑层（NotificationManager）而无
UI 渲染层的缺口，同时把三个仍为死代码的动画模块接入真实用户路径：
- SlideAnimation.slide_in  —— toast 从右滑入。
- FadeTransition.fade_out  —— toast 离场淡出。
- AnimationController      —— 管理同一条 toast 的入/离场动画生命周期。

设计要点：
- 自包含 QFrame 卡片：左侧色条（级别色）+ 标题 + 描述 + 关闭按钮。
- enter() 触发 slide_in 并接 AnimationController；schedule_dismiss(ms) 启动 QTimer，
  到点调 leave() 触发 fade_out，完成后从父布局移除。
- 级别色取 palette（INFO/SUCCESS/WARNING/ERROR），无硬编码。
- 只依赖 PyQt6 + theme.palette + animations + notifications.data，不访问 controller/transport。

约束：UI 控件，不持有 NotificationManager；由上层（如 status_actions）把 manager 的
notification_added/removed 信号接到 ToastWidget 的 enter/leave。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt, QTimer, pyqtSignal
from PyQt6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.notifications.data import NotificationData, NotificationLevel
from embeddebug.serial_station.ui.animations.controller import AnimationController
from embeddebug.serial_station.ui.animations.fade import FadeTransition
from embeddebug.serial_station.ui.animations.slide import SlideAnimation, SlideDirection
from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


# 级别 → 色条/强调色（取 palette 状态色，无硬编码）。
_LEVEL_COLORS: dict[NotificationLevel, str] = {
    NotificationLevel.INFO: P.INFO,
    NotificationLevel.SUCCESS: P.SUCCESS,
    NotificationLevel.WARNING: P.WARNING,
    NotificationLevel.ERROR: P.ERROR,
}

# 级别 → lucide 图标名（Batch 45: 从 Unicode 几何符号迁移到 SVG，对齐全应用图标系统）。
_LEVEL_ICON: dict[NotificationLevel, str] = {
    NotificationLevel.INFO: "info",
    NotificationLevel.SUCCESS: "check-circle",
    NotificationLevel.WARNING: "alert-triangle",
    NotificationLevel.ERROR: "x-circle",
}

_DEFAULT_TIMEOUT_MS = 3000
_DEFAULT_SLIDE_DISTANCE = 80


class ToastWidget(QFrame):
    """单条 toast 卡片，带入场滑入 + 离场淡出动画。

    用法（由上层通知容器驱动）::

        toast = ToastWidget(data, parent=container)
        toast.closed.connect(container.remove_toast)
        container.layout().addWidget(toast)
        toast.enter()
        toast.schedule_dismiss(data.timeout_ms)

    Args:
        data: 通知载荷（级别/标题/描述/超时）。
        slide_distance: 入场滑入距离（px）。
        parent: 父控件。
    """

    # 关闭信号：离场动画完成后发出，上层据此从容器移除。
    closed = pyqtSignal(object)

    def __init__(
        self,
        data: NotificationData,
        slide_distance: int = _DEFAULT_SLIDE_DISTANCE,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationToast")
        self._data = data
        self._slide_distance = max(20, int(slide_distance))
        self._anim_ctrl = AnimationController(self)
        self._dismiss_timer: QTimer | None = None
        self._leaving = False
        self._build_ui()

    # ── UI 构建 ─────────────────────────────────────────────────────
    def _build_ui(self) -> None:
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        # 左侧级别色条（4px 宽，整列填满高度）。
        self._accent = QFrame(self)
        self._accent.setObjectName("serialStationToastAccent")
        self._accent.setFixedWidth(4)
        self._accent.setStyleSheet(
            f"background-color: {_LEVEL_COLORS[self._data.level]}; border: none;"
        )
        layout.addWidget(self._accent)

        # 内容区：级别符号 + 标题/描述 + 关闭按钮。
        body = QWidget(self)
        body_layout = QHBoxLayout(body)
        body_layout.setContentsMargins(T.SPACING_INT_LG, T.SPACING_INT_MD, T.SPACING_INT_MD, T.SPACING_INT_MD)
        body_layout.setSpacing(T.SPACING_INT_LG)

        glyph = QLabel(body)
        glyph.setObjectName("serialStationToastGlyph")
        icon = button_icon(
            _LEVEL_ICON[self._data.level], color=_LEVEL_COLORS[self._data.level]
        )
        if icon is not None and not icon.isNull():
            glyph.setPixmap(icon.pixmap(18, 18))
        glyph.setFixedSize(18, 18)
        glyph.setAlignment(Qt.AlignmentFlag.AlignCenter)
        body_layout.addWidget(glyph)

        text_box = QVBoxLayout()
        text_box.setSpacing(2)
        self._title_label = QLabel(self._data.title, body)
        self._title_label.setObjectName("serialStationToastTitle")
        text_box.addWidget(self._title_label)
        if self._data.message:
            self._message_label = QLabel(self._data.message, body)
            self._message_label.setObjectName("serialStationToastMessage")
            self._message_label.setWordWrap(True)
            text_box.addWidget(self._message_label)
        body_layout.addLayout(text_box, 1)

        close_btn = QPushButton(body)
        close_btn.setObjectName("serialStationToastCloseButton")
        close_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        close_btn.setFlat(True)
        close_btn.setFixedSize(20, 20)
        close_icon = button_icon("x", color=P.TEXT_MUTED)
        if close_icon is not None and not close_icon.isNull():
            close_btn.setIcon(close_icon)
        close_btn.clicked.connect(self.leave)
        body_layout.addWidget(close_btn)

        layout.addWidget(body, 1)

    # ── 动画生命周期 ────────────────────────────────────────────────
    def enter(self) -> None:
        """入场：从右滑入到当前位置（激活 SlideAnimation 死代码）。"""

        anim = SlideAnimation.slide_in(
            self, direction=SlideDirection.RIGHT, distance=self._slide_distance
        )
        self._anim_ctrl.add(anim)
        anim.start()

    def schedule_dismiss(self, timeout_ms: int = _DEFAULT_TIMEOUT_MS) -> None:
        """启动自动消失定时器；timeout_ms<=0 表示需手动关闭。"""

        if timeout_ms <= 0:
            return
        self._dismiss_timer = QTimer(self)
        self._dismiss_timer.setSingleShot(True)
        self._dismiss_timer.setInterval(timeout_ms)
        self._dismiss_timer.timeout.connect(self.leave)
        self._dismiss_timer.start()

    def leave(self) -> None:
        """离场：淡出（激活 FadeTransition 死代码），完成后发 closed 信号。"""

        if self._leaving:
            return
        self._leaving = True
        if self._dismiss_timer is not None:
            self._dismiss_timer.stop()
            self._dismiss_timer = None
        fade = FadeTransition.fade_out(self)
        # fade_out 完成会 hide()；再发 closed 让容器移除。
        fade.finished.connect(self._emit_closed)
        self._anim_ctrl.add(fade)
        fade.start()

    def _emit_closed(self) -> None:
        self.closed.emit(self)

    # ── 访问 ────────────────────────────────────────────────────────
    @property
    def data(self) -> NotificationData:
        return self._data

    @property
    def is_leaving(self) -> bool:
        """是否处于离场中（避免重复 leave）。"""

        return self._leaving

    def stop_animations(self) -> None:
        """停止所有动画（容器销毁前清理）。"""

        self._anim_ctrl.stop_all()
        if self._dismiss_timer is not None:
            self._dismiss_timer.stop()
            self._dismiss_timer = None
