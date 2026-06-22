"""AppController 单元测试 — 串口 controller 持有 + transport 共享。

覆盖：serial_controller 属性、active_transport 初始 None、is_connected 初始 False。
"""

from __future__ import annotations

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.controllers import SerialWorkbenchController


def test_app_controller_creates_serial_controller():
    app = AppController()
    assert isinstance(app.serial_controller, SerialWorkbenchController)


def test_app_controller_active_transport_initial_none():
    app = AppController()
    assert app.active_transport() is None


def test_app_controller_is_connected_initial_false():
    app = AppController()
    assert app.is_connected() is False


def test_app_controller_serial_controller_persistent():
    """多次访问返回同一实例。"""
    app = AppController()
    c1 = app.serial_controller
    c2 = app.serial_controller
    assert c1 is c2


def test_app_controller_multiple_instances_independent():
    """多个 AppController 实例独立。"""
    a1 = AppController()
    a2 = AppController()
    assert a1.serial_controller is not a2.serial_controller
