"""域面板通知接入测试（OTA/BLE/RTT → panel_notify → toast）。

覆盖：
1. panel_notify helper 行为：解析顶层 window、无 notify 降级、None widget 安全。
2. panel_notify 委托到 window.notify（端到端）。
3. OTA/BLE/RTT 三面板源码级接入断言（panel_notify 在关键事件路径调用）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.panels._notify import panel_notify


# ── panel_notify helper 行为 ───────────────────────────────────────
def test_panel_notify_delegates_to_window_notify(qtbot):
    """panel_notify 应委托给 widget.window().notify。"""

    widget = QWidget()
    qtbot.addWidget(widget)
    calls: list = []
    # 模拟 AppShell：在 widget 上挂 notify（实际场景 widget.window() 返回 AppShell）。
    widget.notify = lambda level, title, message, timeout_ms=4000: calls.append(  # type: ignore[attr-defined]
        (level, title, message, timeout_ms)
    )
    panel_notify(widget, "success", "t", "m")
    assert calls == [("success", "t", "m", 4000)]


def test_panel_notify_without_notify_no_crash(qtbot):
    """window 无 notify 方法时 panel_notify 静默跳过（单窗口/测试场景）。"""

    widget = QWidget()
    qtbot.addWidget(widget)
    panel_notify(widget, "info", "t", "m")  # 不应抛异常


def test_panel_notify_none_widget_no_crash():
    """widget 为 None 时 panel_notify 安全返回。"""

    panel_notify(None, "info", "t", "m")  # 不应抛异常


def test_panel_notify_passes_level_title_message(qtbot):
    """各 level 应透传到 window.notify。"""

    widget = QWidget()
    qtbot.addWidget(widget)
    captured: list = []
    widget.notify = lambda level, title, message, timeout_ms=4000: captured.append(  # type: ignore[attr-defined]
        level
    )
    for lvl in ("info", "success", "warning", "error"):
        panel_notify(widget, lvl, "t", "m")
    assert captured == ["info", "success", "warning", "error"]


def test_panel_notify_custom_timeout(qtbot):
    """自定义 timeout_ms 应透传。"""

    widget = QWidget()
    qtbot.addWidget(widget)
    captured: list = []
    widget.notify = lambda level, title, message, timeout_ms=4000: captured.append(  # type: ignore[attr-defined]
        timeout_ms
    )
    panel_notify(widget, "info", "t", "m", timeout_ms=7000)
    assert captured == [7000]


# ── OTA 面板接入断言 ───────────────────────────────────────────────
def test_ota_panel_finished_calls_panel_notify():
    """OTA _on_finished 应在成功/失败路径调 panel_notify（传输完成 toast）。"""

    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

    src = inspect.getsource(OtaPanel._on_finished)
    assert "panel_notify" in src
    assert "success" in src  # 成功
    assert "error" in src    # 失败


def test_ota_panel_start_errors_notify():
    """OTA _start_transfer 未连接/无效固件应 warning toast。"""

    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

    src = inspect.getsource(OtaPanel._start_transfer)
    assert "panel_notify" in src
    assert "warning" in src


# ── BLE 面板接入断言 ───────────────────────────────────────────────
def test_ble_panel_connect_calls_panel_notify():
    """BLE _connect 应在连接成功/失败/断开/未扫描路径调 panel_notify。"""

    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel

    src = inspect.getsource(BlePanel._connect)
    assert "panel_notify" in src
    assert "success" in src  # 连接成功
    assert "error" in src    # 连接失败
    assert "info" in src     # 断开


# ── RTT 面板接入断言 ───────────────────────────────────────────────
def test_rtt_panel_start_stop_call_panel_notify():
    """RTT _start/_stop 应调 panel_notify（串口 success / 演示 info / 停止 info）。"""

    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    start_src = inspect.getsource(RttPanel._start)
    stop_src = inspect.getsource(RttPanel._stop)
    assert "panel_notify" in start_src
    assert "panel_notify" in stop_src
    assert "success" in start_src  # 串口运行
    assert "info" in start_src     # 演示模式


# ── panel_notify helper 存在性 ─────────────────────────────────────
def test_panel_notify_module_exists():
    """panels/_notify.py 应暴露 panel_notify 函数。"""

    from embeddebug.serial_station.ui.panels import _notify

    assert hasattr(_notify, "panel_notify")
    assert callable(_notify.panel_notify)


# ── CAN 面板接入断言 ───────────────────────────────────────────────
def test_can_panel_send_calls_panel_notify():
    """CAN _send 应在未连接/失败路径调 panel_notify（warning/error）。"""

    from embeddebug.serial_station.ui.panels.can_panel import CanPanel

    src = inspect.getsource(CanPanel._send)
    assert "panel_notify" in src
    assert "warning" in src  # 未连接
    assert "error" in src    # 发送失败


# ── Automation 面板接入断言 ────────────────────────────────────────
def test_automation_panel_set_active_calls_panel_notify():
    """Automation _set_active 应在启用/停止路径调 panel_notify（success/info）。"""

    from embeddebug.serial_station.ui.panels.automation_panel import AutomationPanel

    src = inspect.getsource(AutomationPanel._set_active)
    assert "panel_notify" in src
    assert "success" in src  # 启用监听
    assert "info" in src     # 停止监听
