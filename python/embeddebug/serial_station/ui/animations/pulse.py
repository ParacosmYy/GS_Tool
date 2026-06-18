"""脉冲呼吸动画：状态指示灯呼吸效果。"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QTimer, pyqtSignal
from PyQt6.QtWidgets import QGraphicsOpacityEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class PulseAnimation:
    """脉冲呼吸动画工厂。"""

    @staticmethod
    def pulse(widget: QWidget, min_opacity: float = 0.4, max_opacity: float = 1.0,
              duration: int = AnimationTokens.DURATION_SLOWER) -> QPropertyAnimation:
        """单次脉冲：透明度从 max → min → max。"""
        effect = QGraphicsOpacityEffect(widget)
        effect.setOpacity(max_opacity)
        widget.setGraphicsEffect(effect)
        anim = QPropertyAnimation(effect, b"opacity", widget)
        anim.setDuration(duration)
        anim.setStartValue(max_opacity)
        anim.setKeyValueAt(0.5, min_opacity)
        anim.setEndValue(max_opacity)
        anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        return anim

    @staticmethod
    def breathing(widget: QWidget, min_opacity: float = 0.3, max_opacity: float = 1.0,
                  interval_ms: int = 1200) -> QPropertyAnimation:
        """持续呼吸循环：不停脉冲直到停止。

        返回 QPropertyAnimation，设置 loopCount=-1 实现无限循环。
        """
        anim = PulseAnimation.pulse(widget, min_opacity, max_opacity, interval_ms)
        anim.setLoopCount(-1)
        return anim
