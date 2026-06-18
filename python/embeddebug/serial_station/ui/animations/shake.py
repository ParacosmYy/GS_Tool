"""抖动动画：错误反馈（输入框验证失败时左右抖动）。

振幅与次数引用 ``AnimationTokens``，全应用一致。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QPoint
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ShakeAnimation:
    """水平抖动动画工厂。

    所有方法返回的动画自动注册到类级活跃列表防 GC，完成自动移除。
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
    def shake(widget: QWidget,
              amplitude: int = AnimationTokens.SHAKE_AMPLITUDE,
              count: int = AnimationTokens.SHAKE_COUNT) -> QPropertyAnimation:
        """水平抖动：左右往返 count 次后回到原位。

        Args:
            widget: 要抖动的控件。
            amplitude: 每次偏移的像素数。
            count: 抖动往返次数。
        """

        orig_pos = widget.pos()
        anim = QPropertyAnimation(widget, b"pos", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        anim.setStartValue(orig_pos)
        for i in range(count):
            sign = 1 if i % 2 == 0 else -1
            t = (i + 1) / (count * 2 + 1)
            anim.setKeyValueAt(t, QPoint(orig_pos.x() + sign * amplitude, orig_pos.y()))
        anim.setEndValue(orig_pos)
        anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        return ShakeAnimation._track(anim)
