"""按钮 ripple 水波纹反馈（Material Design 风格）。

Batch 7-5 新建。诊断报告剩余项『按钮无 ripple』。

实现：``RippleButton`` 是 QPushButton 子类，覆盖 paintEvent，在父类绘制后叠加
一个从点击位置扩散的半透明圆形涟漪。用 ``QPropertyAnimation`` 驱动
``ripple_progress``（0→1），paintEvent 按 progress 计算 radius 与 alpha。

为什么不用 overlay widget：QWidget 同一时刻只能有一个 graphicsEffect（与
hover_lift 冲突），且 overlay 会拦截鼠标事件。自绘 overlay 在 paintEvent 最稳。

设计要点：
- ripple 从鼠标按下位置扩散，半径从 0 到覆盖按钮最远角。
- alpha 从 80 衰减到 0（淡出），颜色用 ACCENT 半透明。
- 时长 DURATION_NORMAL（240ms）OutCubic，符合 Material ripple 节奏。
- 可通过 ``set_ripple(False)`` 关闭（密集按钮场景）。

约束：只依赖 PyQt6 + theme + animations。
"""

from __future__ import annotations

import math

from PyQt6.QtCore import QPointF, QPropertyAnimation, QRectF, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QPainter, QRadialGradient
from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


class RippleButton(QPushButton):
    """带 Material ripple 水波纹反馈的按钮。

    点击时从按下位置扩散半透明 accent 圆形涟漪，淡出消失。
    可叠加 ScaleAnimation.press（按压回弹）与 hover_lift 获得完整微交互。
    """

    def __init__(self, text: str = "", parent=None) -> None:
        super().__init__(text, parent)
        self.setObjectName("serialStationRippleButton")
        self._ripple_enabled = True
        self._ripple_progress = 0.0
        self._ripple_center: QPointF | None = None
        self._ripple_anim: QPropertyAnimation | None = None

    def set_ripple(self, enabled: bool) -> None:
        """启用/禁用 ripple（密集按钮可关闭）。"""

        self._ripple_enabled = enabled

    def mousePressEvent(self, event) -> None:
        """捕获点击位置启动 ripple，再交父类处理。"""

        if self._ripple_enabled and event.button() == Qt.MouseButton.LeftButton:
            self._start_ripple(QPointF(event.position()))
        super().mousePressEvent(event)

    def _start_ripple(self, center: QPointF) -> None:
        """从 center 位置启动一次 ripple 扩散动画。"""

        self._ripple_center = center
        self._ripple_progress = 0.0
        if self._ripple_anim is not None:
            self._ripple_anim.stop()
        self._ripple_anim = QPropertyAnimation(self, b"ripple_progress", self)
        self._ripple_anim.setDuration(AnimationTokens.DURATION_NORMAL)
        self._ripple_anim.setStartValue(0.0)
        self._ripple_anim.setEndValue(1.0)
        self._ripple_anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._ripple_anim.start()

    def _get_ripple_progress(self) -> float:
        return self._ripple_progress

    def _set_ripple_progress(self, value: float) -> None:
        self._ripple_progress = value
        self.update()

    ripple_progress = pyqtProperty(float, _get_ripple_progress, _set_ripple_progress)

    def paintEvent(self, event) -> None:
        """父类绘制后叠加 ripple 圆形（若进行中）。"""

        super().paintEvent(event)
        if (
            not self._ripple_enabled
            or self._ripple_center is None
            or self._ripple_progress <= 0.0
            or self._ripple_progress >= 1.0
        ):
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        # ripple 半径：从 0 扩散到覆盖按钮最远角。
        cx = self._ripple_center.x()
        cy = self._ripple_center.y()
        # 最远角距离。
        rect = QRectF(self.rect())
        corners = [
            math.hypot(cx - rect.left(), cy - rect.top()),
            math.hypot(cx - rect.right(), cy - rect.top()),
            math.hypot(cx - rect.left(), cy - rect.bottom()),
            math.hypot(cx - rect.right(), cy - rect.bottom()),
        ]
        max_radius = max(corners) if corners else float(rect.width())
        radius = max_radius * self._ripple_progress
        # alpha 从 80 衰减到 0（淡出）。
        alpha = int(80 * (1.0 - self._ripple_progress))
        if alpha <= 0 or radius <= 0:
            return
        gradient = QRadialGradient(cx, cy, radius)
        ripple_color = QColor(P.ACCENT)
        ripple_color.setAlpha(alpha)
        gradient.setColorAt(0.0, ripple_color)
        transparent = QColor(P.ACCENT)
        transparent.setAlpha(0)
        gradient.setColorAt(1.0, transparent)
        painter.setBrush(gradient)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawEllipse(QPointF(cx, cy), radius, radius)


