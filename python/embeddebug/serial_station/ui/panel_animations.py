"""面板过渡动画（QPropertyAnimation）。

为卡片/面板提供进入与切换过渡，对齐 05-ui-standard 铁律 18（面板切换必须有过渡动画，
禁止突然出现/消失）。提供三种可复用动画工厂：

- ``fade_in`` / ``fade_out``：QWidget 透明度淡入淡出（用 QGraphicsOpacityEffect）。
- ``slide_in``：QWidget 从下方滑入（pos 动画）。
- ``card_enter``：卡片进入组合动画（淡入 + 上滑），用于面板首次显示。

设计要点：
- 动画时长遵循 VOFA+ 观感基线（180~260ms），缓动用 OutCubic。
- 工厂返回 ``(animation, effect)``，调用方需持有 effect 防止 GC。
- 不依赖 controller/core/protocol。

约束：本模块只依赖 PyQt6 + 标准库。
"""

from __future__ import annotations

from PyQt6.QtCore import QEasingCurve, QPropertyAnimation, QPoint, QRectF, QTimer
from PyQt6.QtWidgets import QGraphicsBlurEffect, QGraphicsOpacityEffect, QWidget

DURATION_FAST = 160
DURATION_NORMAL = 220
DURATION_SLOW = 300
SLIDE_PIXELS = 16


def fade_in(widget: QWidget, duration_ms: int = DURATION_NORMAL) -> QPropertyAnimation:
    """淡入动画：透明度 0 → 1。"""

    effect = QGraphicsOpacityEffect(widget)
    effect.setOpacity(0.0)
    widget.setGraphicsEffect(effect)
    anim = QPropertyAnimation(effect, b"opacity", widget)
    anim.setDuration(duration_ms)
    anim.setStartValue(0.0)
    anim.setEndValue(1.0)
    anim.setEasingCurve(QEasingCurve.Type.OutCubic)
    return anim


def fade_out(widget: QWidget, duration_ms: int = DURATION_NORMAL) -> QPropertyAnimation:
    """淡出动画：透明度 1 → 0，完成后隐藏 widget。"""

    effect = QGraphicsOpacityEffect(widget)
    effect.setOpacity(1.0)
    widget.setGraphicsEffect(effect)
    anim = QPropertyAnimation(effect, b"opacity", widget)
    anim.setDuration(duration_ms)
    anim.setStartValue(1.0)
    anim.setEndValue(0.0)
    anim.setEasingCurve(QEasingCurve.Type.OutCubic)
    anim.finished.connect(widget.hide)
    return anim


def slide_in(widget: QWidget, duration_ms: int = DURATION_NORMAL) -> QPropertyAnimation:
    """滑入动画：从原位下方 SLIDE_PIXELS 处上滑到原位。"""

    target_pos = widget.pos()
    start_pos = QPoint(target_pos.x(), target_pos.y() + SLIDE_PIXELS)
    widget.move(start_pos)
    anim = QPropertyAnimation(widget, b"pos", widget)
    anim.setDuration(duration_ms)
    anim.setStartValue(start_pos)
    anim.setEndValue(target_pos)
    anim.setEasingCurve(QEasingCurve.Type.OutCubic)
    return anim


def card_enter(widget: QWidget) -> list[QPropertyAnimation]:
    """卡片进入组合动画：淡入 + 上滑，返回动画列表（调用方需 start）。"""

    return [fade_in(widget, DURATION_NORMAL), slide_in(widget, DURATION_NORMAL)]


def stagger(cards: list[QWidget], delay_ms: int = 60) -> list[QPropertyAnimation]:
    """卡片错峰进入：每张卡片延迟 delay_ms 启动，返回全部动画。"""

    animations: list[QPropertyAnimation] = []
    for index, card in enumerate(cards):
        anims = card_enter(card)
        for anim in anims:
            QTimer.singleShot(index * delay_ms, anim.start)
            animations.append(anim)
    return animations
