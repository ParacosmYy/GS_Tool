"""B9 TCP server transport 测试。"""

from __future__ import annotations

import os
import socket as stdlib_socket
import time

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QCoreApplication
from PyQt6.QtNetwork import QAbstractSocket, QTcpServer, QTcpSocket

from embeddebug.serial_station.drivers.base import SerialPortConfig
from embeddebug.serial_station.drivers.registry import TransportRegistry
from embeddebug.serial_station.drivers.tcp_server import (
    TcpServerTransport,
    _parse_listen_host,
    _parse_listen_port,
)


def _free_port() -> int:
    """获取一个可用端口（避免与其它测试冲突）。"""

    s = stdlib_socket.socket(stdlib_socket.AF_INET, stdlib_socket.SOCK_STREAM)
    s.bind(("127.0.0.1", 0))
    port = s.getsockname()[1]
    s.close()
    return port


def _process_events(app: QCoreApplication, ms: int = 50) -> None:
    """处理 Qt 事件若干毫秒，让信号传递。"""

    deadline = time.time() + ms / 1000.0
    while time.time() < deadline:
        app.processEvents()
        time.sleep(0.005)


# ── 端口/host 解析 ────────────────────────────────────────────────
def test_parse_listen_port_colon_form():
    assert _parse_listen_port(":5000") == 5000
    assert _parse_listen_port("0.0.0.0:5000") == 5000


def test_parse_listen_port_bare():
    assert _parse_listen_port("5000") == 5000


def test_parse_listen_port_invalid_raises():
    import pytest

    for bad in ("abc", "0", "70000", ""):
        with pytest.raises(ValueError):
            _parse_listen_port(bad)


def test_parse_listen_host_defaults():
    assert _parse_listen_host(":5000") == "0.0.0.0"
    assert _parse_listen_host("5000") == "0.0.0.0"


def test_parse_listen_host_explicit():
    assert _parse_listen_host("127.0.0.1:5000") == "127.0.0.1"


# ── transport 行为 ────────────────────────────────────────────────
def test_tcp_server_available_ports_empty():
    assert TcpServerTransport.available_ports() == []


def test_tcp_server_open_and_listen(qapp):
    transport = TcpServerTransport()
    port = _free_port()
    config = SerialPortConfig(port_name=f"127.0.0.1:{port}")
    try:
        assert transport.open(config) is True
        assert transport.is_open is True
        assert transport.local_port == port
        assert transport.has_client is False
    finally:
        transport.close()


def test_tcp_server_open_invalid_port_emits_error(qapp):
    transport = TcpServerTransport()
    errors: list[str] = []
    transport.on_error(lambda msg: errors.append(msg))
    config = SerialPortConfig(port_name="0")
    assert transport.open(config) is False
    assert errors
    assert transport.is_open is False


def test_tcp_server_write_without_client_emits_error(qapp):
    transport = TcpServerTransport()
    port = _free_port()
    errors: list[str] = []
    transport.on_error(lambda msg: errors.append(msg))
    transport.open(SerialPortConfig(port_name=f"127.0.0.1:{port}"))
    try:
        written = transport.write(b"hello")
        assert written == 0
        assert "transport_not_open" in errors
    finally:
        transport.close()


def test_tcp_server_accepts_client_and_receives(qapp):
    """端到端：server 监听 → 客户端连接 → 客户端发送 → server 收到字节。"""

    transport = TcpServerTransport()
    port = _free_port()
    received: list[bytes] = []
    transport.on_bytes_received(lambda data: received.append(data))
    assert transport.open(SerialPortConfig(port_name=f"127.0.0.1:{port}"))

    client = QTcpSocket()
    client.connectToHost("127.0.0.1", port)
    _process_events(qapp, 200)
    assert client.state() == QAbstractSocket.SocketState.ConnectedState

    client.write(b"ping")
    client.flush()
    _process_events(qapp, 300)

    assert transport.has_client is True
    assert b"ping" in b"".join(received)

    client.disconnectFromHost()
    _process_events(qapp, 100)
    transport.close()


def test_tcp_server_close_stops_listening(qapp):
    transport = TcpServerTransport()
    port = _free_port()
    transport.open(SerialPortConfig(port_name=f"127.0.0.1:{port}"))
    assert transport.is_open is True
    transport.close()
    assert transport.is_open is False


# ── registry 集成 ─────────────────────────────────────────────────
def test_registry_includes_tcp_server_mode():
    registry = TransportRegistry.with_defaults()
    assert "tcp_server" in registry.modes
    transport = registry.create("tcp_server")
    assert isinstance(transport, TcpServerTransport)


def test_registry_tcp_server_available_ports_empty():
    registry = TransportRegistry.with_defaults()
    assert registry.available_ports("tcp_server") == ()
