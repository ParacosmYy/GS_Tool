"""controller_log_state 单元测试 — 日志条目追加 + 回调通知。"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_log_state import (
    append_connected_entry,
    append_error_entry,
    append_log_entry,
    append_system_entry,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.shared.results import OperationResult


def test_append_log_entry_adds_and_notifies():
    entries = []
    received = []
    entry = SerialWorkbenchLogEntry("tx", "AT", b"AT")
    append_log_entry(entries, [lambda e: received.append(e)], entry)
    assert len(entries) == 1
    assert received == [entry]


def test_append_system_entry():
    entries = []
    received = []
    append_system_entry(entries, [lambda e: received.append(e)], "Connected")
    assert len(entries) == 1
    assert entries[0].direction == "system"
    assert entries[0].text == "Connected"
    assert len(received) == 1


def test_append_error_entry_notifies_both_callbacks():
    entries = []
    log_received = []
    errors = []
    append_error_entry(entries, [lambda e: log_received.append(e)], [lambda m: errors.append(m)], "timeout")
    assert len(entries) == 1
    assert entries[0].direction == "error"
    assert len(log_received) == 1
    assert errors == ["timeout"]


def test_append_connected_entry_success():
    from embeddebug.serial_station.drivers import SerialPortConfig
    entries = []
    config = SerialPortConfig(port_name="COM3", baud_rate=115200)
    config_result = OperationResult.success(config)
    append_connected_entry(entries, [], config_result, "serial")
    assert len(entries) == 1
    assert "connected" in entries[0].text
    assert "COM3" in entries[0].text


def test_append_connected_entry_failure_no_entry():
    entries = []
    fail_result = OperationResult.failure("timeout")
    append_connected_entry(entries, [], fail_result, "serial")
    assert len(entries) == 0


def test_append_multiple_entries():
    entries = []
    append_system_entry(entries, [], "msg1")
    append_system_entry(entries, [], "msg2")
    append_system_entry(entries, [], "msg3")
    assert len(entries) == 3
