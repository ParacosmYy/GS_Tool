"""状态圆点控件 — 小尺寸彩色指示灯 + 可选文本，呼吸态激活 PulseAnimation。

对齐 Linear/Arc/Slack 的连接状态指示器语言：一个 8px 彩色圆点 + 文字，
活动态（连接/运行中）持续呼吸，非活动态静止。相比面板里用 ``●``/``○`` 字符
占位，本控件是真正的自绘控件 + 动画，且把 ``animations.pulse.PulseAnimation``
从死代码接入实际用户路径（Batch 10）。

设计要点：
- 自绘 QWidget，paintEvent 画实心圆（颜色取 palette 状态色）。
- 活动态（GREEN/BLUE）调 ``PulseAnimation.breathing`` 启动透明度呼吸循环；
  非活动态（OFF/YELLOW/RED）静止，避免干扰。
- 可选文本标签（默认无），与圆点水平排列由调用方布局决定；本控件只画点。
- ``set_state`` / ``set_active`` 控制状态切换，呼吸动画随状态启停。

约束：只依赖 PyQt6 + theme.palette + animations.pulse，不访问 controller/transport。
"""

from __future__ import annotations

from enum import Enum

from PyQt6.QtCore import QPropertyAnimation, QRectF, QSize, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QPainter, QRadialGradient
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.pulse import PulseAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


class DotState(Enum):
    """圆点状态枚举（语义对齐 StatusLed.LedState，但独立以避免耦合）。"""

    OFF = "off"        # 灰：未连接 / 未启用
    GREEN = "green"    # 绿：已连接 / 正常
    YELLOW = "yellow"  # 黄：警告 / 中间态
    RED = "red"        # 红：错误 / 断开
    BLUE = "blue"      # 蓝：活动 / 信息


_STATE_COLORS: dict[DotState, str] = {
    DotState.OFF: P.TEXT_DISABLED,
    DotState.GREEN: P.SUCCESS,
    DotState.YELLOW: P.WARNING,
    DotState.RED: P.ERROR,
    DotState.BLUE: P.TERM_TX,
}

# 默认呼吸的活动态：GREEN（已连接）/ BLUE（活动）。
_DEFAULT_BREATHING = frozenset({DotState.GREEN, DotState.BLUE})


