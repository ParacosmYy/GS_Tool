"""iOS/Material 分段控件。

横向多段选择器，选中段用 accent 色背景指示，切换时滑动指示器动画过渡。
用于视图模式切换、示波器通道选择、过滤器分组。

设计要点：
- N 个等宽段，点击任意段切换 current，触发滑动动画。
- 指示器使用 QPropertyAnimation 驱动 ``_indicator_rect``（QRectF，支持亚像素
  平滑插值），避免整数 QRect 跳变。
- 绘制顺序：背景 → 选中段强调底 → 指示器 → 文本（QFontMetrics 居中）。
- 选中段文字（位于 accent 指示器上）用 TEXT_ON_ACCENT，未选中用 TEXT_SECONDARY，对比清晰。

约束：只依赖 PyQt6 + theme + animations，不引入业务层。
"""

from __future__ import annotations


from PyQt6.QtCore import (
    QPropertyAnimation,
    QRectF,
    QSize,
    Qt,
    pyqtProperty,
    pyqtSignal,
)
from PyQt6.QtGui import QColor, QFontMetrics, QMouseEvent, QPainter, QPaintEvent
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P


class SegmentedControl(QWidget):
    """iOS/Material 风格分段控件。

    一行 N 个选项，单选；切换时指示器在段间滑动。通过 ``currentChanged``
    通知外部当前索引变化。

    用法::

        seg = SegmentedControl(["Hex", "ASCII", "Dec"])
        seg.currentChanged.connect(on_mode_changed)
    """

    # 选中索引变化时发射（重复 setCurrent 同索引不发射）。
    currentChanged = pyqtSignal(int)

    def __init__(
        self,
        options: list[str] | None = None,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationSegmentedControl")

        self._options: list[str] = list(options) if options else []
        self._current: int = 0 if self._options else -1
        self._indicator_rect: QRectF = QRectF()
        # 指示器动画（per-widget，避免类级共享）。
        self._indicator_anim: QPropertyAnimation | None = None
        # 防止动画被 GC 的活跃列表（per-instance）。
        self._active_anims: list[QPropertyAnimation] = []

        # 初始化指示器位置（若已有选项）。
        if self._options:
            self._current = 0
            self._indicator_rect = self._segment_rect(0)

    # ── 属性 ────────────────────────────────────────────────────────
    def setOptions(self, options: list[str]) -> None:
        """替换选项列表，钳制 current 到合法区间并即时刷新指示器。"""

        self._options = list(options)
        if not self._options:
            self._current = -1
            self._indicator_rect = QRectF()
            self.update()
            return
        # 钳制到合法范围。
        if self._current < 0 or self._current >= len(self._options):
            self._current = 0
        # 即时定位（无动画），保证新选项布局立刻可见。
        self._indicator_rect = self._segment_rect(self._current)
        self.update()

    def options(self) -> list[str]:
        """返回当前选项列表的拷贝。"""

        return list(self._options)

    def setCurrent(self, index: int) -> None:
        """切换到 index（钳制），动画移动指示器；变化时发射 currentChanged。"""

        if not self._options:
            return
        clamped = max(0, min(index, len(self._options) - 1))
        if clamped == self._current:
            # 同索引不发射、不重复动画。
            return
        self._current = clamped
        self._animate_indicator_to(self._segment_rect(clamped))
        self.currentChanged.emit(clamped)

    def current(self) -> int:
        """返回当前选中索引（无选项时为 -1）。"""

        return self._current

    # ── 几何/尺寸 ────────────────────────────────────────────────────
    def sizeHint(self) -> QSize:
        """根据选项数和文本宽度估算 sizeHint，高度固定 32px。"""

        fm = QFontMetrics(self.font())
        max_text_w = max((fm.horizontalAdvance(opt) for opt in self._options), default=0)
        # 每段预留 文本宽度 + 左右各 24px padding；最小宽度按段数 * 64px。
        per_seg = max(64, max_text_w + 48)
        width = per_seg * max(1, len(self._options))
        return QSize(width, 32)

    def _segment_rect(self, index: int) -> QRectF:
        """计算 index 段的几何（等宽切分控件内容区）。"""

        if not self._options or index < 0 or index >= len(self._options):
            return QRectF()
        n = len(self._options)
        w = self.width()
        h = self.height()
        seg_w = w / n
        x = index * seg_w
        # 指示器留 2px 内边距，避免顶满边框。
        inset = 2.0
        return QRectF(x + inset, inset, seg_w - 2 * inset, h - 2 * inset)

    # ── 指示器动画 ──────────────────────────────────────────────────
    def _animate_indicator_to(self, target_rect: QRectF) -> None:
        """从当前指示器位置滑动到 target_rect（DURATION_NORMAL + EASE_OUT）。"""

        if self._indicator_anim is not None:
            self._indicator_anim.stop()
        # property 名 ``indicatorRect``（pyqtProperty 暴露），底层存储为
        # ``self._indicator_rect``（实例属性）。二者不重名，避免实例属性
        # 覆盖类级 property（参见 ripple.py 的 _ripple_progress / ripple_progress 范式）。
        anim = QPropertyAnimation(self, b"indicatorRect", self)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        anim.setStartValue(self._indicator_rect)
        anim.setEndValue(target_rect)
        # Batch 53: 消费预留 token EASE_OUT_QUAD（分段指示器轻量出场缓动）。
        anim.setEasingCurve(AnimationTokens.EASE_OUT_QUAD)
        # per-instance 跟踪，避免 GC。
        self._active_anims.append(anim)
        anim.finished.connect(lambda a=anim: self._on_anim_finished(a))
        self._indicator_anim = anim
        anim.start()

    def _on_anim_finished(self, anim: QPropertyAnimation) -> None:
        """动画结束后从活跃列表移除，防止列表无限增长。"""

        if anim in self._active_anims:
            self._active_anims.remove(anim)
        if self._indicator_anim is anim:
            self._indicator_anim = None

    # ── indicatorRect 作为 pyqtProperty 供动画驱动 ────────────────
    # 底层存储 ``self._indicator_rect``（实例属性），对外 property 名
    # ``indicatorRect``。二者不重名，避免实例属性覆盖类级 property。
    def _get_indicator_rect(self) -> QRectF:
        return self._indicator_rect

    def _set_indicator_rect(self, rect: QRectF) -> None:
        self._indicator_rect = rect
        self.update()

    indicatorRect = pyqtProperty(QRectF, _get_indicator_rect, _set_indicator_rect)

    # ── 绘制 ────────────────────────────────────────────────────────
    def paintEvent(self, event: QPaintEvent) -> None:
        """绘制背景/边框、指示器、每段文本。"""

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        rect = QRectF(self.rect())
        radius = 6.0

        # 1. 背景面板 + 1px 边框。
        painter.setPen(QColor(P.BORDER))
        painter.setBrush(QColor(P.BG_PANEL))
        painter.drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius)

        if not self._options:
            return

        # 2. 选中段强调底（ACCENT_SOFT，比指示器更宽的 tint）。
        if 0 <= self._current < len(self._options):
            sel_slot = self._segment_rect(self._current)
            painter.setPen(Qt.PenStyle.NoPen)
            soft = QColor(P.ACCENT_SOFT)
            painter.setBrush(soft)
            painter.drawRoundedRect(sel_slot, radius - 1, radius - 1)

        # 3. 指示器（accent 填充，位于 _indicator_rect）。
        if not self._indicator_rect.isNull():
            painter.setBrush(QColor(P.ACCENT))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.drawRoundedRect(self._indicator_rect, radius - 1, radius - 1)

        # 4. 文本：每段居中；选中文本 ACCENT_HOVER，未选 TEXT_SECONDARY。
        fm = QFontMetrics(self.font())
        n = len(self._options)
        seg_w = self.width() / n
        for i, opt in enumerate(self._options):
            text_w = fm.horizontalAdvance(opt)
            text_h = fm.height()
            cx = i * seg_w + seg_w / 2.0
            cy = self.height() / 2.0 + fm.ascent() - text_h / 2.0
            # 指示器覆盖范围内的文字走 on-accent 色。
            if i == self._current:
                color = QColor(P.TEXT_ON_ACCENT)
            else:
                color = QColor(P.TEXT_SECONDARY)
            painter.setPen(color)
            painter.drawText(
                int(cx - text_w / 2.0),
                int(cy),
                opt,
            )

    # ── 交互 ────────────────────────────────────────────────────────
    def mousePressEvent(self, event: QMouseEvent) -> None:
        """左键点击对应段则切换 current；接受事件。"""

        if event.button() != Qt.MouseButton.LeftButton or not self._options:
            event.ignore()
            return
        x = event.position().x()
        seg_w = self.width() / len(self._options)
        index = int(x // seg_w)
        if 0 <= index < len(self._options):
            self.setCurrent(index)
        event.accept()
