from __future__ import annotations

from PyQt6.QtNetwork import QHostAddress, QTcpServer, QUdpSocket

from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    QtSerialPortTransport,
    SerialPortConfig,
    TcpClientTransport,
    UdpDatagramTransport,
)
from embeddebug.serial_station.drivers.tcp_server import TcpServerTransport


def test_fake_transport_open_write_inject_and_close():
    received: list[bytes] = []
    transport = FakeSerialTransport()
    transport.on_bytes_received(received.append)

    assert not transport.is_open

    transport.open(SerialPortConfig(port_name="loopback", baud_rate=115200))
    written = transport.write(b"ping")
    transport.inject_rx(b"pong")
    transport.close()

    assert written == 4
    assert transport.written == [b"ping"]
    assert received == [b"pong"]
    assert not transport.is_open


def test_fake_transport_reports_errors_for_closed_write_and_scripted_open_failure():
    errors: list[str] = []
    transport = FakeSerialTransport(open_error="denied")
    transport.on_error(errors.append)

    assert not transport.open(SerialPortConfig(port_name="COM404"))

    closed = FakeSerialTransport()
    closed.on_error(errors.append)
    assert closed.write(b"data") == 0

    assert errors == ["denied", "transport_not_open"]


def test_qt_serial_transport_configures_port_without_opening(qtbot):
    transport = QtSerialPortTransport()
    config = SerialPortConfig(port_name="COM_TEST", baud_rate=57600)

    transport.configure(config)

    assert transport.config == config
    assert transport.port_name == "COM_TEST"
    assert transport.baud_rate == 57600
    assert not transport.is_open
    assert isinstance(QtSerialPortTransport.available_ports(), list)


def test_qt_serial_transport_configures_frame_settings(qtbot):
    transport = QtSerialPortTransport()
    config = SerialPortConfig(
        port_name="COM_TEST",
        baud_rate=38400,
        data_bits=7,
        parity="even",
        stop_bits="2",
        flow_control="hardware",
    )

    transport.configure(config)

    assert transport.config == config
    assert transport.data_bits == 7
    assert transport.parity == "even"
    assert transport.stop_bits == "2"
    assert transport.flow_control == "hardware"


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


def test_udp_datagram_transport_rejects_invalid_endpoint(qtbot):
    errors: list[str] = []
    transport = UdpDatagramTransport()
    transport.on_error(errors.append)

    assert not transport.open(SerialPortConfig(port_name="127.0.0.1"))
    assert errors == ["udp_endpoint_requires_host_port"]


def test_udp_datagram_transport_loopback_read_write(qtbot):
    received: list[bytes] = []
    peer = QUdpSocket()
    assert peer.bind(QHostAddress(QHostAddress.SpecialAddress.LocalHost), 0)
    transport = UdpDatagramTransport()
    transport.on_bytes_received(received.append)

    assert transport.open(SerialPortConfig(port_name=f"127.0.0.1:{peer.localPort()}"))

    assert peer.writeDatagram(b"pong", QHostAddress.SpecialAddress.LocalHost, transport.local_port) == 4
    qtbot.waitUntil(lambda: received == [b"pong"], timeout=1000)

    assert transport.write(b"ping") == 4
    qtbot.waitUntil(lambda: peer.hasPendingDatagrams(), timeout=1000)
    datagram = peer.receiveDatagram()
    assert bytes(datagram.data()) == b"ping"

    transport.close()
    peer.close()


def test_tcp_client_available_ports_returns_list():
    assert isinstance(TcpClientTransport.available_ports(), list)


def test_tcp_client_disconnected_state_and_callbacks():
    errors: list[str] = []
    transport = TcpClientTransport()

    transport.on_bytes_received(lambda b: None)
    transport.on_error(errors.append)
    transport._emit_error("test_error")
    transport.close()

    assert transport.config is None
    assert transport.is_open is False
    assert errors == ["test_error"]


def test_tcp_client_emit_error_without_callback_no_crash():
    TcpClientTransport()._emit_error("no_callback")


def test_tcp_server_disconnected_state_and_callbacks():
    transport = TcpServerTransport()

    transport.on_bytes_received(lambda b: None)
    transport.on_error(lambda m: None)
    transport.close()

    assert transport.local_port is None
    assert transport.has_client is False
    assert transport.config is None
    assert transport.is_open is False
    assert TcpServerTransport.available_ports() == []


def test_udp_datagram_disconnected_state_and_callbacks():
    transport = UdpDatagramTransport()

    transport.on_bytes_received(lambda b: None)
    transport.on_error(lambda m: None)
    transport.close()

    assert isinstance(UdpDatagramTransport.available_ports(), list)
    assert transport.local_port == 0
    assert transport.config is None
    assert transport.is_open is False