class StatusDot(QWidget):
    """小尺寸状态圆点，活动态呼吸（激活 PulseAnimation 死代码）。

    用法::

        dot = StatusDot()
        layout.addWidget(dot)
        dot.set_state(DotState.GREEN)        # 连接成功，开始呼吸
        dot.set_state(DotState.OFF)          # 断开，停止呼吸

    Args:
        diameter: 圆点直径（px）。
        breathing: 是否对活动态启用呼吸动画（默认 True，可关闭用于静态场景）。
        parent: 父控件。
    """

    def __init__(
        self,
        diameter: int = 8,
        breathing: bool = True,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationStatusDot")
        self._state = DotState.OFF
        self._diameter = max(6, int(diameter))
        self._breathing_enabled = bool(breathing)
        # PulseAnimation 基于 QGraphicsOpacityEffect 驱动 widget 整体透明度，
        # 无需自管 _glow 属性；状态变化用 _flash 短暂高亮（自管 QPropertyAnimation）。
        self._flash = 0.0
        self._flash_anim: QPropertyAnimation | None = None
        self._breathing_anim: QPropertyAnimation | None = None
        self.setFixedSize(QSize(self._diameter, self._diameter))

    @property
    def state(self) -> DotState:
        """当前状态。"""

        return self._state

    def set_state(self, state: DotState) -> None:
        """切换状态：触发短暂闪光；活动态启动呼吸，非活动态停止。

        同状态重复调用视为无变化，不重启动画（避免抖动）。
        """

        if state == self._state:
            return
        self._state = state
        self._stop_breathing()
        self._pulse_flash()
        if self._breathing_enabled and state in _DEFAULT_BREATHING:
            self._start_breathing()
        self.update()

    def set_active(self, active: bool) -> None:
        """便捷切换：active=True→GREEN（呼吸），False→OFF（静止）。"""

        self.set_state(DotState.GREEN if active else DotState.OFF)

    def set_breathing(self, enabled: bool) -> None:
        """手动开关呼吸动画（不影响状态）。"""

        self._breathing_enabled = bool(enabled)
        if not enabled:
            self._stop_breathing()
        elif self._state in _DEFAULT_BREATHING:
            self._start_breathing()

    # ── 动画 ────────────────────────────────────────────────────────
    def _pulse_flash(self) -> None:
        """状态变化时短暂高亮闪光（OutCubic 缓动，自管 QPropertyAnimation）。"""

        self._flash = 1.0
        self._flash_anim = QPropertyAnimation(self, b"flash", self)
        self._flash_anim.setDuration(380)
        self._flash_anim.setStartValue(1.0)
        self._flash_anim.setEndValue(0.0)
        self._flash_anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._flash_anim.start()

    def _start_breathing(self) -> None:
        """启动呼吸（PulseAnimation.breathing 无限循环），持有引用防 GC。

        Batch 10：把 animations.pulse.PulseAnimation 从死代码接入实际用户路径。
        """

        self._stop_breathing()
        # PulseAnimation.breathing 返回 loopCount=-1 的动画，finished 永不触发，
        # 必须由调用方持有引用并在不再需要时 stop_looping；这里存在实例属性防 GC。
        self._breathing_anim = PulseAnimation.breathing(
            self, min_opacity=0.45, max_opacity=1.0, interval_ms=1400
        )
        self._breathing_anim.start()

    def _stop_breathing(self) -> None:
        """停止呼吸：PulseAnimation.stop_looping 清理 _active 列表残留。"""

        if self._breathing_anim is not None:
            PulseAnimation.stop_looping(self)
            self._breathing_anim = None

    # ── flash 属性（状态变化闪光） ─────────────────────────────────
    def _get_flash(self) -> float:
        return self._flash

    def _set_flash(self, value: float) -> None:
        self._flash = float(value)
        self.update()

    flash = pyqtProperty(float, _get_flash, _set_flash)

    # ── 绘制 ────────────────────────────────────────────────────────
    def paintEvent(self, event: object) -> None:
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        rect = QRectF(self.rect())
        cx = rect.center().x()
        cy = rect.center().y()
        radius = min(rect.width(), rect.height()) / 2.0 - 1.0
        if radius <= 0:
            return
        color = QColor(_STATE_COLORS[self._state])
        # 外发光：状态变化闪光增强，呼吸态由 PulseAnimation 控制 widget 整体透明度。
        glow_radius = radius * (1.5 + self._flash * 0.9)
        gradient = QRadialGradient(cx, cy, glow_radius)
        glow_color = QColor(color)
        glow_color.setAlpha(int(50 + self._flash * 110))
        gradient.setColorAt(0.0, glow_color)
        transparent = QColor(color)
        transparent.setAlpha(0)
        gradient.setColorAt(1.0, transparent)
        painter.setBrush(gradient)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawEllipse(
            QRectF(cx - glow_radius, cy - glow_radius, glow_radius * 2, glow_radius * 2)
        )
        # 圆点主体：径向渐变模拟立体球感。
        body_gradient = QRadialGradient(cx - radius * 0.3, cy - radius * 0.3, radius * 1.3)
        body_gradient.setColorAt(0.0, color.lighter(150))
        body_gradient.setColorAt(1.0, color.darker(125))
        painter.setBrush(body_gradient)
        painter.drawEllipse(QRectF(cx - radius, cy - radius, radius * 2, radius * 2))

    def sizeHint(self) -> QSize:
        return QSize(self._diameter, self._diameter)
