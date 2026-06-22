"""protocol_actions select_protocol + injection_actions inject_received 行为边界。

覆盖 protocol_actions.select_protocol：
1. select_protocol 调 controller.set_protocol + set_status_text。

覆盖 injection_actions.inject_received：
2. 空文本 → 不调 controller + status 'RX text is empty' + notify warning。
3. 成功 → controller.inject_received_text + status 'Received fake bytes'。
4. 失败 → status + notify error。
5. _notify 安全降级（host 无 _notify 方法不抛）。
6. _notify 异常被吞（try/except）。
"""

from __future__ import annotations

from types import SimpleNamespace
from unittest.mock import MagicMock

from embeddebug.serial_station.ui import injection_actions, protocol_actions
from embeddebug.shared.results import OperationResult


def _make_host(inject_text="", has_notify=True):
    host = SimpleNamespace()
    host.tr = lambda s: s
    host._inject_edit = MagicMock()
    host._inject_edit.text.return_value = inject_text
    host._status_label = MagicMock()
    host._controller = MagicMock()
    if has_notify:
        host._notify = MagicMock()
    return host


def _ok():
    return OperationResult.success()


def _fail(msg="boom"):
    return OperationResult.failure("err", msg)


# ── protocol_actions.select_protocol ─────────────────────────────
def test_select_protocol_calls_controller():
    host = SimpleNamespace()
    host.tr = lambda s: s
    host._status_label = MagicMock()
    host._controller = MagicMock()
    protocol_actions.select_protocol(host, "raw_data")
    host._controller.set_protocol.assert_called_once_with("raw_data")


def test_select_protocol_sets_status():
    host = SimpleNamespace()
    host.tr = lambda s: s
    host._status_label = MagicMock()
    host._controller = MagicMock()
    protocol_actions.select_protocol(host, "just_float")
    host._status_label.setText.assert_called_once()


# ── inject_received 空文本 ───────────────────────────────────────
def test_inject_empty_does_not_call_controller():
    host = _make_host(inject_text="")
    injection_actions.inject_received(host)
    host._controller.inject_received_text.assert_not_called()


def test_inject_empty_notifies_warning():
    host = _make_host(inject_text="")
    injection_actions.inject_received(host)
    host._notify.assert_called_once()
    assert host._notify.call_args[0][0] == "warning"


# ── inject_received 成功 ──────────────────────────────────────────
def test_inject_success_calls_controller():
    host = _make_host(inject_text="hello")
    host._controller.inject_received_text.return_value = _ok()
    injection_actions.inject_received(host)
    host._controller.inject_received_text.assert_called_once_with("hello")


def test_inject_success_does_not_notify_error():
    host = _make_host(inject_text="hello")
    host._controller.inject_received_text.return_value = _ok()
    injection_actions.inject_received(host)
    # 成功路径不调 _notify（无 error/warning）。
    host._notify.assert_not_called()


# ── inject_received 失败 ──────────────────────────────────────────
def test_inject_failure_calls_controller():
    host = _make_host(inject_text="bad")
    host._controller.inject_received_text.return_value = _fail("timeout")
    injection_actions.inject_received(host)
    host._controller.inject_received_text.assert_called_once_with("bad")


def test_inject_failure_notifies_error():
    host = _make_host(inject_text="bad")
    host._controller.inject_received_text.return_value = _fail("timeout")
    injection_actions.inject_received(host)
    host._notify.assert_called_once()
    assert host._notify.call_args[0][0] == "error"


# ── _notify 安全降级 ──────────────────────────────────────────────
def test_inject_empty_no_notify_method_does_not_crash():
    """host 无 _notify 方法 → getattr None 安全降级不抛。"""

    host = _make_host(inject_text="", has_notify=False)
    injection_actions.inject_received(host)  # 不抛


def test_notify_swallows_exception():
    """_notify 方法抛异常 → try/except 吞（不阻塞注入逻辑）。"""

    host = _make_host(inject_text="")
    host._notify.side_effect = RuntimeError("toast broken")
    injection_actions.inject_received(host)  # 不抛（异常被吞）
