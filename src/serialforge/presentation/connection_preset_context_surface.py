"""Visible summary surface for the selected connection quick preset.

The surface keeps the existing connection hint label as the accessible
next-step contract and adds a compact, theme-aware preset summary around it.
It consumes only a bounded ``ConnectionPreset`` projection and the shared
shell frame; it never starts a connection or owns a business state source.
"""

from __future__ import annotations

import math

from .connection_presets import (
    BUILTIN_CONNECTION_PRESET_KEYS,
    ConnectionPreset,
)
from .property_refresh import refresh_dynamic_property
from .qt import (
    QColor,
    QFrame,
    QHBoxLayout,
    QLabel,
    QPainter,
    QPen,
    QPointF,
    QRectF,
    QSizePolicy,
    Qt,
    QWidget,
)
from .theme import theme_spec_for_widget


class ConnectionPresetContextSurface(QFrame):
    """Render preset identity and the existing connection next-step hint."""

    _TRANSPORT_LABELS = {
        "uart": "UART",
        "tcp_stream": "TCP Client",
        "tcp_server": "TCP Server",
        "udp_datagram": "UDP",
        "ble_gatt": "BLE GATT",
        "rtt": "J-Link RTT",
    }

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._preset: ConnectionPreset | None = None
        self._source = "none"
        self._phase = 0.0
        self._animated = False
        self.setObjectName("connectionPresetContext")
        self.setFrameShape(QFrame.Shape.NoFrame)
        self.setMinimumHeight(42)
        self.setMaximumHeight(48)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAccessibleName("连接快速配置摘要")
        self.setAccessibleDescription(
            "未选择连接快速配置；选择配置只填入表单，不会自动连接。"
        )

        root = QHBoxLayout(self)
        root.setContentsMargins(12, 3, 10, 10)
        root.setSpacing(8)
        self._preset_text = QLabel("未选择快速配置", self)
        self._preset_text.setObjectName("connectionPresetContextTitle")
        self._preset_text.setProperty("role", "subtle")
        self._preset_text.setSizePolicy(
            QSizePolicy.Policy.Preferred,
            QSizePolicy.Policy.Fixed,
        )
        root.addWidget(self._preset_text)

        self._hint_label = QLabel("状态由用户显式连接/绑定", self)
        self._hint_label.setObjectName("connectionHint")
        self._hint_label.setProperty("role", "muted")
        self._hint_label.setAccessibleName("连接准备提示")
        self._hint_label.setWordWrap(False)
        self._hint_label.setSizePolicy(
            QSizePolicy.Policy.Ignored,
            QSizePolicy.Policy.Fixed,
        )
        root.addWidget(self._hint_label, stretch=1)

    @property
    def hint_label(self) -> QLabel:
        """Expose the existing hint contract to the connection controller."""

        return self._hint_label

    def set_preset(self, preset: ConnectionPreset | None) -> None:
        """Project the selected preset without applying or persisting it."""

        self._preset = preset if isinstance(preset, ConnectionPreset) else None
        if self._preset is None:
            self._source = "none"
            self._preset_text.setText("未选择快速配置")
            description = "未选择连接快速配置；选择配置只填入表单，不会自动连接。"
        else:
            self._source = (
                "custom"
                if self._preset.key not in BUILTIN_CONNECTION_PRESET_KEYS
                else "builtin"
            )
            transport = self._TRANSPORT_LABELS.get(
                self._preset.transport.value,
                self._preset.transport.value,
            )
            source = "自定义" if self._source == "custom" else "内置"
            self._preset_text.setText(
                f"{source} · {self._preset.label} · {transport}"
            )
            description = (
                f"当前{source}快速配置：{self._preset.label}。"
                f"{self._preset.description} 只填入表单，不会自动连接。"
            )
        refresh_dynamic_property(self, "source", self._source)
        refresh_dynamic_property(
            self,
            "state",
            "selected" if self._preset else "empty",
        )
        self.setAccessibleDescription(description)
        self._preset_text.setToolTip(description)
        self.updateGeometry()
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared shell frame without creating a local clock."""

        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze decorative motion during reduced-motion/lifecycle stop."""

        self._animated = False
        self.update()

    def motion_active(self) -> bool:
        """Return whether a selected preset has an animated context rail."""

        return self._preset is not None

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        super().paintEvent(event)
        if self.width() < 140 or self.height() < 28:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.rect()).adjusted(0.5, 0.5, -0.5, -0.5)
        panel = QColor(self._surface_color(theme))
        panel.setAlpha(225)
        border = QColor(self._border_color(theme))
        border.setAlpha(205)
        accent = self._accent_color(theme)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setBrush(panel)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 7.0, 7.0)

        marker = QColor(accent)
        marker.setAlpha(210 if self._preset is not None else 120)
        painter.setPen(QPen(marker, 2.0))
        painter.drawLine(
            QPointF(bounds.left() + 1.5, bounds.top() + 8.0),
            QPointF(bounds.left() + 1.5, bounds.bottom() - 9.0),
        )

        rail_left = bounds.left() + 12.0
        rail_right = bounds.right() - 10.0
        rail_y = bounds.bottom() - 5.0
        guide = QColor(theme.border)
        guide.setAlpha(120)
        painter.setPen(QPen(guide, 1.0))
        painter.drawLine(QPointF(rail_left, rail_y), QPointF(rail_right, rail_y))
        span = max(1.0, rail_right - rail_left)
        for index in range(4):
            node = QColor(accent)
            node.setAlpha(185 if self._preset is not None else 85)
            x = rail_left + span * index / 3.0
            painter.setPen(QPen(node, 1.0))
            painter.setBrush(QColor(theme.surface))
            painter.drawEllipse(QPointF(x, rail_y), 2.4, 2.4)
            painter.setBrush(node)
            painter.drawEllipse(QPointF(x, rail_y), 1.0, 1.0)

        if self._animated and self._preset is not None:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(accent)
            pulse.setAlpha(190)
            pulse_x = rail_left + span * travel
            painter.setPen(QPen(pulse, 1.0))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(QPointF(pulse_x, rail_y), 4.8, 4.8)

    def _accent_color(self, theme) -> QColor:
        """Accent color."""
        if self._source == "custom":
            return QColor(theme.accent_purple)
        if self._source == "builtin":
            return QColor(theme.accent_blue)
        return QColor(theme.text_subtle)

    def _surface_color(self, theme) -> str:
        """Surface color."""
        if self._source == "custom":
            return theme.history_surface
        if self._source == "builtin":
            return theme.info_surface
        return theme.neutral_surface

    def _border_color(self, theme) -> str:
        """Border color."""
        if self._source == "custom":
            return theme.history_border
        if self._source == "builtin":
            return theme.info_border
        return theme.neutral_border


__all__ = ["ConnectionPresetContextSurface"]
