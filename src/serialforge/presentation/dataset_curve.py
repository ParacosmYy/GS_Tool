"""Qt Widgets-only renderer for one bounded Dataset curve."""

from __future__ import annotations

import math

from ..domain.protocols import DataOrigin
from .curve import CurveSnapshot
from .qt import QColor, QPainter, QPen, QPointF, QRectF, Qt, QTimer, QWidget, Signal
from .theme import theme_spec_for_widget


class DatasetCurveWidget(QWidget):
    """Render an immutable CurveSnapshot without owning application or worker state."""

    snapshot_changed = Signal(object)

    _REFRESH_INTERVAL_MS = 100

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._snapshot = CurveSnapshot()
        self._pending_snapshot: CurveSnapshot | None = None
        self._refresh_timer = QTimer(self)
        self._refresh_timer.setSingleShot(True)
        self._refresh_timer.setInterval(self._REFRESH_INTERVAL_MS)
        self._refresh_timer.timeout.connect(self._flush_pending)
        self._suspended = False
        self._closed = False
        self._empty_message = "未选择数值序列"
        self._phase = 0.0
        self._animated = False
        self.setObjectName("datasetCurve")
        self.setMinimumWidth(260)
        self.setMinimumHeight(160)
        self.setMaximumHeight(210)
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        self.setAccessibleName("Dataset 数值曲线")
        self.setAccessibleDescription("显示当前 Dataset 数值序列的有限时间序列；错误值不绘制。")

    @property
    def snapshot(self) -> CurveSnapshot:
        return self._snapshot

    def set_snapshot(self, snapshot: CurveSnapshot) -> None:
        if not isinstance(snapshot, CurveSnapshot):
            raise TypeError("DatasetCurveWidget 只接受 CurveSnapshot。")
        if self._closed:
            return
        self._pending_snapshot = snapshot
        self.setAccessibleDescription(_snapshot_accessibility(snapshot))
        if not self._suspended and not self._closed and not self._refresh_timer.isActive():
            self._refresh_timer.start()

    def set_suspended(self, suspended: bool) -> None:
        """Pause only curve painting while the owning window is hidden."""

        if self._closed:
            return
        self._suspended = bool(suspended)
        if self._suspended:
            self._refresh_timer.stop()
        elif self._pending_snapshot is not None and not self._refresh_timer.isActive():
            self._refresh_timer.start()

    def set_empty_message(self, message: str) -> None:
        """Set the short static fallback shown when no curve can be drawn."""

        if self._closed:
            return
        if not isinstance(message, str) or not message.strip():
            raise ValueError("curve empty message must be a non-empty string")
        self._empty_message = message.strip()
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame for empty-state decoration only."""

        self._phase = float(phase)
        self._animated = bool(animated)
        if self._snapshot.field_name is None or not self._snapshot.points:
            self.update()

    def stop(self) -> None:
        """Freeze the empty-state signal rail during reduced motion or suspension."""

        self._animated = False
        self.update()

    def shutdown(self) -> None:
        """Stop the renderer permanently before its parent window is destroyed."""

        self._closed = True
        self._suspended = True
        self._refresh_timer.stop()
        self._pending_snapshot = None

    def flush(self) -> None:
        """Apply a pending snapshot immediately for deterministic lifecycle checks."""

        self._flush_pending()

    def _flush_pending(self) -> None:
        if self._suspended or self._closed:
            self._refresh_timer.stop()
            return
        pending = self._pending_snapshot
        self._pending_snapshot = None
        self._refresh_timer.stop()
        if pending is None or pending == self._snapshot:
            return
        self._snapshot = pending
        self.update()
        self.snapshot_changed.emit(self._snapshot)

    def paintEvent(self, event: object) -> None:
        del event
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.fillRect(self.rect(), QColor(theme_spec_for_widget(self).surface_input))
        snapshot = self._snapshot
        if snapshot.field_name is None:
            self._draw_empty_state(painter, self._empty_message, "选择数值序列")
            self._draw_focus_ring(painter)
            return
        if not snapshot.points:
            reason = "无可绘制数值"
            if snapshot.sample_count == 0:
                reason = "等待 Dataset 样本"
            elif snapshot.skipped_points:
                reason = f"无可绘制数值 · {snapshot.skipped_points} 个错误/非数值点"
            title = "等待 Dataset 样本" if snapshot.sample_count == 0 else "没有可绘制数值"
            self._draw_empty_state(painter, reason, title)
            self._draw_focus_ring(painter)
            return
        plot = QRectF(62.0, 22.0, max(1.0, self.width() - 78.0), max(1.0, self.height() - 54.0))
        values = tuple(point.value for point in snapshot.points)
        minimum = min(values)
        maximum = max(values)
        if math.isclose(minimum, maximum):
            padding = max(1.0, abs(minimum) * 0.05)
            minimum -= padding
            maximum += padding
        self._draw_grid(painter, plot, minimum, maximum, snapshot.elapsed)
        self._draw_line(painter, plot, snapshot, minimum, maximum)
        self._draw_focus_ring(painter)

    def _draw_focus_ring(self, painter: QPainter) -> None:
        if not self.hasFocus():
            return
        focus_pen = QPen(QColor(theme_spec_for_widget(self).accent))
        focus_pen.setWidth(2)
        painter.setPen(focus_pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawRoundedRect(
            QRectF(1.0, 1.0, max(1.0, self.width() - 2.0), max(1.0, self.height() - 2.0)),
            7.0,
            7.0,
        )

    def _draw_empty_state(self, painter: QPainter, message: str, title: str) -> None:
        """Render an actionable-looking waiting canvas without owning an action."""

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(12.0, 12.0, -12.0, -12.0)
        if bounds.width() <= 0 or bounds.height() <= 0:
            return
        panel = QColor(theme.surface)
        panel.setAlpha(170)
        border = QColor(theme.border)
        border.setAlpha(180)
        painter.setBrush(panel)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 10.0, 10.0)

        color = QColor(theme.accent_blue if title == "选择数值序列" else theme.accent)
        if title == "没有可绘制数值":
            color = QColor(theme.warning)
        color.setAlpha(205 if self._animated else 135)

        icon_center = QPointF(bounds.left() + 42.0, bounds.center().y())
        icon_panel = QRectF(icon_center.x() - 22.0, icon_center.y() - 22.0, 44.0, 44.0)
        icon_fill = QColor(theme.surface_input)
        icon_fill.setAlpha(215)
        painter.setBrush(icon_fill)
        painter.setPen(QPen(QColor(theme.border), 1.0))
        painter.drawRoundedRect(icon_panel, 12.0, 12.0)
        painter.setPen(QPen(color, 1.2))
        painter.drawEllipse(icon_center, 13.0, 8.0)
        painter.drawEllipse(icon_center, 8.0, 13.0)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(color)
        painter.drawEllipse(icon_center, 3.0, 3.0)

        text_left = bounds.left() + 76.0
        text_width = max(80.0, bounds.width() - 92.0)
        painter.setPen(QPen(QColor(theme.text)))
        painter.drawText(
            QRectF(text_left, bounds.top() + 14.0, text_width, 22.0),
            Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter,
            title,
        )
        painter.setPen(QPen(QColor(theme.text_muted)))
        painter.drawText(
            QRectF(text_left, bounds.top() + 38.0, text_width, max(24.0, bounds.height() - 58.0)),
            Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignTop | Qt.TextFlag.TextWordWrap,
            message,
        )

        rail_left = text_left
        rail_right = bounds.right() - 12.0
        rail_y = bounds.bottom() - 10.0
        if rail_right <= rail_left:
            return
        guide = QColor(theme.border)
        guide.setAlpha(125)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(rail_left, rail_y), QPointF(rail_right, rail_y))
        node_count = 7
        span = rail_right - rail_left
        for index in range(node_count):
            node = QColor(color)
            node.setAlpha(160 if index < 3 else 65)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(node)
            painter.drawEllipse(
                QPointF(rail_left + span * index / (node_count - 1), rail_y),
                2.0,
                2.0,
            )
        if self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(color)
            pulse.setAlpha(180)
            painter.setPen(QPen(pulse, 1.1))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(rail_left + span * travel, rail_y), 4.0, 4.0)

    def _draw_grid(
        self,
        painter: QPainter,
        plot: QRectF,
        minimum: float,
        maximum: float,
        elapsed: float,
    ) -> None:
        theme = theme_spec_for_widget(self)
        grid_color = QColor(theme.border)
        grid_color.setAlpha(150)
        grid_pen = QPen(grid_color)
        grid_pen.setStyle(Qt.PenStyle.DotLine)
        grid_pen.setWidth(1)
        painter.setPen(grid_pen)
        for index in range(5):
            ratio = index / 4
            y = plot.bottom() - ratio * plot.height()
            painter.setPen(grid_pen)
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y))
            value = minimum + ratio * (maximum - minimum)
            axis_color = QColor(theme.text_subtle)
            painter.setPen(QPen(axis_color))
            painter.drawText(
                QRectF(2.0, y - 9.0, 56.0, 18.0),
                Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter,
                _format_number(value),
            )
        painter.setPen(QPen(QColor(theme.text_subtle)))
        x_label = f"0.00s → {elapsed:.2f}s"
        painter.drawText(
            QRectF(plot.left(), plot.bottom() + 5.0, plot.width(), 18.0),
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter,
            x_label,
        )

    def _draw_line(
        self,
        painter: QPainter,
        plot: QRectF,
        snapshot: CurveSnapshot,
        minimum: float,
        maximum: float,
    ) -> None:
        span = max(1e-12, snapshot.elapsed)
        theme = theme_spec_for_widget(self)
        line_color = QColor(
            theme.accent_purple if snapshot.origin is DataOrigin.HISTORICAL else theme.accent
        )
        glow_color = QColor(line_color)
        glow_color.setAlpha(42)
        glow_pen = QPen(glow_color)
        glow_pen.setWidth(7)
        painter.setPen(glow_pen)
        previous: QPointF | None = None
        for point in snapshot.points:
            x = plot.left() + (point.elapsed / span) * plot.width()
            y = plot.bottom() - ((point.value - minimum) / (maximum - minimum)) * plot.height()
            current = QPointF(x, y)
            if previous is not None:
                painter.drawLine(previous, current)
            previous = current
        line_pen = QPen(line_color)
        line_pen.setWidth(2)
        painter.setPen(line_pen)
        previous = None
        for point in snapshot.points:
            x = plot.left() + (point.elapsed / span) * plot.width()
            y = plot.bottom() - ((point.value - minimum) / (maximum - minimum)) * plot.height()
            current = QPointF(x, y)
            if previous is not None:
                painter.drawLine(previous, current)
            painter.drawEllipse(current, 3.0, 3.0)
            previous = current
        painter.setPen(QPen(QColor(theme.text)))
        title = snapshot.field_name
        if snapshot.unit:
            title += f" ({snapshot.unit})"
        if snapshot.origin is DataOrigin.HISTORICAL:
            title += " · 历史"
        else:
            title += " · 实时"
        painter.drawText(QRectF(64.0, 2.0, self.width() - 72.0, 18.0), title)


def _format_number(value: float) -> str:
    if abs(value) >= 1_000 or (abs(value) > 0 and abs(value) < 0.01):
        return f"{value:.3g}"
    return f"{value:.3f}".rstrip("0").rstrip(".")


def _snapshot_accessibility(snapshot: CurveSnapshot) -> str:
    """Describe the latest immutable curve state without announcing it."""

    if snapshot.field_name is None:
        return "未选择数值序列；请加载 Dataset 并选择一个序列。"
    if snapshot.sample_count == 0:
        return f"已选择 {snapshot.field_name}；等待有效 Dataset 样本。"
    if not snapshot.points:
        return (
            f"{snapshot.field_name} 没有可绘制的有限数值；"
            f"已跳过 {snapshot.skipped_points} 个样本。"
        )
    latest = "无" if snapshot.latest_value is None else f"{snapshot.latest_value:.6g}"
    unit = f" {snapshot.unit}" if snapshot.unit else ""
    origin = "历史" if snapshot.origin is DataOrigin.HISTORICAL else "实时"
    return (
        f"{origin} {snapshot.field_name} 曲线；显示 {len(snapshot.points)} 点，"
        f"最新值 {latest}{unit}，跳过 {snapshot.skipped_points} 点，"
        f"丢弃 {snapshot.truncated_points} 点。"
    )


__all__ = ["DatasetCurveWidget"]
