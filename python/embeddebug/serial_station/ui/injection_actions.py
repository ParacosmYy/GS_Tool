"""Fake RX injection actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


class InjectionActionHost(Protocol):
    """Minimal main-window surface needed by injection action handlers."""

    def tr(self, text: str) -> str: ...


def inject_received(host: InjectionActionHost) -> None:
    text = host._inject_edit.text()
    if not text:
        set_status_text(host, "RX text is empty")
        # Batch 13: 空 RX 文本 → warning toast。
        _notify(host, "warning", host.tr("注入为空"), host.tr("请输入要注入的 RX 文本"))
        return
    result = host._controller.inject_received_text(text)
    if result.ok:
        set_result_status(
            host,
            result,
            success_text="Received fake bytes",
            failure_prefix="Inject failed",
        )
        return
    set_result_status(host, result, success_text="", failure_prefix="Inject failed")
    # Batch 13: 注入失败 → error toast。
    _notify(host, "error", host.tr("注入失败"), getattr(result, "message", "") or "")


def _notify(host: InjectionActionHost, level: str, title: str, message: str) -> None:
    """触发 toast 通知（Batch 13：RX 注入工作流 → 通知系统）。

    host 可能未实现 _notify（无 AppShell 的单窗口/测试场景），getattr 安全降级。
    """

    notify_fn = getattr(host, "_notify", None)
    if callable(notify_fn):
        try:
            notify_fn(level, title, message)
        except Exception:
            pass
