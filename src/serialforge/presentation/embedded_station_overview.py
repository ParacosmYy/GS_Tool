"""Read-only overview of the embedded extension station boundary."""

from __future__ import annotations

import math

from ..application.extension_station import ExtensionStationSummary
from .qt import (
    QColor,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QPainter,
    QPen,
    QPointF,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
)
from .theme import theme_spec_for_widget

_METRIC_MIN_WIDTH = 104
_METRIC_SPACING = 14
_METRIC_COLUMN_OPTIONS = (6, 3, 2, 1)


class _ResponsiveMetricGrid(QWidget):
    """Reflow existing station metrics without owning station state."""

    def __init__(
        self,
        items: tuple[tuple[str, str], ...],
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("extensionStationMetrics")
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._cells = tuple(_build_metric_cell(label, value) for label, value in items)
        self._column_count = 0
        self._layout = QGridLayout(self)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(_METRIC_SPACING)
        self._layout.setVerticalSpacing(5)
        self._refresh_columns()

    def resizeEvent(self, event: object) -> None:  # noqa: N802 - Qt API
        """Resizeevent."""
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._refresh_columns()

    def _refresh_columns(self) -> None:
        """Refresh columns."""
        columns = self._columns_for_width(self.width())
        if columns == self._column_count:
            return
        self._column_count = columns
        for value_label, caption in self._cells:
            self._layout.removeWidget(value_label)
            self._layout.removeWidget(caption)
        for column in range(max(_METRIC_COLUMN_OPTIONS)):
            self._layout.setColumnStretch(column, 0)
        for index, (value_label, caption) in enumerate(self._cells):
            row = (index // columns) * 2
            column = index % columns
            self._layout.addWidget(value_label, row, column)
            self._layout.addWidget(caption, row + 1, column)
        for column in range(columns):
            self._layout.setColumnStretch(column, 1)
        self._layout.activate()

    def _columns_for_width(self, width: int) -> int:
        """Columns for width."""
        if not self._cells:
            return 1
        margins = self._layout.contentsMargins()
        available = max(0, int(width) - margins.left() - margins.right())
        max_columns = min(len(self._cells), max(_METRIC_COLUMN_OPTIONS))
        spacing = max(0, self._layout.horizontalSpacing())
        for columns in _METRIC_COLUMN_OPTIONS:
            if columns > max_columns:
                continue
            required = columns * _METRIC_MIN_WIDTH + (columns - 1) * spacing
            if available >= required:
                return columns
        return 1


class EmbeddedStationRouteGlyph(QWidget):
    """Resource-free route marker for the future extension station."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("extensionStationRouteGlyph")
        self.setFixedSize(96, 64)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")
        self._phase = 0.0
        self._animated = False

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without owning a timer."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the route marker for reduced motion and lifecycle fences."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        theme = theme_spec_for_widget(self)
        bounds = self.rect().adjusted(4, 7, -4, -7)
        panel = QColor(theme.surface_input)
        panel.setAlpha(205)
        border = QColor(theme.info_border)
        border.setAlpha(175)
        painter.setBrush(panel)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 12.0, 12.0)

        center_y = bounds.center().y()
        points = (
            QPointF(bounds.left() + 17.0, center_y),
            QPointF(bounds.center().x(), center_y),
            QPointF(bounds.right() - 17.0, center_y),
        )
        colors = (theme.accent_blue, theme.accent, theme.accent_purple)
        for index, (start, end) in enumerate(zip(points, points[1:], strict=False)):
            rail = QColor(colors[index])
            rail.setAlpha(175 if self._animated else 105)
            painter.setPen(QPen(rail, 1.4))
            painter.drawLine(start, end)

        for index, point in enumerate(points):
            node = QColor(colors[index])
            node.setAlpha(220 if self._animated else 145)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(node)
            painter.drawEllipse(point, 4.0 if index == 1 else 3.0, 4.0 if index == 1 else 3.0)

        if self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            if travel < 0.5:
                position = points[0] * (1.0 - travel * 2.0) + points[1] * (travel * 2.0)
            else:
                position = points[1] * (2.0 - travel * 2.0) + points[2] * (travel * 2.0 - 1.0)
            pulse = QColor(theme.accent_pink)
            pulse.setAlpha(170)
            painter.setBrush(pulse)
            painter.drawEllipse(position, 2.3, 2.3)


class EmbeddedStationOverview(QFrame):
    """Render the application-owned summary and its decorative route marker."""

    def __init__(
        self,
        summary: ExtensionStationSummary,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("extensionStationOverview")
        self.setProperty("role", "stationBand")
        self.setProperty("source", "history")
        self.setAccessibleName("扩展工具站接入概览")
        self.setAccessibleDescription(
            f"当前展示 {summary.capability_count} 个能力槽位，已激活后端 "
            f"{summary.active_backend_count} 个；"
            f"分组计数为 {_group_summary_text(summary)}；真实接入需要目标授权。"
        )
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)

        root = QHBoxLayout(self)
        root.setContentsMargins(11, 10, 11, 10)
        root.setSpacing(12)
        self._route = EmbeddedStationRouteGlyph(self)
        root.addWidget(self._route, alignment=Qt.AlignmentFlag.AlignVCenter)

        content = QVBoxLayout()
        content.setContentsMargins(0, 0, 0, 0)
        content.setSpacing(7)
        header = QHBoxLayout()
        header.setContentsMargins(0, 0, 0, 0)
        title = QLabel("接入概览")
        title.setProperty("role", "section")
        title.setProperty("scope", "station")
        title.setAccessibleName("扩展工具站接入概览标题")
        header.addWidget(title)
        header.addStretch(1)
        state = QLabel("只读规划层")
        state.setObjectName("extensionStationOverviewState")
        state.setProperty("role", "status")
        state.setAccessibleName("扩展工具站当前状态")
        state.setAccessibleDescription("当前没有激活真实 OTA 或调试后端。")
        header.addWidget(state)
        content.addLayout(header)

        metric_items = [
            ("能力槽位", str(summary.capability_count)),
            ("已激活后端", str(summary.active_backend_count)),
            ("当前动作", summary.current_action),
            *(
                (group.label, str(group.capability_count))
                for group in summary.group_summaries
            ),
        ]
        content.addWidget(_ResponsiveMetricGrid(tuple(metric_items)))

        prerequisite = QLabel(summary.prerequisite)
        prerequisite.setProperty("role", "subtle")
        prerequisite.setWordWrap(True)
        prerequisite.setAccessibleName("扩展工具站接入前置条件")
        prerequisite.setAccessibleDescription(prerequisite.text())
        content.addWidget(prerequisite)
        root.addLayout(content, stretch=1)

    def set_frame(self, phase: float, animated: bool) -> None:
        """Fan the existing shell frame to the decorative route marker."""

        self._route.set_frame(phase, animated)

    def stop(self) -> None:
        """Freeze the route marker without changing the read-only summary."""

        self._route.stop()


def build_embedded_station_overview(
    summary: ExtensionStationSummary,
    parent: QWidget | None = None,
) -> EmbeddedStationOverview:
    """Render an application-owned summary without deriving business state."""

    return EmbeddedStationOverview(summary, parent)


def _build_metric_cell(label: str, value: str) -> tuple[QLabel, QLabel]:
    """Build one metric pair while keeping its accessibility contract."""

    value_label = QLabel(value)
    value_label.setObjectName("extensionStationOverviewValue")
    value_label.setSizePolicy(
        QSizePolicy.Policy.Ignored,
        QSizePolicy.Policy.Preferred,
    )
    value_label.setMinimumWidth(0)
    value_label.setWordWrap(True)
    value_label.setAccessibleName(label)
    value_label.setAccessibleDescription(f"{label}：{value}")
    caption = QLabel(label)
    caption.setProperty("role", "subtle")
    caption.setAccessibleName(f"{label}说明")
    return value_label, caption


def _group_summary_text(summary: ExtensionStationSummary) -> str:
    """Format bounded group counts for the accessible station summary."""

    if not summary.group_summaries:
        return "暂无分组摘要"
    return "、".join(
        f"{group.label} {group.capability_count} 个"
        for group in summary.group_summaries
    )


__all__ = [
    "EmbeddedStationOverview",
    "EmbeddedStationRouteGlyph",
    "build_embedded_station_overview",
]
