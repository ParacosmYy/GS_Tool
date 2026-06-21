"""scale/slide 基础变换 + 按钮接线集成测试（合并自 scale_press/wiring/ui_animations）。"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEvent, QEasingCurve, QRect
from PyQt6.QtWidgets import QApplication, QPushButton, QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.animations import (
    AnimationTokens,
    ScaleAnimation,
    SlideAnimation,
    SlideDirection,
)
from embeddebug.serial_station.ui.command_section import build_send_row
from embeddebug.serial_station.ui.connection_toolbar import build_connection_toolbar
from embeddebug.serial_station.ui.main_window import SerialStationMainWindow
from embeddebug.serial_station.ui.micro_interactions import (
    install_nav_hover_scale,
    install_scale_press,
)
from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel


@pytest.fixture(autouse=True)
def _clean_active():
    ScaleAnimation._active.clear()


def _btn(qtbot, w=100, h=40):
    b = QPushButton("test"); b.setGeometry(0, 0, w, h); qtbot.addWidget(b)
    return b


def _dim_scale(rect, orig):
    return (rect.width() / orig.width(), rect.height() / orig.height())


def test_tokens_contract():
    assert AnimationTokens.DURATION_INSTANT < AnimationTokens.DURATION_FAST
    assert AnimationTokens.DURATION_FAST < AnimationTokens.DURATION_NORMAL
    assert AnimationTokens.DURATION_NORMAL < AnimationTokens.DURATION_SLOW
    assert AnimationTokens.EASE_OUT == QEasingCurve.Type.OutCubic
    assert AnimationTokens.EASE_OUT_BACK == QEasingCurve.Type.OutBack
    assert 0 < AnimationTokens.SCALE_PRESSED < 1.0
    assert AnimationTokens.SCALE_HOVER > 1.0
    assert AnimationTokens.SCALE_NORMAL == 1.0


def test_scale_press_returns_animation(qtbot):
    btn = QPushButton("X"); qtbot.addWidget(btn)
    anim = ScaleAnimation.press(btn)
    assert anim.duration() == AnimationTokens.DURATION_FAST
    assert anim.easingCurve() == AnimationTokens.EASE_OUT_BACK


def test_scale_pop_in(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = ScaleAnimation.pop_in(w)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert isinstance(anim.startValue(), QRect) and isinstance(anim.endValue(), QRect)


def test_scale_bounce(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = ScaleAnimation.bounce(w)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert isinstance(anim.endValue(), QRect)


def test_slide_in_left(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.LEFT, 50)
    assert anim.startValue().x() < anim.endValue().x()


def test_slide_in_right(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.RIGHT, 50)
    assert anim.startValue().x() > anim.endValue().x()


def test_slide_in_up(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.UP, 50)
    assert anim.startValue().y() < anim.endValue().y()


def test_slide_in_down(qtbot):
    w = QWidget(); qtbot.addWidget(w)
    anim = SlideAnimation.slide_in(w, SlideDirection.DOWN, 50)
    assert anim.startValue().y() > anim.endValue().y()


def test_slide_out_hides_widget(qtbot):
    w = QWidget(); w.show(); qtbot.addWidget(w)
    anim = SlideAnimation.slide_out(w, SlideDirection.RIGHT, 50)
    assert anim.duration() == AnimationTokens.DURATION_FAST


def test_slide_direction_enum():
    assert SlideDirection.LEFT.value == "left"
    assert SlideDirection.RIGHT.value == "right"


def test_press_down_end_scale_below_one(qtbot):
    btn = _btn(qtbot); orig = QRect(btn.geometry())
    anim = ScaleAnimation.press_down(btn)
    wr, hr = _dim_scale(anim.endValue(), orig)
    assert wr < 1.0 and hr < 1.0
    assert 0.93 < wr < 0.99 and 0.93 < hr < 0.99


def test_press_down_center_anchored(qtbot):
    btn = _btn(qtbot)
    orig_center = QRect(btn.geometry()).center()
    assert ScaleAnimation.press_down(btn).endValue().center() == orig_center


def test_press_down_uses_ease_out_and_instant_duration(qtbot):
    anim = ScaleAnimation.press_down(_btn(qtbot))
    assert anim.easingCurve().type() == QEasingCurve.Type.OutCubic
    assert anim.duration() == AnimationTokens.DURATION_INSTANT


def test_press_up_end_scale_is_one(qtbot):
    btn = _btn(qtbot); orig = QRect(btn.geometry())
    anim = ScaleAnimation.press_up(btn, orig)
    assert anim.endValue().width() == orig.width()
    assert anim.endValue().height() == orig.height()


def test_press_up_restores_original_geometry(qtbot):
    btn = _btn(qtbot); orig = QRect(btn.geometry())
    assert ScaleAnimation.press_up(btn, orig).endValue().size() == orig.size()


def test_press_up_uses_ease_out_back(qtbot):
    assert ScaleAnimation.press_up(_btn(qtbot)).easingCurve().type() == QEasingCurve.Type.OutBack


def test_press_down_registered_in_active(qtbot):
    anim = ScaleAnimation.press_down(_btn(qtbot))
    assert anim in ScaleAnimation._active
    anim.stop()


def test_press_up_discarded_after_finished(qtbot):
    anim = ScaleAnimation.press_up(_btn(qtbot))
    assert anim in ScaleAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    assert anim not in ScaleAnimation._active


def test_install_scale_press_connects_pressed_and_released(qtbot):
    btn = _btn(qtbot); install_scale_press(btn)
    btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1
    ScaleAnimation._active.clear()
    btn.released.emit()
    assert len(ScaleAnimation._active) >= 1


def test_install_scale_press_pressed_triggers_press_down(qtbot):
    btn = _btn(qtbot); install_scale_press(btn)
    btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1


def test_install_scale_press_released_triggers_press_up(qtbot):
    btn = _btn(qtbot); install_scale_press(btn)
    btn.released.emit()
    assert len(ScaleAnimation._active) >= 1


def test_install_scale_press_full_press_release_cycle(qtbot):
    btn = _btn(qtbot); orig = QRect(btn.geometry()); install_scale_press(btn)
    btn.pressed.emit()
    down = ScaleAnimation._active[0]; down.start()
    qtbot.waitUntil(lambda: down.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    wd, hd = _dim_scale(btn.geometry(), orig)
    assert wd < 1.0 and hd < 1.0
    ScaleAnimation._active.clear()
    btn.released.emit()
    up = ScaleAnimation._active[0]; up.start()
    qtbot.waitUntil(lambda: up.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    assert btn.geometry().width() == orig.width() and btn.geometry().height() == orig.height()


def test_nav_hover_enter_scales_up(qtbot):
    btn = _btn(qtbot, w=40, h=40); orig = QRect(btn.geometry())
    install_nav_hover_scale(btn)
    QApplication.sendEvent(btn, QEvent(QEvent.Type.Enter))
    assert len(ScaleAnimation._active) >= 1
    anim = ScaleAnimation._active[0]
    wr = anim.endValue().width() / orig.width()
    hr = anim.endValue().height() / orig.height()
    assert wr > 1.0 and hr > 1.0


def test_nav_hover_leave_restores_size(qtbot):
    btn = _btn(qtbot, w=40, h=40); orig = QRect(btn.geometry())
    install_nav_hover_scale(btn)
    QApplication.sendEvent(btn, QEvent(QEvent.Type.Enter))
    ScaleAnimation._active.clear()
    QApplication.sendEvent(btn, QEvent(QEvent.Type.Leave))
    assert len(ScaleAnimation._active) >= 1
    anim = ScaleAnimation._active[0]
    assert anim.endValue().width() == orig.width() and anim.endValue().height() == orig.height()


def test_settings_apply_button_has_scale_press(qtbot):
    panel = SettingsPanel(); panel.build(AppController())
    btn = panel._widget.findChild(QPushButton, "serialStationSettingsApplyButton")
    assert btn is not None
    btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1


def test_connection_buttons_have_scale_press(qtbot):
    window = SerialStationMainWindow()
    build_connection_toolbar(window, window._controller, window)
    for name in ("serialStationConnectButton", "serialStationConnectSerialButton", "serialStationDisconnectButton"):
        btn = window.findChild(QPushButton, name)
        assert btn is not None, f"{name} missing"
        btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 3


def test_tcp_udp_connect_buttons_have_scale_press(qtbot):
    window = SerialStationMainWindow()
    build_connection_toolbar(window, window._controller, window)
    for name in ("serialStationConnectTcpButton", "serialStationConnectUdpButton"):
        btn = window.findChild(QPushButton, name)
        assert btn is not None, f"{name} missing"
        btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 2


def test_send_button_has_scale_press(qtbot):
    window = SerialStationMainWindow(); build_send_row(window, window)
    btn = window.findChild(QPushButton, "serialStationSendButton")
    assert btn is not None
    btn.pressed.emit()
    assert len(ScaleAnimation._active) >= 1
