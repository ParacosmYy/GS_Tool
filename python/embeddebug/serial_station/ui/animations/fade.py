"""淡入淡出过渡：页面/面板切换。"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QSequentialAnimationGroup, QTimer, pyqtSignal
from PyQt6.QtWidgets import QGraphicsOpacityEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class FadeTransition:
    """淡入淡出动画工厂。"""

    @staticmethod
    def fade_in(widget: QWidget, duration: int = AnimationTokens.DURATION_NORMAL) -> QPropertyAnimation:
        """淡入：透明度 0 → 1。"""
        effect = QGraphicsOpacityEffect(widget)
        effect.setOpacity(0.0)
        widget.setGraphicsEffect(effect)
        widget.show()
        anim = QPropertyAnimation(effect, b"opacity", widget)
        anim.setDuration(duration)
        anim.setStartValue(0.0)
        anim.setEndValue(1.0)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return anim

    @staticmethod
    def fade_out(widget: QWidget, duration: int = AnimationTokens.DURATION_FAST) -> QPropertyAnimation:
        """淡出：透明度 1 → 0，完成后隐藏。"""
        effect = QGraphicsOpacityEffect(widget)
        effect.setOpacity(1.0)
        widget.setGraphicsEffect(effect)
        anim = QPropertyAnimation(effect, b"opacity", widget)
        anim.setDuration(duration)
        anim.setStartValue(1.0)
        anim.setEndValue(0.0)
        anim.setEasingCurve(AnimationTokens.EASE_IN)
        anim.finished.connect(widget.hide)
        return anim

    @staticmethod
    def cross_fade(out_widget: QWidget, in_widget: QWidget,
                   duration: int = AnimationTokens.DURATION_NORMAL) -> QSequentialAnimationGroup:
        """交叉淡出淡入：先淡出旧面板，再淡入新面板。"""
        group = QSequentialAnimationGroup(out_widget)
        fade_out = FadeTransition.fade_out(out_widget, duration // 2)
        group.addAnimation(fade_out)
        # 延迟显示新面板
        QTimer.singleShot(duration // 2, in_widget.show)
        fade_in = FadeTransition.fade_in(in_widget, duration // 2)
        group.addAnimation(fade_in)
        return group
