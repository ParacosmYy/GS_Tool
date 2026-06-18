"""Command send and history actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.command_entry_text import apply_command_history_selection
from embeddebug.serial_station.ui.command_history_options import populate_command_history_options
from embeddebug.serial_station.ui.status_messages import set_result_status, set_status_text


class CommandActionHost(Protocol):
    """Minimal main-window surface needed by command action handlers."""

    def tr(self, text: str) -> str: ...

    def _refresh_command_history(self) -> None: ...


def send_text(host: CommandActionHost) -> None:
    text = host._send_edit.text()
    if not text:
        set_status_text(host, "Command is empty")
        # Batch 8: 空命令校验失败 → 抖动 send_edit 反馈（激活 ShakeAnimation）。
        _shake_widget(host._send_edit)
        # Batch 13: 空命令 → warning toast。
        _notify(host, "warning", host.tr("命令为空"), host.tr("请输入要发送的命令"))
        return
    result = host._controller.send_text_result(text)
    if result.ok:
        refresh_command_history(host)
        set_result_status(host, result, success_text="Command sent", failure_prefix="Send failed")
        return
    set_result_status(host, result, success_text="", failure_prefix="Send failed")
    # Batch 13: 发送失败 → error toast。
    _notify(host, "error", host.tr("发送失败"), getattr(result, "message", "") or "")


def _notify(host: CommandActionHost, level: str, title: str, message: str) -> None:
    """触发 toast 通知（Batch 13：命令发送工作流 → 通知系统）。

    host 可能未实现 _notify（无 AppShell 的单窗口/测试场景），getattr 安全降级。
    """

    notify_fn = getattr(host, "_notify", None)
    if callable(notify_fn):
        try:
            notify_fn(level, title, message)
        except Exception:
            pass


def _shake_widget(widget: object) -> None:
    """对控件触发左右抖动动画（校验失败反馈）。

    Batch 8：激活 ShakeAnimation 死代码，输入校验失败时控件抖动。
    动画失败不阻塞（抖动是锦上添花）。
    """

    try:
        from embeddebug.serial_station.ui.animations.shake import ShakeAnimation

        ShakeAnimation.shake(widget).start()
    except Exception:
        pass


def refresh_command_history(host: CommandActionHost) -> None:
    history = host._controller.command_history
    populate_command_history_options(host._command_history_combo, history)


def select_command_history(host: CommandActionHost, text: str) -> None:
    apply_command_history_selection(host._send_edit, text)
