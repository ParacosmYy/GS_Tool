"""域面板通知 helper（Batch 14）。

域面板（OTA/BLE/RTT/CAN/Automation）通过 AppShell 的 QStackedWidget 接入，
其 widget reparent 后 ``self._widget.window()`` 返回 AppShell。本 helper 提供
``panel_notify``，把面板的反馈意图委托给 AppShell.notify（→ NotificationManager
→ ToastContainer → ToastWidget 渲染 toast），供域面板在关键事件（传输完成、
连接/断开、扫描完成）处调用。

设计要点：
- 只接收 widget（面板持有的顶层 QWidget），用 ``widget.window()`` 解析 AppShell。
- AppShell 未实现 notify（单窗口/测试场景）时静默跳过，不影响面板逻辑。
- 动画/通知失败一律不抛异常（toast 是锦上添花）。

约束：只依赖 PyQt6 QWidget，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget


def panel_notify(
    widget: QWidget | None,
    level: str,
    title: str,
    message: str,
    timeout_ms: int = 4000,
) -> None:
    """把域面板通知委托给 effective 顶层窗口（AppShell）的 toast 系统。

    Args:
        widget: 面板持有的顶层 QWidget（widget.window() 解析 AppShell）。
        level: "info" / "success" / "warning" / "error"。
        title: toast 标题。
        message: toast 描述（可空）。
        timeout_ms: 自动消失毫秒。
    """

    if widget is None:
        return
    try:
        top = widget.window()
    except Exception:
        return
    notify_fn = getattr(top, "notify", None)
    if not callable(notify_fn):
        return
    try:
        notify_fn(level, title, message, timeout_ms=timeout_ms)
    except Exception:
        pass  # toast 是锦上添花，失败不阻塞面板事件处理。
