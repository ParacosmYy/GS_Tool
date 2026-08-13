"""Presentation-only summary for the protocol configuration editor.

The protocol controls and the protocol status surface remain the source of
truth. This label only renders their current visible values as a compact
"draft or applied" context, plus a decorative rail driven by the shared shell
motion frame. It owns no parser gate, worker, or timer.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

from .property_refresh import refresh_dynamic_property
from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget

_KNOWN_STATES = frozenset({"active", "waiting", "draft", "blocked", "history", "idle"})
_MOVING_STATES = frozenset({"active", "waiting", "draft"})
_NODE_COUNT = 4


@dataclass(frozen=True, slots=True)
class ProtocolConfigContextProjection:
    """Small immutable presentation contract for the visible editor values."""

    framing: str = "Raw"
    checksum: str = "无"
    max_frame_bytes: int = 4_096
    state: str = "waiting"

    def __post_init__(self) -> None:
        framing = _clip_label(self.framing, "Raw")
        checksum = _clip_label(self.checksum, "无")
        max_frame_bytes = _bounded_frame_bytes(self.max_frame_bytes)
        state = self.state if self.state in _KNOWN_STATES else "waiting"
        object.__setattr__(self, "framing", framing)
        object.__setattr__(self, "checksum", checksum)
        object.__setattr__(self, "max_frame_bytes", max_frame_bytes)
        object.__setattr__(self, "state", state)


def project_protocol_config(
    framing: object,
    checksum: object,
    max_frame_bytes: object,
    state: object,
) -> ProtocolConfigContextProjection:
    """Project native editor values without re-evaluating parser eligibility."""

    normalized_state = state if isinstance(state, str) else "waiting"
    return ProtocolConfigContextProjection(
        framing=framing if isinstance(framing, str) else str(framing),
        checksum=checksum if isinstance(checksum, str) else str(checksum),
        max_frame_bytes=max_frame_bytes,
        state=normalized_state,
    )


class ProtocolConfigContextSurface(QLabel):
    """Show configuration intent and lifecycle state in one compact surface."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._projection = ProtocolConfigContextProjection()
        self._phase = 0.0
        self._animated = False
        self.setObjectName("protocolConfigContext")
        self.setProperty("role", "subtle")
        self.setMinimumWidth(190)
        self.setMaximumWidth(520)
        self.setMinimumHeight(30)
        self.setWordWrap(False)
        self.setContentsMargins(8, 2, 8, 10)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("协议配置摘要")
        self._apply_projection()

    def set_projection(self, projection: ProtocolConfigContextProjection) -> None:
        """Consume a presentation projection and refresh semantic properties."""

        if not isinstance(projection, ProtocolConfigContextProjection):
            return
        if self._projection == projection:
            return
        self._projection = projection
        self._apply_projection()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a local clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the decorative rail during reduced motion or suspension."""

        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if self.width() < 120 or self.height() < 22:
            return
        theme = theme_spec_for_widget(self)
        content_rect = QRectF(self.contentsRect())
        band_top = content_rect.bottom() + 1.0
        band_bottom = float(self.rect().bottom()) - 1.0
        band_height = band_bottom - band_top
        if band_height < 5.0:
            return
        bounds = QRectF(content_rect.left(), band_top, content_rect.width(), band_height)
        bounds = bounds.adjusted(10.0, 0.0, -10.0, 0.0)
        left = bounds.left()
        right = bounds.right()
        baseline_y = bounds.center().y()
        span = max(1.0, right - left)
        color = self._state_color(theme)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        guide = QColor(theme.border)
        guide.setAlpha(110)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, baseline_y), QPointF(right, baseline_y))

        active_nodes = self._active_node_count()
        gap = span / (_NODE_COUNT - 1)
        for index in range(_NODE_COUNT):
            x = left + gap * index
            node = QColor(color)
            node.setAlpha(60 if index >= active_nodes else 185)
            painter.setPen(QPen(node, 1.0))
            painter.setBrush(QColor(theme.surface_input))
            painter.drawEllipse(QPointF(x, baseline_y), 2.5, 2.5)
            if index < active_nodes:
                painter.setPen(Qt.PenStyle.NoPen)
                painter.setBrush(node)
                painter.drawEllipse(QPointF(x, baseline_y), 1.15, 1.15)

        if self._animated and self._projection.state in _MOVING_STATES:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            spark = QColor(color)
            spark.setAlpha(145 + int(travel * 80))
            painter.setPen(QPen(spark, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(left + span * travel, baseline_y), 4.2, 4.2)

    def _apply_projection(self) -> None:
        """Apply projection."""
        refresh_dynamic_property(self, "state", self._projection.state)
        description = self._description()
        self.setText(self._summary_text())
        self.setAccessibleDescription(description)
        self.setToolTip(description)
        self.update()

    def _summary_text(self) -> str:
        """Summary text."""
        stage = {
            "active": "已应用 · 活跃",
            "waiting": "已应用 · 等待接收",
            "draft": "草稿 · 待应用",
            "blocked": "暂不可用",
            "history": "历史解析",
            "idle": "未启用",
        }.get(self._projection.state, "已应用")
        return (
            f"{stage} · {_compact_label(self._projection.framing)} · "
            f"{_compact_label(self._projection.checksum)} · "
            f"max {self._projection.max_frame_bytes:,} B"
        )

    def _description(self) -> str:
        """Description."""
        return (
            f"协议配置状态：{self._summary_text()}。"
            "配置值来自当前编辑控件；应用按钮负责真正更新解析状态。"
        )

    def _active_node_count(self) -> int:
        """Active node count."""
        return {
            "active": _NODE_COUNT,
            "waiting": 3,
            "draft": 2,
            "history": 3,
            "blocked": 1,
            "idle": 1,
        }.get(self._projection.state, 1)

    def _state_color(self, theme) -> QColor:
        """State color."""
        value = {
            "active": theme.success,
            "waiting": theme.accent_blue,
            "draft": theme.accent_purple,
            "history": theme.accent_purple,
            "blocked": theme.text_subtle,
            "idle": theme.text_subtle,
        }.get(self._projection.state, theme.text_subtle)
        color = QColor(value)
        color.setAlpha(210)
        return color


def _bounded_frame_bytes(value: object) -> int:
    """Bounded frame bytes."""
    try:
        return max(1, min(65_536, int(value)))
    except (TypeError, ValueError, OverflowError):
        return 4_096


def _clip_label(value: object, fallback: str) -> str:
    """Clip label."""
    text = value if isinstance(value, str) else str(value)
    text = " ".join(text.split())
    return (text or fallback)[:48]


def _compact_label(value: str) -> str:
    """Compact label."""
    return {
        "Line (LF/CRLF)": "Line",
        "Length prefix": "Length",
        "MAVLink v1/v2 · Stream": "MAVLink stream",
        "Modbus RTU · Timed": "Modbus RTU timed",
        "NMEA 0183 XOR (*HH)": "NMEA XOR",
    }.get(value, value)


__all__ = [
    "ProtocolConfigContextProjection",
    "ProtocolConfigContextSurface",
    "project_protocol_config",
]
