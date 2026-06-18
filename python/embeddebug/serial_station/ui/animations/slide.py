"""滑入滑出动画：从指定方向滑入或滑出。

时长引用 ``AnimationTokens``，入场用 ``DURATION_NORMAL``、离场用 ``DURATION_FAST``。
"""

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
    """方向性滑入/滑出动画工厂。

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
    def _offset(pos: QPoint, direction: "SlideDirection", distance: int) -> QPoint:
        if direction == SlideDirection.LEFT:
            return QPoint(pos.x() - distance, pos.y())
        if direction == SlideDirection.RIGHT:
            return QPoint(pos.x() + distance, pos.y())
        if direction == SlideDirection.UP:
            return QPoint(pos.x(), pos.y() - distance)
        return QPoint(pos.x(), pos.y() + distance)

    @staticmethod
    def slide_in(widget: QWidget, direction: SlideDirection = SlideDirection.LEFT,
                 distance: int = 60) -> QPropertyAnimation:
        """从指定方向滑入到当前位置。"""

        target_pos = widget.pos()
        start_pos = SlideAnimation._offset(target_pos, direction, distance)
        widget.move(start_pos)
        anim = QPropertyAnimation(widget, b"pos", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        anim.setStartValue(start_pos)
        anim.setEndValue(target_pos)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return SlideAnimation._track(anim)

    @staticmethod
    def slide_out(widget: QWidget, direction: SlideDirection = SlideDirection.RIGHT,
                  distance: int = 60) -> QPropertyAnimation:
        """从当前位置滑出到指定方向，完成后隐藏。"""

        start_pos = widget.pos()
        end_pos = SlideAnimation._offset(start_pos, direction, distance)
        anim = QPropertyAnimation(widget, b"pos", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        anim.setStartValue(start_pos)
        anim.setEndValue(end_pos)
        anim.setEasingCurve(AnimationTokens.EASE_IN)
        anim.finished.connect(widget.hide)
        return SlideAnimation._track(anim)
