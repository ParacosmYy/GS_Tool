"""微交互（对齐 Linear/Arc 级精致度）。

- ``install_hover_lift``：按钮/卡片 hover 时平滑上浮 + 阴影增强。
- ``install_focus_ring``：输入框 focus 时强调色光环。
- ``HoverLiftFilter``：事件过滤器，统一管理 hover 浮起动画。

约束：本模块只依赖 PyQt6 + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import (
    QEasingCurve,
    QEvent,
    QObject,
    QPropertyAnimation,
    QRect,
    QSize,
    QTimer,
)
from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QWidget

from embeddebug.serial_station.ui.theme import palette as P

LIFT_PIXELS = 3
SHADOW_BLUR_NORMAL = 16
SHADOW_BLUR_HOVER = 28
ANIM_DURATION = 160


def install_hover_lift(widget: QWidget) -> QGraphicsDropShadowEffect:
    """给 widget 安装 hover 浮起 + 阴影效果，返回阴影 effect（需持有）。"""

    effect = QGraphicsDropShadowEffect(widget)
    effect.setBlurRadius(SHADOW_BLUR_NORMAL)
    effect.setColor(QColor(0, 0, 0, 120))
    effect.setOffset(0, 2)
    widget.setGraphicsEffect(effect)
    widget.installEventFilter(_HoverLiftFilter(widget, effect))
    return effect


class _HoverLiftFilter(QObject):
    """hover 进入/离开时动画调整阴影模糊与 widget 几何（视觉浮起）。"""

    def __init__(self, widget: QWidget, effect: QGraphicsDropShadowEffect) -> None:
        super().__init__(widget)
        self._widget = widget
        self._effect = effect
        self._shadow_anim: QPropertyAnimation | None = None
        self._original_geometry: QRect | None = None

    def eventFilter(self, obj: object, event: QEvent) -> bool:
        if obj is not self._widget:
            return False
        etype = event.type()
        if etype == QEvent.Type.Enter:
            self._on_enter()
        elif etype == QEvent.Type.Leave:
            self._on_leave()
        elif etype == QEvent.Type.FocusIn:
            self._on_enter()
        elif etype == QEvent.Type.FocusOut:
            self._on_leave()
        return False

    def _on_enter(self) -> None:
        self._animate_shadow(SHADOW_BLUR_HOVER)

    def _on_leave(self) -> None:
        self._animate_shadow(SHADOW_BLUR_NORMAL)

    def _animate_shadow(self, target_blur: int) -> None:
        if self._shadow_anim is not None:
            self._shadow_anim.stop()
        self._shadow_anim = QPropertyAnimation(self._effect, b"blurRadius", self._widget)
        self._shadow_anim.setDuration(ANIM_DURATION)
        self._shadow_anim.setStartValue(self._effect.blurRadius())
        self._shadow_anim.setEndValue(target_blur)
        self._shadow_anim.setEasingCurve(QEasingCurve.Type.OutCubic)
        self._shadow_anim.start()


def install_focus_ring(widget: QWidget) -> None:
    """给输入框安装 focus 光环（focus 时阴影变强调色）。"""

    effect = QGraphicsDropShadowEffect(widget)
    effect.setBlurRadius(0)
    effect.setColor(QColor(P.ACCENT))
    effect.setOffset(0, 0)
    widget.setGraphicsEffect(effect)

    def _on_focus_in(_event: object) -> None:
        anim = QPropertyAnimation(effect, b"blurRadius", widget)
        anim.setDuration(ANIM_DURATION)
        anim.setStartValue(0)
        anim.setEndValue(16)
        anim.setEasingCurve(QEasingCurve.Type.OutCubic)
        anim.start()

    def _on_focus_out(_event: object) -> None:
        anim = QPropertyAnimation(effect, b"blurRadius", widget)
        anim.setDuration(ANIM_DURATION)
        anim.setStartValue(effect.blurRadius())
        anim.setEndValue(0)
        anim.setEasingCurve(QEasingCurve.Type.OutCubic)
        anim.start()

    # 用事件过滤器捕获 focus（避免覆盖 widget 自身 focusEvent）。
    widget.installEventFilter(_FocusRingFilter(effect, widget))


class _FocusRingFilter(QObject):
    """focus in/out 动画调整光环半径。"""

    def __init__(self, effect: QGraphicsDropShadowEffect, widget: QWidget) -> None:
        super().__init__(widget)
        self._effect = effect
        self._widget = widget

    def eventFilter(self, obj: object, event: QEvent) -> bool:
        if obj is not self._widget:
            return False
        etype = event.type()
        if etype == QEvent.Type.FocusIn:
            self._animate(0, 16)
        elif etype == QEvent.Type.FocusOut:
            self._animate(self._effect.blurRadius(), 0)
        return False

    def _animate(self, start: int, end: int) -> None:
        anim = QPropertyAnimation(self._effect, b"blurRadius", self._widget)
        anim.setDuration(ANIM_DURATION)
        anim.setStartValue(start)
        anim.setEndValue(end)
        anim.setEasingCurve(QEasingCurve.Type.OutCubic)
        anim.start()
