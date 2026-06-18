"""脉冲呼吸动画：状态指示灯呼吸效果。

提供单次脉冲与无限循环呼吸。基于 ``QGraphicsOpacityEffect``。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation
from PyQt6.QtWidgets import QGraphicsOpacityEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class PulseAnimation:
    """脉冲呼吸动画工厂。

    所有方法返回的动画自动注册到类级活跃列表防 GC，完成自动移除。
    呼吸循环（loopCount=-1）的动画需要调用方在不再需要时显式 ``stop()``
    并从 ``_active`` 移除（通过 ``stop_looping`` 辅助方法）。
    """

    _active: list = []

    @classmethod
    def _track(cls, anim: QPropertyAnimation) -> QPropertyAnimation:
        cls._active.append(anim)
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QPropertyAnimation) -> None:
        try:
            cls._active.remove(anim)
        except ValueError:
            pass

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
        return PulseAnimation._track(anim)

    @staticmethod
    def breathing(widget: QWidget, min_opacity: float = 0.3, max_opacity: float = 1.0,
                  interval_ms: int = 1200) -> QPropertyAnimation:
        """持续呼吸循环：不停脉冲直到停止。

        返回 ``loopCount=-1`` 的动画。调用方需持有引用；停止时调用
        ``PulseAnimation.stop_looping(widget)`` 或自行 ``anim.stop()``。

        Args:
            widget: 目标控件。
            min_opacity: 最小透明度（呼吸谷值）。
            max_opacity: 最大透明度（呼吸峰值）。
            interval_ms: 单次脉冲周期（毫秒）。
        """

        anim = PulseAnimation.pulse(widget, min_opacity, max_opacity, interval_ms)
        anim.setLoopCount(-1)
        # 无限循环动画 finished 永不触发，_track 注册的自动清理 lambda 不会执行，
        # 动画会一直留在 _active 列表。调用方停止时用 stop_looping(widget) 清理。
        return anim

    @staticmethod
    def stop_looping(widget: QWidget) -> None:
        """停止 widget 上所有正在进行的呼吸/脉冲动画。

        遍历活跃列表，停止以该 widget 为 parent 的动画。
        """

        for anim in list(PulseAnimation._active):
            if anim.parent() is widget:
                anim.stop()
                PulseAnimation._discard(anim)
