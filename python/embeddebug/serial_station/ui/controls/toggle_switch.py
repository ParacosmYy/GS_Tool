"""滑动开关组件 — iOS 风格 on/off 切换。

用于设置面板（显示网格/自动滚动/深色模式/十六进制显示等）。
对标 VOFA+ / MobaXterm 的设置开关。自绘 + 动画。

用法::

    toggle = ToggleSwitch(parent)
    toggle.set_checked(True)
    toggle.toggled.connect(on_toggle_changed)

约束：只依赖 PyQt6 + theme + animations。无业务逻辑。
"""

from __future__ import annotations

from PyQt6.QtCore import QEasingCurve, QPointF, QPropertyAnimation, QRectF, Qt, pyqtSignal
from PyQt6.QtGui import QMouseEvent, QPaintEvent, QPainter, QPen
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.theme import palette as P


_TRACK_W = 36.0
_TRACK_H = 20.0
_KNOB_D = 16.0
_KNOB_MARGIN = (_TRACK_H - _KNOB_D) / 2.0


class ToggleSwitch(QWidget):
    """iOS 风格滑动开关。

    信号：
        toggled(bool): 状态变化时发射。
    """

    toggled = pyqtSignal(bool)

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationToggleSwitch")
        self.setFixedSize(int(_TRACK_W), int(_TRACK_H))
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self._checked = False
        self._knob_x = _KNOB_MARGIN
        self._anim: QPropertyAnimation | None = None

    def is_checked(self) -> bool:
        return self._checked

    def set_checked(self, checked: bool) -> None:
        """程序化设置状态（不发射 toggled 信号）。"""
        if self._checked == checked:
            return
        self._checked = checked
        target = self._knob_target_x()
        self._knob_x = target
        self.update()

    def toggle(self) -> None:
        """切换状态并发射 toggled 信号。"""
        self._checked = not self._checked
        self._animate_knob()
        self.toggled.emit(self._checked)

    def mousePressEvent(self, event: QMouseEvent) -> None:
        if event.button() == Qt.MouseButton.LeftButton:
            self.toggle()
        event.accept()

    def paintEvent(self, event: QPaintEvent) -> None:
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        track_color = P.ACCENT if self._checked else P.BG_INPUT
        painter.setPen(Qt.PenStyle.NoPen)
        from PyQt6.QtGui import QColor

        painter.setBrush(QColor(track_color))
        rect = QRectF(0, 0, _TRACK_W, _TRACK_H)
        painter.drawRoundedRect(rect, _TRACK_H / 2, _TRACK_H / 2)
        # Knob
        knob_color = QColor(P.TEXT_INVERTED)
        painter.setBrush(knob_color)
        knob_rect = QRectF(self._knob_x, _KNOB_MARGIN, _KNOB_D, _KNOB_D)
        painter.drawEllipse(knob_rect)

    def _knob_target_x(self) -> float:
        return (_TRACK_W - _KNOB_D - _KNOB_MARGIN) if self._checked else _KNOB_MARGIN

    def _animate_knob(self) -> None:
        """动画滑动 knob 到目标位置。"""
        target = self._knob_target_x()
        if self._anim is not None:
            self._anim.stop()
        self._anim = QPropertyAnimation(self, b"_knob_pos")
        self._anim.setDuration(160)
        self._anim.setStartValue(self._knob_x)
        self._anim.setEndValue(target)
        self._anim.setEasingCurve(QEasingCurve.Type.OutCubic)
        self._anim.valueChanged.connect(self._on_knob_pos)
        self._anim.start()

    def _on_knob_pos(self, val: float) -> None:
        self._knob_x = val
        self.update()
