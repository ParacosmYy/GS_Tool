"""NavRail 滑动指示器 — 激活项指示条平滑滑动到当前选中按钮。

旧实现用 QSS border-left: 3px solid 做激活态指示条，切换模式时指示条在旧按钮处
瞬间消失、新按钮处瞬间出现，违反 05-ui-standard 铁律 18（禁止突然出现/消失）。

本模块提供独立指示条 widget，叠加在 NavRail 上：初次进入淡入到第 0 个按钮位置；
切换激活项时用 QPropertyAnimation 滑动到新按钮位置（对标 Linear / VS Code
Activity Bar / Material 3 NavigationRail）。reduced motion 开启时折叠为瞬时跳转。

约束：只依赖 PyQt6 + theme.palette + animations；不访问 controller/transport。
"""

from __future__ import annotations

import logging

from PyQt6.QtCore import QEasingCurve, QPropertyAnimation, QRect, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QPainter
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.reduced_motion import ReducedMotionState
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P

_log = logging.getLogger(__name__)

_INDICATOR_WIDTH_PX = 3
_INDICATOR_HEIGHT_RATIO = 0.6
_INDICATOR_LEFT_PADDING_PX = 0


class NavIndicator(QWidget):
    """NavRail 激活项滑动指示条。"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("serialStationNavIndicator")
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setVisible(False)
        self._target_rect = QRect(0, 0, _INDICATOR_WIDTH_PX, 0)
        self._opacity = 0.0
        self._slide_anim = None
        self._fade_anim = None

    def paintEvent(self, event):
        if self._opacity <= 0.0:
            return
        painter = QPainter(self)
        try:
            painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
            color = QColor(P.ACCENT)
            color.setAlphaF(self._opacity)
            painter.setBrush(color)
            painter.setPen(Qt.PenStyle.NoPen)
            rect = self.rect().adjusted(0, 0, -1, -1)
            radius = _INDICATOR_WIDTH_PX / 2
            painter.drawRoundedRect(rect, radius, radius)
        finally:
            painter.end()

    @staticmethod
    def _indicator_rect_for(button_rect):
        indicator_h = max(1, int(button_rect.height() * _INDICATOR_HEIGHT_RATIO))
        indicator_y = button_rect.y() + (button_rect.height() - indicator_h) // 2
        return QRect(_INDICATOR_LEFT_PADDING_PX, indicator_y,
                     _INDICATOR_WIDTH_PX, indicator_h)

    def move_to(self, button_rect, animate=True):
        target = self._indicator_rect_for(button_rect)
        self._target_rect = target

        if self._opacity <= 0.0:
            # 首次显示（opacity 仍为 0）：定位 + 淡入。
            self.setGeometry(target)
            self._fade_in()
            return

        if not animate or ReducedMotionState.current().enabled:
            self.setGeometry(target)
            self.update()
            return

        if self._slide_anim is not None:
            self._slide_anim.stop()
        self._slide_anim = QPropertyAnimation(self, b"geometry", self)
        self._slide_anim.setDuration(
            ReducedMotionState.current().apply_duration(AnimationTokens.DURATION_NORMAL)
        )
        self._slide_anim.setStartValue(self.geometry())
        self._slide_anim.setEndValue(target)
        self._slide_anim.setEasingCurve(
            ReducedMotionState.current().apply_easing(QEasingCurve.Type.OutQuart)
        )
        self._slide_anim.start()

    def _fade_in(self):
        self._opacity = 0.0
        self.setVisible(True)
        self.update()
        if self._fade_anim is not None:
            self._fade_anim.stop()
        self._fade_anim = QPropertyAnimation(self, b"indicatorOpacity", self)
        self._fade_anim.setDuration(
            ReducedMotionState.current().apply_duration(AnimationTokens.DURATION_FAST)
        )
        self._fade_anim.setStartValue(0.0)
        self._fade_anim.setEndValue(1.0)
        self._fade_anim.setEasingCurve(
            ReducedMotionState.current().apply_easing(AnimationTokens.EASE_OUT)
        )
        self._fade_anim.start()

    @pyqtProperty(float)
    def indicatorOpacity(self):
        return self._opacity

    @indicatorOpacity.setter
    def indicatorOpacity(self, value):
        self._opacity = max(0.0, min(1.0, float(value)))
        self.update()

    def stop_animations(self):
        if self._slide_anim is not None:
            try:
                self._slide_anim.stop()
            except Exception:
                _log.warning("nav indicator slide stop failed", exc_info=True)
            self._slide_anim = None
        if self._fade_anim is not None:
            try:
                self._fade_anim.stop()
            except Exception:
                _log.warning("nav indicator fade stop failed", exc_info=True)
            self._fade_anim = None
