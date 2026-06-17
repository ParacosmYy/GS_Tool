from __future__ import annotations

from PyQt6.QtNetwork import QHostAddress, QTcpServer

from embeddebug.serial_station.drivers import SerialPortConfig, TcpClientTransport


def test_tcp_client_transport_rejects_invalid_endpoint(qtbot):
    errors: list[str] = []
    transport = TcpClientTransport()
    transport.on_error(errors.append)

    assert not transport.open(SerialPortConfig(port_name="127.0.0.1"))
    assert errors == ["tcp_endpoint_requires_host_port"]


def test_tcp_client_transport_loopback_read_write(qtbot):
    received: list[bytes] = []
    server = QTcpServer()
    assert server.listen(QHostAddress(QHostAddress.SpecialAddress.LocalHost), 0)
    transport = TcpClientTransport(connect_timeout_ms=1000)
    transport.on_bytes_received(received.append)

    assert transport.open(SerialPortConfig(port_name=f"127.0.0.1:{server.serverPort()}"))
    assert server.waitForNewConnection(1000)
    peer = server.nextPendingConnection()
    assert peer is not None

    assert peer.write(b"pong") == 4
    assert peer.waitForBytesWritten(1000)
    qtbot.waitUntil(lambda: received == [b"pong"], timeout=1000)

    assert transport.write(b"ping") == 4
    qtbot.waitUntil(lambda: peer.bytesAvailable() >= 4, timeout=1000)
    assert bytes(peer.readAll()) == b"ping"

    transport.close()
    peer.close()
    server.close()
