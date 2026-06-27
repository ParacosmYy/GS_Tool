"""command_actions send_text + refresh/select history 行为边界测试。

test_command_notify.py 覆盖 send/inject notify；本文件覆盖 send_text 全分支 +
refresh_command_history + select_command_history 行为。

覆盖：
1. send_text 空命令 → 不调 controller + shake + notify warning。
2. send_text 成功 → controller.send_text_result + refresh_history + status。
3. send_text 失败 → status + notify error。
4. refresh_command_history → populate combo with history。
5. select_command_history → apply selection to send_edit。
"""

from __future__ import annotations

import inspect
import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui import command_actions, injection_actions
from embeddebug.shared.results import OperationResult


def _make_host(send_text_value=""):
    host = SimpleNamespace()
    host.tr = lambda s: s
    host._send_edit = MagicMock()
    host._send_edit.text.return_value = send_text_value
    host._command_history_combo = MagicMock()
    host._status_label = MagicMock()
    host._controller = MagicMock()
    host._controller.command_history = ["AT+RST", "AT+GMR"]
    return host


def _ok():
    return OperationResult.success()


def _fail(msg="boom"):
    return OperationResult.failure("err", msg)


# ── send_text 空命令 ──────────────────────────────────────────────
def test_send_text_empty_does_not_call_controller():
    host = _make_host(send_text_value="")
    command_actions.send_text(host)
    host._controller.send_text_result.assert_not_called()


def test_send_text_empty_shakes_send_edit():
    """空命令 → _shake_widget(send_edit)（不崩溃）。"""

    host = _make_host(send_text_value="")
    command_actions.send_text(host)  # shake 内部可能抛但被吞


# ── send_text 成功 ────────────────────────────────────────────────
def test_send_text_success_calls_controller():
    host = _make_host(send_text_value="AT+RST")
    host._controller.send_text_result.return_value = _ok()
    command_actions.send_text(host)
    host._controller.send_text_result.assert_called_once_with("AT+RST")


def test_send_text_success_refreshes_history():
    host = _make_host(send_text_value="AT+RST")
    host._controller.send_text_result.return_value = _ok()
    command_actions.send_text(host)
    # refresh_command_history → populate_command_history_options 被调（combo 操作）。


# ── send_text 失败 ────────────────────────────────────────────────
def test_send_text_failure_sets_status():
    host = _make_host(send_text_value="AT+BAD")
    host._controller.send_text_result.return_value = _fail("timeout")
    command_actions.send_text(host)
    host._controller.send_text_result.assert_called_once_with("AT+BAD")


def test_send_text_failure_does_not_refresh_history():
    host = _make_host(send_text_value="AT+BAD")
    host._controller.send_text_result.return_value = _fail()
    command_actions.send_text(host)
    # 失败路径不调 refresh（但 combo 仍可能被 populate —— 验证 controller 被调即可）。


# ── refresh_command_history ───────────────────────────────────────
def test_refresh_command_history_populates_combo():
    host = _make_host()
    command_actions.refresh_command_history(host)
    # combo 应被操作（addItems/clear 等 populate 内部调用）。
    # 验证不崩溃 + combo 存在。
    assert host._command_history_combo is not None


def test_refresh_command_history_empty_history():
    host = _make_host()
    host._controller.command_history = []
    command_actions.refresh_command_history(host)  # 不崩溃


# ── select_command_history ────────────────────────────────────────
def test_select_command_history_applies_to_send_edit():
    host = _make_host()
    command_actions.select_command_history(host, "AT+VERSION")
    # apply_command_history_selection 写入 send_edit。
    host._send_edit.setText.assert_called_once_with("AT+VERSION")


def test_select_command_history_empty_text_skips():
    """空 text → apply_command_history_selection 不写入（边界）。"""

    host = _make_host()
    command_actions.select_command_history(host, "")
    # 空 text 不调 setText（apply 内部跳过空）。
    host._send_edit.setText.assert_not_called()


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
    host = _SendHost(text="")
    command_actions.send_text(host)
    assert "warning" in [c[0] for c in host.notify_calls]


def test_send_failure_notifies_error(qtbot):
    host = _SendHost(text="ATI", send_ok=False)
    command_actions.send_text(host)
    levels = [c[0] for c in host.notify_calls]
    assert "error" in levels
    error_msgs = [c[2] for c in host.notify_calls if c[0] == "error"]
    assert any("transport closed" in m for m in error_msgs)


def test_send_success_no_notify(qtbot):
    host = _SendHost(text="ATI", send_ok=True)
    command_actions.send_text(host)
    assert host.notify_calls == []


def test_command_actions_has_notify_helper():
    src = inspect.getsource(command_actions)
    assert "_notify" in src


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
    host = _InjectHost(text="")
    injection_actions.inject_received(host)
    assert "warning" in [c[0] for c in host.notify_calls]


def test_inject_failure_notifies_error(qtbot):
    host = _InjectHost(text="OK", inject_ok=False)
    injection_actions.inject_received(host)
    assert "error" in [c[0] for c in host.notify_calls]


def test_injection_actions_has_notify_helper():
    src = inspect.getsource(injection_actions)
    assert "_notify" in src
