"""OTA SerialTransportAdapter 单元测试 — 适配器队列 + 超时读 + drain。"""

from __future__ import annotations

import time

from embeddebug.ota.transport_adapter import SerialTransportAdapter
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig


def _make_adapter() -> tuple[SerialTransportAdapter, FakeSerialTransport]:
    transport = FakeSerialTransport()
    transport.open(SerialPortConfig(port_name="fake", baud_rate=0))
    adapter = SerialTransportAdapter(transport)
    return adapter, transport


def test_write_forwards_to_transport():
    adapter, transport = _make_adapter()
    written = adapter.write(b"\x01\x02")
    assert written == 2
    assert b"\x01\x02" in transport.written


def test_read_timeout_returns_empty():
    adapter, _ = _make_adapter()
    result = adapter.read(timeout_ms=50)
    assert result == b""


def test_inject_then_read():
    """inject_rx 后 adapter 能读到。"""
    adapter, transport = _make_adapter()
    transport.inject_rx(b"\x41\x42")
    time.sleep(0.05)  # 等回调入队
    result = adapter.read(timeout_ms=100)
    assert result == b"\x41\x42"


def test_drain_empty():
    adapter, _ = _make_adapter()
    assert adapter.drain() == b""


def test_drain_after_inject():
    adapter, transport = _make_adapter()
    transport.inject_rx(b"\x01\x02\x03")
    time.sleep(0.05)
    result = adapter.drain()
    assert result == b"\x01\x02\x03"


def test_drain_clears_queue():
    """drain 后再读应超时。"""
    adapter, transport = _make_adapter()
    transport.inject_rx(b"\x01")
    time.sleep(0.05)
    adapter.drain()
    assert adapter.read(timeout_ms=50) == b""
