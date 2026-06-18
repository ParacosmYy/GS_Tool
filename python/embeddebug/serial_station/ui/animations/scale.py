"""缩放动画：按钮按压回弹、hover 放大。

点击按钮时缩小到 SCALE_PRESSED 然后弹回 1.0，
产生物理按压反馈感。使用 QGraphicsEffect 实现缩放。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QEasingCurve
from PyQt6.QtWidgets import QGraphicsScale, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ScaleAnimation:
    """控件缩放动画工厂。"""

    @staticmethod
    def press(widget: QWidget) -> QPropertyAnimation:
        """按压回弹：缩小到 0.92 再弹回 1.0。"""
        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        orig = widget.geometry()
        pressed = orig
        pressed.setWidth(int(orig.width() * AnimationTokens.SCALE_PRESSED))
        pressed.setHeight(int(orig.height() * AnimationTokens.SCALE_PRESSED))
        pressed.moveCenter(orig.center())
        anim.setStartValue(orig)
        anim.setKeyValueAt(0.5, pressed)
        anim.setEndValue(orig)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
        return anim

    @staticmethod
    def pop_in(widget: QWidget) -> QPropertyAnimation:
        """弹入：从 0.5 缩放到 1.0（面板出现效果）。"""
        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        orig = widget.geometry()
        small = orig
        small.setWidth(int(orig.width() * 0.5))
        small.setHeight(int(orig.height() * 0.5))
        small.moveCenter(orig.center())
        anim.setStartValue(small)
        anim.setEndValue(orig)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
        return anim

    @staticmethod
    def bounce(widget: QWidget) -> QPropertyAnimation:
        """弹跳：短暂放大到 1.1 再回 1.0（通知出现）。"""
        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        orig = widget.geometry()
        big = orig
        big.setWidth(int(orig.width() * 1.1))
        big.setHeight(int(orig.height() * 1.1))
        big.moveCenter(orig.center())
        anim.setStartValue(orig)
        anim.setKeyValueAt(0.4, big)
        anim.setEndValue(orig)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BOUNCE)
        return anim
