"""SerialPanel 包装契约单元测试 — 把 SerialStationMainWindow 接入 AppShell。

SerialPanel 是 AppShell 的串口模式 ModePanel：包装 SerialStationMainWindow，
取其 centralWidget 嵌入 QStackedWidget，保留 window 实例作为 owner。

覆盖：
- 构造：window 属性初始为 None（build 前未实例化底层窗口）。
- build：返回 QWidget wrapper，objectName=serialStationSerialPanel（QSS 契约）。
- build 后 window 属性暴露 SerialStationMainWindow 实例（供命令面板访问 action 委托）。
- build 后 wrapper 非空且有 layout（内容区已挂入）。
- on_enter/on_leave：空实现，不抛异常（MVP 无额外动作）。
- 重复 build：第二次 build 创建新 window 实例（window 属性更新）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.main_window import SerialStationMainWindow
from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel


def _make_app_controller() -> AppController:
    """构造共享 AppController（SerialStationMainWindow 需要）。"""

    return AppController()


def test_constructor_window_is_none_initially():
    """构造后 window 属性为 None（build 前未实例化底层窗口）。"""

    panel = SerialPanel()
    assert panel.window is None


def test_build_returns_qwidget_wrapper(qtbot):
    """build 返回 QWidget wrapper（嵌入 QStackedWidget 用）。"""

    panel = SerialPanel()
    widget = panel.build(_make_app_controller())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_build_wrapper_objectname_contract(qtbot):
    """wrapper objectName = serialStationSerialPanel（QSS 契约 + findChild 可达）。"""

    panel = SerialPanel()
    widget = panel.build(_make_app_controller())
    qtbot.addWidget(widget)
    assert widget.objectName() == "serialStationSerialPanel"


def test_build_exposes_window(qtbot):
    """build 后 window 属性暴露 SerialStationMainWindow 实例（供命令面板/action 委托）。"""

    panel = SerialPanel()
    panel.build(_make_app_controller())
    assert isinstance(panel.window, SerialStationMainWindow)


def test_build_wrapper_has_layout(qtbot):
    """wrapper 有 layout（centralWidget 内容区已挂入）。"""

    panel = SerialPanel()
    widget = panel.build(_make_app_controller())
    qtbot.addWidget(widget)
    assert widget.layout() is not None
    # layout 至少含 1 个 item（centralWidget）。
    assert widget.layout().count() >= 1


def test_on_enter_is_noop(qtbot):
    """on_enter 空实现，不抛异常（MVP 无额外动作）。"""

    panel = SerialPanel()
    panel.build(_make_app_controller())
    # 不应抛异常。
    panel.on_enter()


def test_on_leave_is_noop(qtbot):
    """on_leave 空实现，保留连接与日志（MVP 无额外动作）。"""

    panel = SerialPanel()
    panel.build(_make_app_controller())
    # 不应抛异常。
    panel.on_leave()


def test_on_enter_leave_works_without_build():
    """on_enter/on_leave 在 build 前也可安全调用（防御性空实现）。"""

    panel = SerialPanel()
    panel.on_enter()
    panel.on_leave()


def test_repeated_build_creates_new_window(qtbot):
    """第二次 build 创建新 window 实例（window 属性更新到新实例）。"""

    panel = SerialPanel()
    panel.build(_make_app_controller())
    first_window = panel.window
    panel.build(_make_app_controller())
    second_window = panel.window
    assert first_window is not second_window
    assert isinstance(second_window, SerialStationMainWindow)
