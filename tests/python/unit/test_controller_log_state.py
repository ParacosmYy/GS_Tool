from __future__ import annotations

from embeddebug.serial_station.controllers.controller_log_state import (
    append_connected_entry,
    append_error_entry,
    append_log_entry,
    append_system_entry,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.drivers import SerialPortConfig
from embeddebug.shared import OperationResult


def test_append_log_entry_stores_entry_and_notifies_log_callbacks():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []

    append_log_entry(
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
        SerialWorkbenchLogEntry("tx", "ping", b"ping"),
    )

    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("tx", "ping", b"ping")
    ]
    assert logged == ["tx:ping"]


def test_append_system_entry_builds_utf8_raw_and_notifies_log_callbacks():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []

    append_system_entry(entries, [lambda entry: logged.append(entry.text)], "profile loaded")

    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("system", "profile loaded", b"profile loaded")
    ]
    assert logged == ["profile loaded"]


def test_append_error_entry_logs_before_notifying_error_callbacks():
    entries: list[SerialWorkbenchLogEntry] = []
    events: list[str] = []

    append_error_entry(
        entries,
        [lambda entry: events.append(f"log:{entry.direction}:{entry.text}")],
        [lambda message: events.append(f"error:{message}:{len(entries)}")],
        "transport_not_open",
    )

    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("error", "transport_not_open", b"transport_not_open")
    ]
    assert events == ["log:error:transport_not_open", "error:transport_not_open:1"]


def test_append_connected_entry_logs_only_successful_configs():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []

    append_connected_entry(
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
        OperationResult.success(SerialPortConfig("FAKE_LOOPBACK", 115200)),
        "fake",
    )
    append_connected_entry(
        entries,
        [lambda entry: logged.append(entry.text)],
        OperationResult.failure("transport_open_failed", "denied"),
        "fake",
    )

    assert [(entry.direction, entry.text, entry.raw) for entry in entries] == [
        ("system", "connected: fake FAKE_LOOPBACK", b"connected: fake FAKE_LOOPBACK")
    ]
    assert logged == ["system:connected: fake FAKE_LOOPBACK"]
