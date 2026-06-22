"""SerialWorkbenchLogEntry 单元测试 — frozen dataclass 契约。"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry


def test_log_entry_basic():
    e = SerialWorkbenchLogEntry(direction="tx", text="AT", raw=b"\x41\x54")
    assert e.direction == "tx"
    assert e.text == "AT"
    assert e.raw == b"\x41\x54"


def test_log_entry_rx():
    e = SerialWorkbenchLogEntry(direction="rx", text="OK", raw=b"\x4F\x4B")
    assert e.direction == "rx"


def test_log_entry_system():
    e = SerialWorkbenchLogEntry(direction="system", text="Connected", raw=b"")
    assert e.direction == "system"
    assert e.raw == b""


def test_log_entry_empty_raw():
    e = SerialWorkbenchLogEntry(direction="error", text="timeout", raw=b"")
    assert e.raw == b""


def test_log_entry_frozen():
    e = SerialWorkbenchLogEntry(direction="tx", text="x", raw=b"")
    with pytest.raises((AttributeError, TypeError)):
        e.direction = "rx"
