"""PyQt6 TCP client transport for Serial Station."""

from __future__ import annotations

from PyQt6.QtCore import QIODeviceBase
from PyQt6.QtNetwork import QAbstractSocket, QTcpSocket

from embeddebug.serial_station.drivers.base import (
    BytesCallback,
    ErrorCallback,
    SerialPortConfig,
    SerialTransport,
)


class TcpClientTransport(SerialTransport):
    """QTcpSocket adapter that preserves the SerialTransport byte contract."""

    def __init__(
        self,
        socket: QTcpSocket | None = None,
        connect_timeout_ms: int = 1000,
    ) -> None:
        self._socket = socket or QTcpSocket()
        self._connect_timeout_ms = connect_timeout_ms
        self._config: SerialPortConfig | None = None
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self._socket.readyRead.connect(self._handle_ready_read)
        self._socket.errorOccurred.connect(self._handle_error)

    @property
    def config(self) -> SerialPortConfig | None:
        return self._config

    @property
    def is_open(self) -> bool:
        return self._socket.state() == QAbstractSocket.SocketState.ConnectedState

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
        self._socket.connectToHost(host, port, QIODeviceBase.OpenModeFlag.ReadWrite)
        if not self._socket.waitForConnected(self._connect_timeout_ms):
            self._emit_error(self._socket.errorString() or "tcp_connect_failed")
            return False
        return True

    def close(self) -> None:
        self._socket.disconnectFromHost()
        if self._socket.state() != QAbstractSocket.SocketState.UnconnectedState:
            self._socket.waitForDisconnected(200)

    def write(self, data: bytes) -> int:
        if not self.is_open:
            self._emit_error("transport_not_open")
            return 0
        return int(self._socket.write(bytes(data)))

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def _handle_ready_read(self) -> None:
        payload = bytes(self._socket.readAll())
        if not payload:
            return
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
        raise ValueError("tcp_endpoint_requires_host_port")
    try:
        port = int(port_text)
    except ValueError as exc:
        raise ValueError("tcp_endpoint_port_invalid") from exc
    if port < 1 or port > 65535:
        raise ValueError("tcp_endpoint_port_invalid")
    return host, port
