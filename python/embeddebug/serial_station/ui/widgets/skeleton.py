"""骨架屏组件 — shimmer 加载占位。

用于数据加载中（如扫描端口、拉取 GATT 树、加载会话）时的占位，替代空白控件。
对齐 Linear/Arc/shadcn 的 skeleton shimmer 语言：灰底 + 横向移动的渐变光带。

Batch 5 (C1) 新建。诊断报告指出全仓零骨架屏实现，OTA/CAN/Automation 等面板在
数据为空时直接显示空白控件，缺少「加载中」占位。

设计要点：
- 自绘 QWidget，paintEvent 画圆角灰底 + 移动的渐变光带（shimmer）。
- shimmer 用 QPropertyAnimation 驱动 ``shimmer_offset``（0.0~1.0）循环移动，
  QLinearGradient 从左到右扫过，模拟 shimmer 效果。
- 多行骨架用 ``SkeletonBlock``（一组行）组合。

约束：只依赖 PyQt6 + theme + animations，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QRectF, Qt, pyqtProperty
from PyQt6.QtGui import QColor, QLinearGradient, QPainter
from PyQt6.QtWidgets import QVBoxLayout, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


class SkeletonWidget(QWidget):
    """单行骨架屏：圆角灰底 + shimmer 扫光。

    用法::

        skeleton = SkeletonWidget(height=20)
        layout.addWidget(skeleton)
    """

    def __init__(self, parent: QWidget | None = None, height: int = 20) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationSkeleton")
        self._shimmer_offset = 0.0
        self._shimmer_anim: QPropertyAnimation | None = None
        self.setFixedHeight(height)
        self.setMinimumWidth(40)
        self._start_shimmer()

    def _start_shimmer(self) -> None:
        """启动 shimmer 循环动画（offset 0→1 无限循环）。"""

        self._shimmer_anim = QPropertyAnimation(self, b"shimmer_offset", self)
        self._shimmer_anim.setDuration(1400)
        self._shimmer_anim.setStartValue(0.0)
        self._shimmer_anim.setEndValue(1.0)
        self._shimmer_anim.setLoopCount(-1)
        self._shimmer_anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        self._shimmer_anim.start()

    def stop_shimmer(self) -> None:
        """停止 shimmer（数据加载完成后调用）。"""

        if self._shimmer_anim is not None:
            self._shimmer_anim.stop()
            self._shimmer_anim = None

    def _get_offset(self) -> float:
        return self._shimmer_offset

    def _set_offset(self, value: float) -> None:
        self._shimmer_offset = value
        self.update()

    shimmer_offset = pyqtProperty(float, _get_offset, _set_offset)

    def paintEvent(self, event: object) -> None:
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        rect = QRectF(self.rect())
        # 灰底（BG_PANEL_RAISED 比卡片底略亮，形成骨架块）。
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QColor(P.BG_PANEL_RAISED))
        painter.drawRoundedRect(rect, 6, 6)
        # shimmer 扫光：从左到右移动的渐变光带。
        w = rect.width()
        band = w * 0.4  # 光带宽度 40%
        x = -band + (w + band) * self._shimmer_offset
        gradient = QLinearGradient(x, 0, x + band, 0)
        gradient.setColorAt(0.0, QColor(255, 255, 255, 0))
        gradient.setColorAt(0.5, QColor(255, 255, 255, 28))
        gradient.setColorAt(1.0, QColor(255, 255, 255, 0))
        painter.setBrush(gradient)
        painter.drawRoundedRect(rect, 6, 6)


class SkeletonBlock(QWidget):
    """多行骨架屏组合：标题行 + 多个正文行 + 间距。

    用于面板首屏加载占位（如 GATT 树、CAN 帧表、日志区）。

    Args:
        rows: 正文行数。
        row_height: 每行高度。
        with_title: 是否含标题行（较宽）。
    """

    def __init__(
        self,
        parent: QWidget | None = None,
        rows: int = 3,
        row_height: int = 16,
        with_title: bool = True,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationSkeletonBlock")
        self._skeletons: list[SkeletonWidget] = []
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)

        if with_title:
            title = SkeletonWidget(self, height=row_height + 4)
            title.setMaximumWidth(180)
            layout.addWidget(title)
            self._skeletons.append(title)

        for i in range(rows):
            row = SkeletonWidget(self, height=row_height)
            # 最后一行做短一点，模拟自然文本末行。
            if i == rows - 1:
                row.setMaximumWidth(int(self.width() * 0.6) if self.width() > 0 else 200)
            layout.addWidget(row)
            self._skeletons.append(row)

    def stop_all(self) -> None:
        """停止所有骨架 shimmer。"""

        for sk in self._skeletons:
            sk.stop_shimmer()
