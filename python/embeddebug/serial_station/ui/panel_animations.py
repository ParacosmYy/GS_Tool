"""面板过渡动画（QPropertyAnimation）。

为卡片/面板提供进入与切换过渡，对齐 05-ui-standard 铁律 18（面板切换必须有过渡动画，
禁止突然出现/消失）。提供可复用动画工厂：

- ``fade_in`` / ``fade_out``：QWidget 透明度淡入淡出（用 QGraphicsOpacityEffect）。
- ``slide_in``：QWidget 从下方滑入（pos 动画）。
- ``card_enter``：卡片进入组合动画（淡入 + 上滑），用于面板首次显示。
- ``stagger_fade``：多卡片错峰纯淡入（不 move 控件，对动态布局安全）。

设计要点：
- 时长与缓动**统一引用** ``animations.tokens.AnimationTokens``，消除旧的双轨制常量。
- 工厂返回 ``QPropertyAnimation``（持有 effect 引用防 GC，effect 以 widget 为 parent）。
- 不依赖 controller/core/protocol。

约束：本模块只依赖 PyQt6 + 标准库。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QPoint, QTimer
from PyQt6.QtWidgets import QGraphicsOpacityEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

# 兼容性别名：旧代码可能 import DURATION_* 常量；保留并指向 token。
DURATION_FAST = AnimationTokens.DURATION_FAST
DURATION_NORMAL = AnimationTokens.DURATION_NORMAL
DURATION_SLOW = AnimationTokens.DURATION_NORMAL  # 历史语义：卡片入场用 normal
SLIDE_PIXELS = 16


def fade_in(widget: QWidget, duration_ms: int = AnimationTokens.DURATION_NORMAL) -> QPropertyAnimation:
    """淡入动画：透明度 0 → 1。"""

    effect = QGraphicsOpacityEffect(widget)
    effect.setOpacity(0.0)
    widget.setGraphicsEffect(effect)
    anim = QPropertyAnimation(effect, b"opacity", widget)
    anim.setDuration(duration_ms)
    anim.setStartValue(0.0)
    anim.setEndValue(1.0)
    anim.setEasingCurve(AnimationTokens.EASE_OUT)
    return anim


def fade_out(widget: QWidget, duration_ms: int = AnimationTokens.DURATION_FAST) -> QPropertyAnimation:
    """淡出动画：透明度 1 → 0，完成后隐藏 widget。"""

    effect = QGraphicsOpacityEffect(widget)
    effect.setOpacity(1.0)
    widget.setGraphicsEffect(effect)
    anim = QPropertyAnimation(effect, b"opacity", widget)
    anim.setDuration(duration_ms)
    anim.setStartValue(1.0)
    anim.setEndValue(0.0)
    anim.setEasingCurve(AnimationTokens.EASE_OUT)
    anim.finished.connect(widget.hide)
    return anim


def slide_in(widget: QWidget, duration_ms: int = AnimationTokens.DURATION_NORMAL) -> QPropertyAnimation:
    """滑入动画：从原位下方 SLIDE_PIXELS 处上滑到原位。"""

    target_pos = widget.pos()
    start_pos = QPoint(target_pos.x(), target_pos.y() + SLIDE_PIXELS)
    widget.move(start_pos)
    anim = QPropertyAnimation(widget, b"pos", widget)
    anim.setDuration(duration_ms)
    anim.setStartValue(start_pos)
    anim.setEndValue(target_pos)
    anim.setEasingCurve(AnimationTokens.EASE_OUT)
    return anim


def card_enter(widget: QWidget) -> list[QPropertyAnimation]:
    """卡片进入组合动画：淡入 + 上滑，返回动画列表（调用方需 start）。"""

    return [fade_in(widget, AnimationTokens.DURATION_NORMAL),
            slide_in(widget, AnimationTokens.DURATION_NORMAL)]


def stagger_fade(cards: list[QWidget], delay_ms: int = 60) -> list[QPropertyAnimation]:
    """卡片错峰淡入（Batch 21）：纯透明度淡入，不移动控件，对动态布局安全。

    只调 fade_in（仅设 opacity，不 move），适合布局管理的卡片容器。
    每张卡片延迟 index*delay_ms 启动。

    历史：旧版 ``stagger``（card_enter 含 slide_in 会 move 控件，与布局定位冲突）
    已在 Batch 24 删除，统一用本函数。
    """

    animations: list[QPropertyAnimation] = []
    for index, card in enumerate(cards):
        anim = fade_in(card, AnimationTokens.DURATION_NORMAL)
        QTimer.singleShot(index * delay_ms, anim.start)
        animations.append(anim)
    return animations
