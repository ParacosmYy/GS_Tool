"""Batch 13 测试（分体 2）：命令发送 + RX 注入工作流 → 通知系统。

从 test_connection_notify_batch13.py 拆出，满足 250 行测试文件预算。
覆盖 command_actions.send_text 与 injection_actions.inject_received 的
空输入（warning）+ 失败（error）→ toast 路径。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

from embeddebug.serial_station.ui import command_actions, injection_actions


class _SendHost:
    """模拟 CommandActionHost，捕获 _notify 调用。"""

    def __init__(self, text="", send_ok=True) -> None:
        self._send_edit = MagicMock()
        self._send_edit.text.return_value = text
        self._controller = MagicMock()
        self._controller.send_text_result.return_value = MagicMock(
            ok=send_ok, message="" if send_ok else "transport closed"
        )
        self._command_history_combo = MagicMock()
        self._status_label = MagicMock()
        self.notify_calls: list = []

    def tr(self, text: str) -> str:
        return text

    def _refresh_command_history(self) -> None:
        pass

    def _notify(self, level, title, message, timeout_ms=3000):
        self.notify_calls.append((level, title, message))


def test_send_empty_command_notifies_warning(qtbot):
    """空命令应 warning toast（除已有抖动反馈外，Batch 13 增加通知）。"""

    host = _SendHost(text="")
    command_actions.send_text(host)
    levels = [c[0] for c in host.notify_calls]
    assert "warning" in levels


def test_send_failure_notifies_error(qtbot):
    """发送失败（transport 错误）应 error toast。"""

    host = _SendHost(text="ATI", send_ok=False)
    command_actions.send_text(host)
    levels = [c[0] for c in host.notify_calls]
    assert "error" in levels
    # error 消息应含 controller 返回的原因。
    error_msgs = [c[2] for c in host.notify_calls if c[0] == "error"]
    assert any("transport closed" in m for m in error_msgs)


def test_send_success_no_notify(qtbot):
    """发送成功不应弹 toast（成功路径靠状态标签反馈即可，避免 toast 噪音）。"""

    host = _SendHost(text="ATI", send_ok=True)
    command_actions.send_text(host)
    assert host.notify_calls == []


def test_command_actions_has_notify_helper():
    """command_actions 应含 _notify helper（Batch 13）。"""

    src = inspect.getsource(command_actions)
    assert "_notify" in src


# ── RX 注入工作流 → 通知 ───────────────────────────────────────────
class _InjectHost:
    """模拟 InjectionActionHost，捕获 _notify 调用。"""

    def __init__(self, text="", inject_ok=True) -> None:
        self._inject_edit = MagicMock()
        self._inject_edit.text.return_value = text
        self._controller = MagicMock()
        self._controller.inject_received_text.return_value = MagicMock(
            ok=inject_ok, message="" if inject_ok else "not fake transport"
        )
        self._status_label = MagicMock()
        self.notify_calls: list = []

    def tr(self, text: str) -> str:
        return text

    def _notify(self, level, title, message, timeout_ms=3000):
        self.notify_calls.append((level, title, message))


def test_inject_empty_text_notifies_warning(qtbot):
    """空 RX 文本应 warning toast。"""

    host = _InjectHost(text="")
    injection_actions.inject_received(host)
    levels = [c[0] for c in host.notify_calls]
    assert "warning" in levels


def test_inject_failure_notifies_error(qtbot):
    """注入失败（非 fake transport）应 error toast。"""

    host = _InjectHost(text="OK", inject_ok=False)
    injection_actions.inject_received(host)
    levels = [c[0] for c in host.notify_calls]
    assert "error" in levels


def test_injection_actions_has_notify_helper():
    """injection_actions 应含 _notify helper（Batch 13）。"""

    src = inspect.getsource(injection_actions)
    assert "_notify" in src
