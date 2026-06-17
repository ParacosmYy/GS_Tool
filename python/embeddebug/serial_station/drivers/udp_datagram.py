"""PyQt6 UDP datagram transport for Serial Station."""

from __future__ import annotations

from PyQt6.QtNetwork import QAbstractSocket, QHostAddress, QUdpSocket

from embeddebug.serial_station.drivers.base import (
    BytesCallback,
    ErrorCallback,
    SerialPortConfig,
    SerialTransport,
)


class UdpDatagramTransport(SerialTransport):
    """QUdpSocket adapter for datagram-oriented byte exchange."""

    def __init__(self, socket: QUdpSocket | None = None) -> None:
        self._socket = socket or QUdpSocket()
        self._config: SerialPortConfig | None = None
        self._remote_address: QHostAddress | None = None
        self._remote_port = 0
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self._socket.readyRead.connect(self._handle_ready_read)
        self._socket.errorOccurred.connect(self._handle_error)

    @property
    def config(self) -> SerialPortConfig | None:
        return self._config

    @property
    def is_open(self) -> bool:
        return self._socket.state() == QAbstractSocket.SocketState.BoundState

    @property
    def local_port(self) -> int:
        return int(self._socket.localPort())

    @staticmethod
    def available_ports() -> list[str]:
        return []

    def open(self, config: SerialPortConfig) -> bool:
        self._config = config
        try:
            host, port = _parse_endpoint(config.port_name)
        except ValueError as exc:
            self._emit_error(str(exc))
            return False
        self._remote_address = QHostAddress(host)
        self._remote_port = port
        if self._remote_address.isNull():
            self._emit_error("udp_endpoint_host_invalid")
            return False
        if self._socket.state() != QAbstractSocket.SocketState.UnconnectedState:
            self._socket.close()
        ok = self._socket.bind(QHostAddress(QHostAddress.SpecialAddress.AnyIPv4), 0)
        if not ok:
            self._emit_error(self._socket.errorString() or "udp_bind_failed")
        return bool(ok)

    def close(self) -> None:
        self._socket.close()
        self._remote_address = None
        self._remote_port = 0

    def write(self, data: bytes) -> int:
        if not self.is_open or self._remote_address is None or self._remote_port == 0:
            self._emit_error("transport_not_open")
            return 0
        return int(self._socket.writeDatagram(bytes(data), self._remote_address, self._remote_port))

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def _handle_ready_read(self) -> None:
        while self._socket.hasPendingDatagrams():
            datagram = self._socket.receiveDatagram()
            payload = bytes(datagram.data())
            if not payload:
                continue
            for callback in list(self._bytes_callbacks):
                callback(payload)

    def _handle_error(self, error: QAbstractSocket.SocketError) -> None:
        if error == QAbstractSocket.SocketError.UnknownSocketError and not self._socket.errorString():
            return
        self._emit_error(self._socket.errorString() or error.name)

    def _emit_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)


def _parse_endpoint(endpoint: str) -> tuple[str, int]:
    host, separator, port_text = endpoint.rpartition(":")
    if not separator or not host or not port_text:
        raise ValueError("udp_endpoint_requires_host_port")
    try:
        port = int(port_text)
    except ValueError as exc:
        raise ValueError("udp_endpoint_port_invalid") from exc
    if port < 1 or port > 65535:
        raise ValueError("udp_endpoint_port_invalid")
    return host, port
