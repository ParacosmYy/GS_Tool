"""状态 LED 控件（对齐 VOFA+ 状态灯）。

多色状态灯，可绑定通道值阈值自动变色，或手动设置状态。
自绘圆形 + 内发光效果，颜色取自 palette 状态色。

Batch 3 (B1) 改进：
1. ``_pulse_glow`` 加 ``OutCubic`` 缓动（原默认 Linear，状态灯渐隐机械）。
2. 新增常亮呼吸模式：GREEN/BLUE（连接/活动）状态下持续呼吸（``PulseAnimation``），
   稳定态不再退化成静止色块，状态灯「活」起来。OFF/RED/YELLOW 不呼吸（避免干扰）。
3. 提供 ``set_breathing(bool)`` 手动控制呼吸开关。

约束：本模块只依赖 PyQt6 + theme.palette + animations，不访问 controller/transport。
"""

from __future__ import annotations

from enum import Enum

from PyQt6.QtCore import QPropertyAnimation, QRectF, QSize, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QPainter, QRadialGradient
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T

# 会触发常亮呼吸的状态（连接/活动指示灯应「活」着）。
_BREATHING_STATES = frozenset()


class LedState(Enum):
    """LED 状态枚举。"""

    OFF = "off"          # 灰
    GREEN = "green"      # 正常/连接
    YELLOW = "yellow"    # 警告
    RED = "red"          # 错误/断开
    BLUE = "blue"        # 信息/活动


_STATE_COLORS: dict[LedState, str] = {
    LedState.OFF: P.TEXT_DISABLED,
    LedState.GREEN: P.SUCCESS,
    LedState.YELLOW: P.WARNING,
    LedState.RED: P.ERROR,
    LedState.BLUE: P.TERM_TX,
}

# 默认呼吸的状态：GREEN（已连接）和 BLUE（活动）。
_DEFAULT_BREATHING = frozenset({LedState.GREEN, LedState.BLUE})


class StatusLed(QWidget):
    """自绘状态 LED，支持状态切换、阈值绑定与常亮呼吸。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationStatusLed")
        self._state = LedState.OFF
        self._label = ""
        self.setMinimumSize(QSize(20, 20))
        self._glow = 0.0
        self._breathing_enabled = True  # 默认开启常亮呼吸
        self._breathing_anim: QPropertyAnimation | None = None

    @property
    def state(self) -> LedState:
        return self._state

    def set_state(self, state: LedState) -> None:
        """切换状态，触发短暂发光脉冲；若新状态属于呼吸态则启动常亮呼吸。"""

        if state == self._state:
            return
        self._state = state
        self._stop_breathing()
        self._pulse_glow()
        # 新状态若是呼吸态（GREEN/BLUE）且呼吸开启，启动常亮呼吸。
        if self._breathing_enabled and state in _DEFAULT_BREATHING:
            self._start_breathing()
        self.update()

    def set_breathing(self, enabled: bool) -> None:
        """手动开关常亮呼吸。"""

        self._breathing_enabled = enabled
        if not enabled:
            self._stop_breathing()
        elif self._state in _DEFAULT_BREATHING:
            self._start_breathing()

    def set_label(self, label: str) -> None:
        self._label = label
        self.setToolTip(label)
        self.update()

    def set_from_value(self, value: float, thresholds: tuple[float, float, float]) -> None:
        """按阈值自动设置状态。

        thresholds = (warn_below, error_below, ok_above)：
        - value >= ok_above → GREEN
        - warn_below <= value < ok_above → YELLOW
        - error_below <= value < warn_below → RED（或 OFF 如果更低）
        """

        warn, error, ok = thresholds
        if value >= ok:
            self.set_state(LedState.GREEN)
        elif value >= warn:
            self.set_state(LedState.YELLOW)
        elif value >= error:
            self.set_state(LedState.RED)
        else:
            self.set_state(LedState.OFF)

    def _pulse_glow(self) -> None:
        """状态变化时短暂发光（QPropertyAnimation + OutCubic 缓动）。"""

        self._glow = 1.0
        anim = QPropertyAnimation(self, b"glow", self)
        anim.setDuration(400)
        anim.setStartValue(1.0)
        anim.setEndValue(0.0)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        anim.start()

    def _start_breathing(self) -> None:
        """启动常亮呼吸（glow 在 0.0~0.5 间循环，模拟心跳）。"""

        self._stop_breathing()
        self._breathing_anim = QPropertyAnimation(self, b"glow", self)
        self._breathing_anim.setDuration(1200)
        self._breathing_anim.setStartValue(0.0)
        self._breathing_anim.setKeyValueAt(0.5, 0.45)
        self._breathing_anim.setEndValue(0.0)
        self._breathing_anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        self._breathing_anim.setLoopCount(-1)
        self._breathing_anim.start()

    def _stop_breathing(self) -> None:
        if self._breathing_anim is not None:
            self._breathing_anim.stop()
            self._breathing_anim = None
            self._glow = 0.0
            self.update()

    def _get_glow(self) -> float:
        return self._glow

    def _set_glow(self, value: float) -> None:
        self._glow = value
        self.update()

    glow = pyqtProperty(float, _get_glow, _set_glow)

    def paintEvent(self, event: object) -> None:
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        rect = QRectF(self.rect())
        cx = rect.center().x()
        cy = rect.center().y()
        radius = min(rect.width(), rect.height()) / 2.0 - 2.0
        if radius <= 0:
            return
        color = QColor(_STATE_COLORS[self._state])
        # 外发光（状态变化时增强，呼吸时持续微发光）。
        glow_radius = radius * (1.6 + self._glow * 0.8)
        gradient = QRadialGradient(cx, cy, glow_radius)
        glow_color = QColor(color)
        glow_color.setAlpha(int(60 + self._glow * 120))
        gradient.setColorAt(0.0, glow_color)
        transparent = QColor(color)
        transparent.setAlpha(0)
        gradient.setColorAt(1.0, transparent)
        painter.setBrush(gradient)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawEllipse(QRectF(cx - glow_radius, cy - glow_radius, glow_radius * 2, glow_radius * 2))
        # LED 主体。
        body_gradient = QRadialGradient(cx - radius * 0.3, cy - radius * 0.3, radius * 1.2)
        body_gradient.setColorAt(0.0, color.lighter(150))
        body_gradient.setColorAt(1.0, color.darker(130))
        painter.setBrush(body_gradient)
        painter.drawEllipse(QRectF(cx - radius, cy - radius, radius * 2, radius * 2))
        # 标签（如有）。
        if self._label:
            painter.setPen(QColor(P.TEXT_SECONDARY))
            font = painter.font()
            font.setPointSize(T.FONT_POINT_TINY)
            painter.setFont(font)
            painter.drawText(rect, Qt.AlignmentFlag.AlignBottom | Qt.AlignmentFlag.AlignHCenter, self._label)

    def sizeHint(self) -> QSize:
        return QSize(24, 24)