def install_ripple(button: QPushButton) -> None:
    """给现有 QPushButton 动态注入 ripple（不替换类）。

    用于无法改父类的按钮（如 QSS 配置的固定业务按钮）。
    通过 monkey-patch mousePressEvent 捕获点击位置 + 重写 paintEvent 绘制。
    注意：动态注入不如继承 RippleButton 干净，优先用 RippleButton。
    """

    # 简单实现：记录原始方法，注入 ripple 状态。
    button._ripple_enabled = True  # type: ignore[attr-defined]
    button._ripple_progress = 0.0  # type: ignore[attr-defined]
    button._ripple_center = None  # type: ignore[attr-defined]
    button._ripple_anim = None  # type: ignore[attr-defined]
    original_mouse_press = button.mousePressEvent

    def _patched_press(event):
        if getattr(button, "_ripple_enabled", False) and event.button() == Qt.MouseButton.LeftButton:
            _trigger_ripple(button, QPointF(event.position()))
        original_mouse_press(event)

    button.mousePressEvent = _patched_press  # type: ignore[assignment]
    original_paint = button.paintEvent

    def _patched_paint(event):
        original_paint(event)
        _paint_ripple(button)

    button.paintEvent = _patched_paint  # type: ignore[assignment]


def _trigger_ripple(button: QPushButton, center: QPointF) -> None:
    """触发一次 ripple 扩散（install_ripple 辅助）。"""

    button._ripple_center = center  # type: ignore[attr-defined]
    button._ripple_progress = 0.0  # type: ignore[attr-defined]
    if button._ripple_anim is not None:  # type: ignore[attr-defined]
        button._ripple_anim.stop()  # type: ignore[attr-defined]
    anim = QPropertyAnimation(button, b"ripple_progress", button)
    anim.setDuration(AnimationTokens.DURATION_NORMAL)
    anim.setStartValue(0.0)
    anim.setEndValue(1.0)
    anim.setEasingCurve(AnimationTokens.EASE_OUT)
    anim.start()
    button._ripple_anim = anim  # type: ignore[attr-defined]


def _paint_ripple(button: QPushButton) -> None:
    """绘制 ripple 圆形（install_ripple 辅助）。"""

    progress = getattr(button, "_ripple_progress", 0.0)
    center = getattr(button, "_ripple_center", None)
    if progress <= 0.0 or progress >= 1.0 or center is None:
        return
    painter = QPainter(button)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
    cx, cy = center.x(), center.y()
    rect = QRectF(button.rect())
    corners = [
        math.hypot(cx - rect.left(), cy - rect.top()),
        math.hypot(cx - rect.right(), cy - rect.top()),
        math.hypot(cx - rect.left(), cy - rect.bottom()),
        math.hypot(cx - rect.right(), cy - rect.bottom()),
    ]
    max_radius = max(corners) if corners else float(rect.width())
    radius = max_radius * progress
    alpha = int(80 * (1.0 - progress))
    if alpha <= 0 or radius <= 0:
        return
    gradient = QRadialGradient(cx, cy, radius)
    ripple_color = QColor(P.ACCENT)
    ripple_color.setAlpha(alpha)
    gradient.setColorAt(0.0, ripple_color)
    transparent = QColor(P.ACCENT)
    transparent.setAlpha(0)
    gradient.setColorAt(1.0, transparent)
    painter.setBrush(gradient)
    painter.setPen(Qt.PenStyle.NoPen)
    painter.drawEllipse(QPointF(cx, cy), radius, radius)
