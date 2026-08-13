"""Presentation-only empty state for the command batch workspace."""

from __future__ import annotations

import math
from enum import StrEnum

from .action_surface import ActionRailButton
from .qt import (
    QColor,
    QEvent,
    QFrame,
    QGridLayout,
    QLabel,
    QLayout,
    QPainter,
    QPen,
    QPointF,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
    Signal,
)
from .theme import theme_spec_for_widget


class CommandBatchGlyph(QWidget):
    """Draw a small step-map glyph without relying on a font or image asset."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(58, 58)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_frame(self, phase: float, animated: bool) -> None:
        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        del event
        theme = theme_spec_for_widget(self)
        bounds = self.rect().adjusted(4, 4, -4, -4)
        center = QPointF(bounds.center())
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        surface = QColor(theme.surface_input)
        surface.setAlpha(210)
        border = QColor(theme.border)
        border.setAlpha(180)
        painter.setBrush(surface)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 13.0, 13.0)

        rail = QColor(theme.accent_purple)
        rail.setAlpha(205 if self._animated else 145)
        painter.setPen(QPen(rail, 1.4))
        painter.drawLine(
            QPointF(bounds.left() + 13.0, center.y()),
            QPointF(bounds.right() - 13.0, center.y()),
        )
        for index in range(3):
            x = bounds.left() + 13.0 + index * 12.0
            node = QColor(theme.accent if index == 2 else theme.accent_blue)
            node.setAlpha(220 if self._animated else 155)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(node)
            painter.drawEllipse(QPointF(x, center.y()), 3.0, 3.0)

        if self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(theme.accent)
            pulse.setAlpha(42 + int(wave * 48))
            painter.setBrush(pulse)
            painter.drawEllipse(center, 11.0 + wave * 3.0, 11.0 + wave * 3.0)


class CommandBatchStepCard(QFrame):
    """Show one static command-batch onboarding step without owning state."""

    def __init__(self, number: int, title: str, hint: str, parent: QWidget) -> None:
        super().__init__(parent)
        self.setObjectName("commandBatchStep")
        self.setProperty("role", "surface")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        self.setAccessibleName(f"批量命令步骤 {number}：{title}")
        self.setAccessibleDescription(hint)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 10, 12, 10)
        layout.setSpacing(4)

        index = QLabel(f"{number:02d}", self)
        index.setObjectName("commandBatchStepIndex")
        index.setProperty("role", "subtle")
        index.setAccessibleName(f"第 {number} 步")
        layout.addWidget(index)

        title_label = QLabel(title, self)
        title_label.setObjectName("commandBatchStepTitle")
        title_label.setProperty("role", "section")
        title_label.setSizePolicy(
            QSizePolicy.Policy.Ignored,
            QSizePolicy.Policy.Fixed,
        )
        layout.addWidget(title_label)

        hint_label = QLabel(hint, self)
        hint_label.setObjectName("commandBatchStepHint")
        hint_label.setProperty("role", "muted")
        hint_label.setWordWrap(True)
        hint_label.setSizePolicy(
            QSizePolicy.Policy.Ignored,
            QSizePolicy.Policy.Preferred,
        )
        layout.addWidget(hint_label)


class CommandBatchStepRail(QWidget):
    """Responsive three-step guide for the command workspace empty state."""

    _WIDE_MIN_WIDTH = 720
    _WIDE_COLUMNS = 3
    _COMPACT_COLUMNS = 1

    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        self.setObjectName("commandBatchStepRail")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        self.setAccessibleName("批量命令快速开始")
        self.setAccessibleDescription(
            "新建批量命令，添加发送步骤，然后执行并查看每个步骤的结果。"
        )
        self._columns = 0
        self._layout = QGridLayout(self)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setVerticalSpacing(8)
        self._cards = (
            CommandBatchStepCard(1, "新建", "先命名一个可复用的发送序列。", self),
            CommandBatchStepCard(2, "添加步骤", "按顺序加入常用的发送内容。", self),
            CommandBatchStepCard(3, "执行查看", "运行后在下方查看步骤状态。", self),
        )
        self._relayout()

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def _relayout(self, columns: int | None = None) -> None:
        """Place every existing card once for the current logical width."""

        if columns is None:
            columns = (
                self._WIDE_COLUMNS
                if self.width() >= self._WIDE_MIN_WIDTH
                else self._COMPACT_COLUMNS
            )
        if columns == self._columns and self._layout.count() == len(self._cards):
            return
        self._columns = columns
        for card_index, card in enumerate(self._cards):
            self._layout.removeWidget(card)
            row, column = divmod(card_index, columns)
            self._layout.addWidget(card, row, column)
        self._layout.setHorizontalSpacing(10 if columns > 1 else 0)
        for column in range(self._WIDE_COLUMNS):
            self._layout.setColumnStretch(column, 1 if column < columns else 0)


class _CommandEmptyHeaderMode(StrEnum):
    """Responsive compositions for the empty-state header."""

    REGULAR = "regular"
    COMPACT = "compact"
    NARROW = "narrow"


class _ResponsiveCommandEmptyHeader(QWidget):
    """Reflow existing empty-state visuals without owning their behavior."""

    def __init__(
        self,
        glyph: QWidget,
        copy_layout: QVBoxLayout,
        action_button: QWidget,
        *,
        parent: QWidget,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("responsiveCommandEmptyHeader")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._glyph = glyph
        self._copy_layout = copy_layout
        self._action_button = action_button
        self._mode: _CommandEmptyHeaderMode | None = None
        self._reflowing = False
        self._layout = QGridLayout(self)
        self._layout.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(16)
        self._layout.setVerticalSpacing(8)
        self._relayout(force=True)

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def changeEvent(self, event: object) -> None:
        super().changeEvent(event)  # type: ignore[arg-type]
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self._invalidate_sizing()

    def event(self, event: object) -> bool:
        accepted = super().event(event)  # type: ignore[arg-type]
        if event.type() is QEvent.Type.LayoutRequest:
            self._invalidate_sizing()
        return accepted

    def minimumSizeHint(self) -> object:
        hint = super().minimumSizeHint()
        hint.setWidth(self._required_width(_CommandEmptyHeaderMode.NARROW))
        return hint

    def sizeHint(self) -> object:
        hint = super().sizeHint()
        hint.setWidth(
            max(
                hint.width(),
                self._required_width(_CommandEmptyHeaderMode.REGULAR, preferred=True),
            )
        )
        return hint

    def _invalidate_sizing(self) -> None:
        if self._reflowing:
            return
        self._layout.invalidate()
        self._layout.activate()
        self.updateGeometry()
        self._relayout()

    def _relayout(self, *, force: bool = False) -> None:
        if self._reflowing:
            return
        mode = self._mode_for_width()
        if not force and mode is self._mode:
            self._apply_column_contract(mode)
            self._layout.invalidate()
            self._layout.activate()
            return

        self._reflowing = True
        try:
            self._layout.removeWidget(self._glyph)
            self._layout.removeItem(self._copy_layout)
            self._layout.removeWidget(self._action_button)
            if mode is _CommandEmptyHeaderMode.REGULAR:
                self._layout.addWidget(
                    self._glyph,
                    0,
                    0,
                    1,
                    1,
                    Qt.AlignmentFlag.AlignVCenter,
                )
                self._layout.addLayout(self._copy_layout, 0, 1)
                self._layout.addWidget(
                    self._action_button,
                    0,
                    2,
                    1,
                    1,
                    Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter,
                )
            elif mode is _CommandEmptyHeaderMode.COMPACT:
                self._layout.addWidget(
                    self._glyph,
                    0,
                    0,
                    1,
                    1,
                    Qt.AlignmentFlag.AlignVCenter,
                )
                self._layout.addLayout(self._copy_layout, 0, 1)
                self._layout.addWidget(
                    self._action_button,
                    1,
                    0,
                    1,
                    2,
                    Qt.AlignmentFlag.AlignLeft,
                )
            else:
                self._layout.addWidget(
                    self._glyph,
                    0,
                    0,
                    alignment=Qt.AlignmentFlag.AlignLeft,
                )
                self._layout.addLayout(self._copy_layout, 1, 0)
                self._layout.addWidget(
                    self._action_button,
                    2,
                    0,
                    alignment=Qt.AlignmentFlag.AlignLeft,
                )
            self._apply_column_contract(mode)
            self._mode = mode
            self._layout.invalidate()
            self._layout.activate()
            self.updateGeometry()
        finally:
            self._reflowing = False

    def _mode_for_width(self) -> _CommandEmptyHeaderMode:
        available_width = max(0, self.contentsRect().width())
        if available_width >= self._required_width(
            _CommandEmptyHeaderMode.REGULAR,
            preferred=True,
        ):
            return _CommandEmptyHeaderMode.REGULAR
        if available_width >= self._required_width(_CommandEmptyHeaderMode.COMPACT):
            return _CommandEmptyHeaderMode.COMPACT
        return _CommandEmptyHeaderMode.NARROW

    def _required_width(
        self,
        mode: _CommandEmptyHeaderMode,
        *,
        preferred: bool = False,
    ) -> int:
        glyph_width = self._widget_width(self._glyph, preferred=preferred)
        action_width = self._widget_width(self._action_button, preferred=preferred)
        copy_width = self._layout_width(self._copy_layout, preferred=preferred)
        spacing = max(0, self._layout.horizontalSpacing())
        if mode is _CommandEmptyHeaderMode.REGULAR:
            width = glyph_width + copy_width + action_width + spacing * 2
        elif mode is _CommandEmptyHeaderMode.COMPACT:
            width = max(glyph_width + copy_width + spacing, action_width)
        else:
            width = max(glyph_width, copy_width, action_width)
        margins = self._layout.contentsMargins()
        return width + margins.left() + margins.right()

    def _apply_column_contract(self, mode: _CommandEmptyHeaderMode) -> None:
        for column in range(3):
            self._layout.setColumnMinimumWidth(column, 0)
            self._layout.setColumnStretch(column, 0)
        if mode in {
            _CommandEmptyHeaderMode.REGULAR,
            _CommandEmptyHeaderMode.COMPACT,
        }:
            self._layout.setColumnStretch(1, 1)
        else:
            self._layout.setColumnStretch(0, 1)

    @staticmethod
    def _layout_width(layout: QVBoxLayout, *, preferred: bool) -> int:
        minimum = layout.minimumSize().width()
        return max(minimum, layout.sizeHint().width()) if preferred else minimum

    @staticmethod
    def _widget_width(widget: QWidget, *, preferred: bool) -> int:
        minimum = max(widget.minimumWidth(), widget.minimumSizeHint().width())
        return max(minimum, widget.sizeHint().width()) if preferred else minimum


class CommandBatchEmptyState(QFrame):
    """Guide the first batch-command action without owning command state."""

    MOTION_MODE = "activity"
    new_requested = Signal()

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("commandBatchEmptyState")
        self.setProperty("role", "surface")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        self.setAccessibleName("批量命令空态")

        root = QVBoxLayout(self)
        root.setContentsMargins(18, 16, 18, 16)
        root.setSpacing(14)

        self._glyph = CommandBatchGlyph(self)

        copy = QVBoxLayout()
        copy.setContentsMargins(0, 0, 0, 0)
        copy.setSpacing(3)
        self._eyebrow = QLabel("COMMAND BATCH / READY", self)
        self._eyebrow.setObjectName("commandBatchEmptyEyebrow")
        self._eyebrow.setProperty("role", "subtle")
        copy.addWidget(self._eyebrow)
        self._title = QLabel("还没有批量命令", self)
        self._title.setObjectName("commandBatchEmptyTitle")
        self._title.setAccessibleName("批量命令空态标题")
        copy.addWidget(self._title)
        self._hint = QLabel(
            "把常用发送步骤保存成一个序列，之后可以一键按顺序执行。",
            self,
        )
        self._hint.setObjectName("commandBatchEmptyHint")
        self._hint.setProperty("role", "muted")
        self._hint.setWordWrap(True)
        self._hint.setAccessibleName("批量命令下一步提示")
        copy.addWidget(self._hint)
        # Keep copy compact while the empty-state card uses spare space for
        # composition; otherwise each one-line label becomes a tall blank band.
        for copy_label in (self._eyebrow, self._title, self._hint):
            copy_label.setSizePolicy(
                QSizePolicy.Policy.Preferred,
                QSizePolicy.Policy.Fixed,
            )

        self.action_button = ActionRailButton("新建批量命令", self)
        self.action_button.setObjectName("primaryButton")
        self.action_button.setAccessibleName("新建批量命令")
        self.action_button.setAccessibleDescription("打开批量命令编辑器。")
        self.action_button.setToolTip("打开批量命令编辑器。")
        self.action_button.clicked.connect(self._emit_new_requested)
        header = _ResponsiveCommandEmptyHeader(
            self._glyph,
            copy,
            self.action_button,
            parent=self,
        )
        root.addWidget(header)

        self._step_rail = CommandBatchStepRail(self)
        root.addWidget(self._step_rail)

    def set_content(self, title: str, hint: str) -> None:
        """Update copy from a presentation projection, not from application state."""

        self._title.setText(title)
        self._hint.setText(hint)
        description = f"{title}。{hint}"
        self.setAccessibleDescription(description)
        self._hint.setAccessibleDescription(hint)

    def set_frame(self, phase: float, animated: bool) -> None:
        """Fan the shared shell frame to the decorative children."""

        self._glyph.set_frame(phase, animated)
        self.action_button.set_frame(phase, animated)

    def stop(self) -> None:
        """Freeze all decoration during reduced motion, pause, hide, or close."""

        self._glyph.stop()
        self.action_button.stop()

    def _emit_new_requested(self, checked: bool = False) -> None:
        del checked
        self.new_requested.emit()


__all__ = ["CommandBatchEmptyState", "CommandBatchGlyph"]
