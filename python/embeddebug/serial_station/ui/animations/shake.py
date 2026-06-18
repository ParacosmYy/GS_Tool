"""抖动动画：错误反馈（输入框验证失败时左右抖动）。"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QPoint

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ShakeAnimation:
    """水平抖动动画工厂。"""

    @staticmethod
    def shake(widget, amplitude: int = 8, count: int = 3) -> QPropertyAnimation:
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
        return anim
