"""Connection control state helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class ConnectionControlHost(Protocol):
    """Minimal surface needed to update connection command buttons."""

    _connect_button: object
    _connect_serial_button: object
    _connect_tcp_button: object
    _connect_udp_button: object
    _disconnect_button: object


def set_connection_control_state(
    host: ConnectionControlHost, *, connected: bool, has_serial_ports: bool
) -> None:
    host._connect_button.setEnabled(not connected)
    host._connect_serial_button.setEnabled(not connected and has_serial_ports)
    host._connect_tcp_button.setEnabled(not connected)
    host._connect_udp_button.setEnabled(not connected)
    host._disconnect_button.setEnabled(connected)
    # Batch 46 P0-2: 连接成功时对断开按钮做 pop 动画（视觉反馈，
    # 激活 ScaleAnimation.pop，对标 VOFA+ 连接按钮状态变化反馈）。
    if connected:
        try:
            from embeddebug.serial_station.ui.animations.scale import ScaleAnimation

            ScaleAnimation.pop(host._disconnect_button).start()
        except Exception:
            pass  # 动画是锦上添花，失败不阻塞连接逻辑。
        try:
            from embeddebug.serial_station.ui.animations.glow import GlowAnimation

            GlowAnimation.pulse(host._disconnect_button, loops=3).start()
        except Exception:
            pass  # 动画是锦上添花，失败不阻塞连接逻辑。
