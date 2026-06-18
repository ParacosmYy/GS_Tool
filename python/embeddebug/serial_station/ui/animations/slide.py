"""滑入滑出动画：从指定方向滑入或滑出。"""

from __future__ import annotations

from enum import Enum

from PyQt6.QtCore import QPropertyAnimation, QPoint
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class SlideDirection(Enum):
    """滑入方向。"""
    LEFT = "left"
    RIGHT = "right"
    UP = "up"
    DOWN = "down"


class SlideAnimation:
    """方向性滑入/滑出动画工厂。"""

    @staticmethod
    def slide_in(widget: QWidget, direction: SlideDirection = SlideDirection.LEFT,
                 distance: int = 60) -> QPropertyAnimation:
        """从指定方向滑入到当前位置。"""
        target_pos = widget.pos()
        offset_map = {
            SlideDirection.LEFT: QPoint(target_pos.x() - distance, target_pos.y()),
            SlideDirection.RIGHT: QPoint(target_pos.x() + distance, target_pos.y()),
            SlideDirection.UP: QPoint(target_pos.x(), target_pos.y() - distance),
            SlideDirection.DOWN: QPoint(target_pos.x(), target_pos.y() + distance),
        }
        anim = QPropertyAnimation(widget, b"pos", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        anim.setStartValue(offset_map[direction])
        anim.setEndValue(target_pos)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return anim

    @staticmethod
    def slide_out(widget: QWidget, direction: SlideDirection = SlideDirection.RIGHT,
                  distance: int = 60) -> QPropertyAnimation:
        """从当前位置滑出到指定方向。"""
        start_pos = widget.pos()
        offset_map = {
            SlideDirection.LEFT: QPoint(start_pos.x() - distance, start_pos.y()),
            SlideDirection.RIGHT: QPoint(start_pos.x() + distance, start_pos.y()),
            SlideDirection.UP: QPoint(start_pos.x(), start_pos.y() - distance),
            SlideDirection.DOWN: QPoint(start_pos.x(), start_pos.y() + distance),
        }
        anim = QPropertyAnimation(widget, b"pos", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        anim.setStartValue(start_pos)
        anim.setEndValue(offset_map[direction])
        anim.setEasingCurve(AnimationTokens.EASE_IN)
        anim.finished.connect(widget.hide)
        return anim
