from __future__ import annotations

from PyQt6.QtNetwork import QHostAddress, QUdpSocket

from embeddebug.serial_station.drivers import SerialPortConfig, UdpDatagramTransport


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
