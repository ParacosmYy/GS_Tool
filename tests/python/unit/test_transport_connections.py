"""transport_connections + connection_results 单元测试。

用 fake transport 测 open_transport_result + open_serial/endpoint_transport。
覆盖：成功路径返回 OperationResult.success、失败返回 failure、
config 字段传递正确。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.connection_results import open_transport_result
from embeddebug.serial_station.controllers.transport_connections import (
    open_endpoint_transport,
    open_serial_transport,
)
from embeddebug.serial_station.drivers import SerialPortConfig, SerialTransport, TransportRegistry


class _FakeTransport(SerialTransport):
    """可控开/关的假 transport。"""

    def __init__(self, should_open: bool = True) -> None:
        self._should_open = should_open
        self.opened_config: SerialPortConfig | None = None
        self._is_open = False

    def open(self, config: SerialPortConfig) -> bool:
        if self._should_open:
            self.opened_config = config
            self._is_open = True
            return True
        return False

    def close(self) -> None:
        self._is_open = False

    def read(self) -> bytes:
        return b""

    def write(self, data: bytes) -> int:
        return len(data)

    @property
    def is_open(self) -> bool:
        return self._is_open

    @property
    def config(self) -> SerialPortConfig | None:
        return self.opened_config

    def on_bytes_received(self, callback): ...
    def on_error(self, callback): ...


class _FakeRegistry(TransportRegistry):
    """返回预设 transport 的假 registry。"""

    def __init__(self, transport: SerialTransport) -> None:
        self._transport = transport

    def create(self, mode: str) -> SerialTransport:
        return self._transport


def test_open_transport_result_success():
    transport = _FakeTransport(should_open=True)
    config = SerialPortConfig(port_name="COM1", baud_rate=9600)
    result = open_transport_result(transport, config, "serial")
    assert result.ok is True
    assert result.value == config


def test_open_transport_result_failure():
    transport = _FakeTransport(should_open=False)
    config = SerialPortConfig(port_name="COM1", baud_rate=9600)
    result = open_transport_result(transport, config, "serial")
    assert result.failed is True
    assert "serial" in result.message or "Failed" in result.message


def test_open_serial_transport_creates_and_opens():
    transport = _FakeTransport(should_open=True)
    registry = _FakeRegistry(transport)
    replaced: list[SerialTransport] = []
    result = open_serial_transport(
        registry,
        replaced.append,
        port_name="COM3",
        baud_rate=115200,
        data_bits=8,
    )
    assert result.ok is True
    assert len(replaced) == 1
    assert transport.opened_config is not None
    assert transport.opened_config.port_name == "COM3"
    assert transport.opened_config.baud_rate == 115200


def test_open_endpoint_transport_tcp():
    transport = _FakeTransport(should_open=True)
    registry = _FakeRegistry(transport)
    replaced: list[SerialTransport] = []
    result = open_endpoint_transport(registry, replaced.append, "tcp", "192.168.1.1", 8080)
    assert result.ok is True
    assert transport.opened_config is not None
    assert "192.168.1.1:8080" in transport.opened_config.port_name


def test_open_endpoint_transport_failure_returns_error():
    transport = _FakeTransport(should_open=False)
    registry = _FakeRegistry(transport)
    result = open_endpoint_transport(registry, lambda _: None, "udp", "x", 1)
    assert result.failed is True
