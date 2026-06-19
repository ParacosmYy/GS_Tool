"""Batch 15 测试：scale 弹性反馈 + NavRail hover 缩亮的真实接线。

验证 ``install_scale_press`` / ``install_nav_hover_scale`` 被接到现有按钮，
让 Batch 14 的 scale 动画对用户可见。覆盖：
- install_nav_hover_scale：Enter 事件放大（scale>1）、Leave 回原尺寸。
- settings_panel：Apply 按钮 + accent 色点接了 install_scale_press（build 后 pressed 触发动画）。
- connection_toolbar：connect/disconnect/connect_serial/tcp/udp 按钮接了 scale press。
- command_section：send 按钮接了 scale press。
- app_shell：NavRail 按钮接了 install_scale_press + install_nav_hover_scale。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, QRect
from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.animations.scale import ScaleAnimation
from embeddebug.serial_station.ui.micro_interactions import (
    install_nav_hover_scale,
    install_scale_press,
)


def _make_button(qtbot, w: int = 40, h: int = 40) -> QPushButton:
    btn = QPushButton("x")
    btn.setGeometry(0, 0, w, h)
    qtbot.addWidget(btn)
    return btn


# ── install_nav_hover_scale ───────────────────────────────────────
def test_nav_hover_enter_scales_up(qtbot):
    """Enter 事件 → 放大到 HOVER_SCALE（1.12），scale > 1.0。"""

    from PyQt6.QtWidgets import QApplication

    btn = _make_button(qtbot)
    orig = QRect(btn.geometry())
    install_nav_hover_scale(btn)
    ScaleAnimation._active.clear()
    # 事件过滤器需经 QApplication.notify 派发才触发（直接 btn.event 绕过过滤器）。
    QApplication.sendEvent(btn, QEvent(QEvent.Type.Enter))
    assert len(ScaleAnimation._active) >= 1
    anim = ScaleAnimation._active[0]
    w_ratio = anim.endValue().width() / orig.width()
    h_ratio = anim.endValue().height() / orig.height()
    assert w_ratio > 1.0 and h_ratio > 1.0, "enter should scale up (>1.0)"
    ScaleAnimation._active.clear()


def test_nav_hover_leave_restores_size(qtbot):
    """Leave 事件 → 回弹到原尺寸（width/height == 原）。"""

    from PyQt6.QtWidgets import QApplication

    btn = _make_button(qtbot)
    orig = QRect(btn.geometry())
    install_nav_hover_scale(btn)

    ScaleAnimation._active.clear()
    QApplication.sendEvent(btn, QEvent(QEvent.Type.Enter))
    ScaleAnimation._active.clear()
    QApplication.sendEvent(btn, QEvent(QEvent.Type.Leave))
    assert len(ScaleAnimation._active) >= 1
    anim = ScaleAnimation._active[0]
    assert anim.endValue().width() == orig.width()
    assert anim.endValue().height() == orig.height()
    ScaleAnimation._active.clear()


# ── settings_panel Apply 按钮接线 ─────────────────────────────────
def test_settings_apply_button_has_scale_press(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    panel.build(AppController())
    apply_btn = panel._widget.findChild(QPushButton, "serialStationSettingsApplyButton")
    assert apply_btn is not None
    # pressed 信号 emit 应触发 press_down 动画（间接证明 install_scale_press 已接）。
    ScaleAnimation._active.clear()
    apply_btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1, "Apply button should have scale press"
    ScaleAnimation._active.clear()


# ── connection_toolbar 按钮接线 ───────────────────────────────────
def test_connection_buttons_have_scale_press(qtbot):
    from embeddebug.serial_station.ui.connection_toolbar import build_connection_toolbar
    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow

    window = SerialStationMainWindow()
    build_connection_toolbar(window, window._controller, window)
    ScaleAnimation._active.clear()
    for objname in (
        "serialStationConnectButton",
        "serialStationConnectSerialButton",
        "serialStationDisconnectButton",
    ):
        btn = window.findChild(QPushButton, objname)
        assert btn is not None, f"{objname} missing"
        btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 3, "connect/disconnect buttons should have scale press"
    ScaleAnimation._active.clear()


def test_tcp_udp_connect_buttons_have_scale_press(qtbot):
    from embeddebug.serial_station.ui.connection_toolbar import build_connection_toolbar
    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow

    window = SerialStationMainWindow()
    build_connection_toolbar(window, window._controller, window)
    ScaleAnimation._active.clear()
    for objname in ("serialStationConnectTcpButton", "serialStationConnectUdpButton"):
        btn = window.findChild(QPushButton, objname)
        assert btn is not None, f"{objname} missing"
        btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 2
    ScaleAnimation._active.clear()


# ── command_section send 按钮接线 ─────────────────────────────────
def test_send_button_has_scale_press(qtbot):
    from embeddebug.serial_station.ui.command_section import build_send_row
    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow

    window = SerialStationMainWindow()
    build_send_row(window, window)
    send_btn = window.findChild(QPushButton, "serialStationSendButton")
    assert send_btn is not None
    ScaleAnimation._active.clear()
    send_btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1, "send button should have scale press"
    ScaleAnimation._active.clear()
