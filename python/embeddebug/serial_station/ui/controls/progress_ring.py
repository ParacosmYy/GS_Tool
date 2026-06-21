"""环形进度条。

自绘 QPainter 圆弧进度，支持确定值模式 (0..maximum) 与不确定模式（旋转弧段）。
setValue 触发平滑插值动画。用于 OTA 升级进度、数据加载、连接握手、长时间任务指示。

约束：本模块只依赖 PyQt6 + theme.palette + animations，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QRectF, QSize, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QFont, QFontMetrics, QPainter, QPaintEvent, QPen
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


class ProgressRing(QWidget):
    """环形进度条。

    确定模式：从顶部 (90°) 顺时针绘制 ``(_value - minimum) / (maximum - minimum)``
    比例的圆弧，并在中心显示百分比文本。
    不确定模式：绘制一段 90° 弧段，绕圆心匀速旋转。
    ``setValue`` 与 ``setIndeterminate`` 互斥：设值会自动关闭不确定模式。
    """

    def __init__(
        self,
        minimum: int = 0,
        maximum: int = 100,
        value: int = 0,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationProgressRing")
        # 逻辑量程与值（用户设置的语义值）。
        self._minimum: int = int(minimum)
        self._maximum: int = int(maximum)
        self._value: int = int(value)
        # 动画驱动的显示值（可能滞后于 _value），初始与逻辑值同步。
        self._display_value: float = float(self._value)
        # 不确定模式状态。
        self._indeterminate: bool = False
        self._indeterminate_angle: float = 0.0
        # 动画对象引用（防 GC，并支持外部检测）。
        self._value_anim: QPropertyAnimation | None = None
        self._indeterminate_anim: QPropertyAnimation | None = None

    # ── 量程 / 取值 ──────────────────────────────────────────────────
    def setMinimum(self, minimum: int) -> None:
        """设置最小值；若破坏 min<=max 则同步抬高 maximum。"""

        self._minimum = int(minimum)
        if self._maximum < self._minimum:
            self._maximum = self._minimum
        self._clamp_value()
        self.update()

    def minimum(self) -> int:
        return self._minimum

    def setMaximum(self, maximum: int) -> None:
        """设置最大值；若破坏 min<=max 则同步压低 minimum。"""

        self._maximum = int(maximum)
        if self._maximum < self._minimum:
            self._minimum = self._maximum
        self._clamp_value()
        self.update()

    def maximum(self) -> int:
        return self._maximum

    def setRange(self, minimum: int, maximum: int) -> None:
        """一次性设置量程区间。"""

        self._minimum = int(minimum)
        self._maximum = int(maximum)
        if self._maximum < self._minimum:
            self._maximum = self._minimum
        self._clamp_value()
        self.update()

    def setValue(self, value: int) -> None:
        """设置当前值并启动平滑动画。

        值会被 clamp 到 [minimum, maximum]。调用本方法会自动关闭不确定模式
        （两种模式互斥，避免视觉冲突）。
        """

        if self._indeterminate:
            self.setIndeterminate(False)
        clamped = max(self._minimum, min(self._maximum, int(value)))
        self._value = clamped
        # 启动 display_value 动画：从当前显示值插值到新逻辑值。
        if self._value_anim is not None:
            self._value_anim.stop()
        self._value_anim = QPropertyAnimation(self, b"displayValue", self)
        self._value_anim.setDuration(AnimationTokens.DURATION_NORMAL)
        self._value_anim.setStartValue(self._display_value)
        self._value_anim.setEndValue(float(clamped))
        self._value_anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._value_anim.start()

    def value(self) -> int:
        return self._value

    # ── 不确定模式 ───────────────────────────────────────────────────
    def setIndeterminate(self, indeterminate: bool) -> None:
        """开启/关闭不确定模式（旋转弧段）。

        开启时启动 0→360 无限循环动画；关闭时停止动画并清除引用。
        """

        self._indeterminate = bool(indeterminate)
        if indeterminate:
            if self._indeterminate_anim is not None:
                self._indeterminate_anim.stop()
            anim = QPropertyAnimation(self, b"indeterminateAngle", self)
            anim.setDuration(AnimationTokens.DURATION_SLOWER)
            anim.setStartValue(0.0)
            anim.setEndValue(360.0)
            anim.setEasingCurve(AnimationTokens.LINEAR)
            anim.setLoopCount(-1)  # 无限循环
            self._indeterminate_anim = anim
            anim.start()
        else:
            if self._indeterminate_anim is not None:
                self._indeterminate_anim.stop()
            self._indeterminate_anim = None
        self.update()

    def isIndeterminate(self) -> bool:
        return self._indeterminate

    # ── 内部辅助 ─────────────────────────────────────────────────────
    def _clamp_value(self) -> None:
        """量程变化后把 _value 与 _display_value 重新 clamp 到新区间。"""

        clamped = max(self._minimum, min(self._maximum, self._value))
        if clamped != self._value:
            self._value = clamped
        self._display_value = float(clamped)

    # ── pyqtProperty: displayValue（值动画目标） ─────────────────────
    def _get_display_value(self) -> float:
        return self._display_value

    def _set_display_value(self, value: float) -> None:
        self._display_value = float(value)
        self.update()

    displayValue = pyqtProperty(float, _get_display_value, _set_display_value)

    # ── pyqtProperty: indeterminateAngle（旋转动画目标） ─────────────
    def _get_indeterminate_angle(self) -> float:
        return self._indeterminate_angle

    def _set_indeterminate_angle(self, angle: float) -> None:
        self._indeterminate_angle = float(angle)
        self.update()

    indeterminateAngle = pyqtProperty(float, _get_indeterminate_angle, _set_indeterminate_angle)

    # ── 尺寸 / 绘制 ──────────────────────────────────────────────────
    def sizeHint(self) -> QSize:
        return QSize(64, 64)

    def paintEvent(self, event: QPaintEvent) -> None:
        """绘制背景轨道 + 进度弧 + 中心百分比文本。

        - 不确定模式：从 ``_indeterminate_angle`` 起，绘制 90° 旋转弧段。
        - 确定模式：从顶部 (90°) 起，按比例绘制进度弧。
        - ``_maximum == _minimum`` 时跳过比例计算（防除零），仅绘制轨道。
        """

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        pen_width = 4
        # 圆环外接矩形：留出 pen 宽度的内边距，避免被裁剪。
        rect = QRectF(self.rect()).adjusted(
            pen_width / 2.0, pen_width / 2.0, -pen_width / 2.0, -pen_width / 2.0
        )
        side = min(rect.width(), rect.height())
        if side <= 0:
            return
        # 以中心为基准构造正方形外接矩形，保证圆环不变形。
        cx = rect.center().x()
        cy = rect.center().y()
        ring_rect = QRectF(cx - side / 2.0, cy - side / 2.0, side, side)

        # 背景轨道（整圆，弱色）。
        track_pen = QPen(QColor(P.BG_PANEL), pen_width)
        track_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(track_pen)
        painter.drawArc(ring_rect, 0, 360 * 16)

        # 进度弧。
        value_pen = QPen(QColor(P.ACCENT), pen_width)
        value_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(value_pen)
        if self._indeterminate:
            # 旋转弧段：从当前角度起顺时针扫 90°。
            start = int(self._indeterminate_angle * 16)
            painter.drawArc(ring_rect, start, 90 * 16)
        else:
            span = self._maximum - self._minimum
            if span > 0:
                # 显示值 clamp 后再算比例，防止动画过冲溢出。
                disp_clamped = max(
                    float(self._minimum), min(float(self._maximum), self._display_value)
                )
                ratio = (disp_clamped - float(self._minimum)) / float(span)
                ratio = max(0.0, min(1.0, ratio))
                # Qt drawArc 角度以 1/16° 为单位，0° 在 3 点钟方向。
                # 顶部 12 点钟是 90°，顺时针为负角度方向。
                start_angle = 90 * 16
                sweep = int(-ratio * 360 * 16)
                painter.drawArc(ring_rect, start_angle, sweep)

        # 中心百分比文本（仅确定模式且尺寸足够时绘制）。
        if (not self._indeterminate) and side >= 40 and span > 0:
            disp_clamped = max(
                float(self._minimum), min(float(self._maximum), self._display_value)
            )
            ratio = (disp_clamped - float(self._minimum)) / float(span)
            ratio = max(0.0, min(1.0, ratio))
            percent = int(round(ratio * 100))
            text = f"{percent}%"
            font = QFont()
            font.setPointSizeF(max(7.0, side / 6.0))
            font.setBold(True)
            painter.setFont(font)
            # 根据实际文本宽度选择对比色，避免长文本溢出。
            metrics = QFontMetrics(font)
            if metrics.horizontalAdvance(text) > side - pen_width * 2 - 4:
                painter.setPen(QColor(P.TEXT_SECONDARY))
            else:
                painter.setPen(QColor(P.TEXT_PRIMARY))
            painter.drawText(ring_rect, Qt.AlignmentFlag.AlignCenter, text)
