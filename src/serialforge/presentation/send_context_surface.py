"""Presentation-only send-payload context and signal rail.

The native send controls remain the source of truth for editing and wire
submission. This module only turns their current form values into an immutable
visual projection and paints a compact explanation beside the send status. It
owns no transport state, validation policy, or timer.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

from ..domain.models import MAX_COMMAND_PAYLOAD_BYTES, CommandMode
from .property_refresh import refresh_dynamic_property
from .qt import QColor, QLabel, QPainter, QPen, QPointF, QRectF, Qt, QWidget
from .theme import theme_spec_for_widget

MAX_CONTEXT_BYTES = MAX_COMMAND_PAYLOAD_BYTES + 2
_BAR_COUNT = 10


@dataclass(frozen=True, slots=True)
class SendContextProjection:
    """Small presentation contract for the current send form."""

    mode: str = "text"
    payload_bytes: int = 0
    wire_bytes: int = 0
    append_newline: bool = False
    valid: bool = True
    empty: bool = True
    error: str = ""

    def __post_init__(self) -> None:
        mode = self.mode if self.mode in {"text", "hex"} else "text"
        payload_bytes = _bounded_int(self.payload_bytes)
        wire_bytes = _bounded_int(self.wire_bytes)
        valid = bool(self.valid)
        empty = bool(self.empty) or payload_bytes == 0
        if not valid:
            empty = False
        object.__setattr__(self, "mode", mode)
        object.__setattr__(self, "payload_bytes", payload_bytes)
        object.__setattr__(self, "wire_bytes", max(payload_bytes, wire_bytes))
        object.__setattr__(self, "append_newline", bool(self.append_newline))
        object.__setattr__(self, "valid", valid)
        object.__setattr__(self, "empty", empty)
        object.__setattr__(self, "error", _clip_error(self.error))


def project_send_form(
    text: object,
    mode: object,
    append_newline: object,
) -> SendContextProjection:
    """Project native form values without changing the send path semantics."""

    value = text if isinstance(text, str) else str(text)
    normalized_mode = getattr(mode, "value", mode)
    mode_value = str(normalized_mode).lower()
    if mode_value not in {CommandMode.TEXT.value, CommandMode.HEX.value}:
        mode_value = CommandMode.TEXT.value
    if not value:
        return SendContextProjection(
            mode=mode_value,
            append_newline=append_newline,
        )

    if mode_value == CommandMode.HEX.value:
        try:
            payload = bytes.fromhex("".join(value.split()))
        except ValueError:
            return SendContextProjection(
                mode=mode_value,
                append_newline=append_newline,
                valid=False,
                error="Hex 内容需要成对的十六进制字符。",
            )
    else:
        payload = value.encode("utf-8")
    newline = bool(append_newline)
    if len(payload) > MAX_COMMAND_PAYLOAD_BYTES:
        return SendContextProjection(
            mode=mode_value,
            payload_bytes=MAX_COMMAND_PAYLOAD_BYTES,
            wire_bytes=MAX_COMMAND_PAYLOAD_BYTES + (2 if newline else 0),
            append_newline=newline,
            valid=False,
            error=(
                f"payload 超过 {MAX_COMMAND_PAYLOAD_BYTES // 1024} KiB 上限，"
                "请缩短内容。"
            ),
        )
    payload_size = len(payload)
    return SendContextProjection(
        mode=mode_value,
        payload_bytes=payload_size,
        wire_bytes=min(MAX_CONTEXT_BYTES, payload_size + (2 if newline else 0)),
        append_newline=newline,
        empty=not payload,
    )


class SendContextSurface(QLabel):
    """Keep a short send summary readable while adding a bounded signal rail."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._projection: SendContextProjection | None = None
        self._phase = 0.0
        self._animated = False
        self.setMinimumWidth(190)
        self.setMaximumWidth(300)
        self.setMinimumHeight(30)
        self.setContentsMargins(7, 2, 7, 10)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("当前发送内容摘要")
        self.set_projection(self._projection)

    def set_projection(self, projection: SendContextProjection) -> None:
        """Consume the current form projection and refresh its semantic state."""

        if not isinstance(projection, SendContextProjection):
            return
        if self._projection == projection:
            return
        self._projection = projection
        refresh_dynamic_property(self, "state", self._state())
        self.setText(self._summary_text())
        description = self._description()
        self.setAccessibleDescription(description)
        self.setToolTip(description)
        self.update()

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
        projection = self._projection
        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(8.0, 0.0, -8.0, -3.0)
        if bounds.width() <= 0 or bounds.height() <= 0:
            return
        left = bounds.left()
        right = bounds.right()
        baseline = bounds.bottom()
        span = max(1.0, right - left)

        if not projection.valid:
            color = QColor(theme.error)
        elif projection.empty:
            color = QColor(theme.text_subtle)
        elif projection.mode == CommandMode.HEX.value:
            color = QColor(theme.accent_purple)
        else:
            color = QColor(theme.accent_blue)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        guide = QColor(theme.border)
        guide.setAlpha(125)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(left, baseline), QPointF(right, baseline))

        active_bars = self._active_bars(projection)
        slot = span / _BAR_COUNT
        for index in range(_BAR_COUNT):
            bar = QColor(color)
            bar.setAlpha(205 if index < active_bars else 60)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(bar)
            height = 3.0 if index < active_bars else 1.5
            painter.drawRoundedRect(
                QRectF(
                    left + index * slot + 1.0,
                    baseline - height,
                    max(2.0, slot - 2.0),
                    height,
                ),
                1.0,
                1.0,
            )

        if projection.valid and not projection.empty and self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse_x = left + span * travel
            pulse = QColor(theme.accent)
            pulse.setAlpha(135 + int(travel * 75))
            painter.setPen(QPen(pulse, 1.2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, baseline - 2.0), 3.2, 3.2)

    def _state(self) -> str:
        """State."""
        if not self._projection.valid:
            return "invalid"
        return "empty" if self._projection.empty else "ready"

    @staticmethod
    def _active_bars(projection: SendContextProjection) -> int:
        """Active bars."""
        if not projection.valid or projection.empty:
            return 0
        ratio = min(1.0, projection.wire_bytes / 64.0)
        return max(1, min(_BAR_COUNT, math.ceil(ratio * _BAR_COUNT)))

    def _summary_text(self) -> str:
        """Summary text."""
        projection = self._projection
        mode_label = "HEX" if projection.mode == CommandMode.HEX.value else "UTF-8"
        if not projection.valid:
            return f"{mode_label} · 格式待修正"
        if projection.empty:
            return f"{mode_label} · 等待输入"
        newline = "CRLF" if projection.append_newline else "无换行"
        return (
            f"{mode_label} · {projection.payload_bytes} B · "
            f"wire {projection.wire_bytes} B · {newline}"
        )

    def _description(self) -> str:
        """Description."""
        projection = self._projection
        mode_label = "Hex" if projection.mode == CommandMode.HEX.value else "UTF-8 文本"
        if not projection.valid:
            return f"{mode_label}格式无效。{projection.error or '请修正后再发送。'}"
        if projection.empty:
            return f"当前为 {mode_label} 模式，等待输入内容。可选追加 CRLF。"
        newline = "已追加 CRLF" if projection.append_newline else "未追加换行"
        return (
            f"当前为 {mode_label} 模式，payload {projection.payload_bytes} B，"
            f"wire payload {projection.wire_bytes} B，{newline}。"
        )


def _bounded_int(value: object) -> int:
    """Bounded int."""
    if isinstance(value, bool):
        return 0
    try:
        return max(0, min(MAX_CONTEXT_BYTES, int(value)))
    except (TypeError, ValueError, OverflowError):
        return 0


def _clip_error(value: object) -> str:
    """Clip error."""
    text = value if isinstance(value, str) else str(value)
    return text[:256]


__all__ = ["SendContextProjection", "SendContextSurface", "project_send_form"]
