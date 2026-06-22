"""PyQt6 TCP server transport for Serial Station.

QTcpServer 监听端口，接受一个客户端连接，把首条连接的 socket 作为收发通道。
后续连接被拒绝（单客户端模式，对齐 VOFA+ TCP server 调试场景）。

对齐 SerialTransport 字节契约：open/close/write/on_bytes_received/on_error。
"""

from __future__ import annotations

from PyQt6.QtNetwork import QAbstractSocket, QHostAddress, QTcpServer, QTcpSocket

from embeddebug.serial_station.drivers.base import (
    BytesCallback,
    ErrorCallback,
    SerialPortConfig,
    SerialTransport,
)


class TcpServerTransport(SerialTransport):
    """QTcpServer adapter：监听端口，接受首条连接作为收发通道。"""

    def __init__(self, server: QTcpServer | None = None) -> None:
        self._server = server or QTcpServer()
        self._client: QTcpSocket | None = None
        self._config: SerialPortConfig | None = None
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self._server.newConnection.connect(self._handle_new_connection)

    @property
    def config(self) -> SerialPortConfig | None:
        return self._config

    @property
    def is_open(self) -> bool:
        return self._server.isListening()

    @property
    def local_port(self) -> int | None:
        port = self._server.serverPort()
        return int(port) if port > 0 else None

    @property
    def has_client(self) -> bool:
        return self._client is not None and self._client.state() == QAbstractSocket.SocketState.ConnectedState

    @staticmethod
    def available_ports() -> list[str]:
        return []

    def open(self, config: SerialPortConfig) -> bool:
        self._config = config
        try:
            port = _parse_listen_port(config.port_name)
        except ValueError as exc:
            self._emit_error(str(exc))
            return False
        host = _parse_listen_host(config.port_name, default="0.0.0.0")
        address = QHostAddress(host)
        if not self._server.listen(address, port):
            self._emit_error(self._server.serverErrorString() or "tcp_server_listen_failed")
            return False
        return True

    def close(self) -> None:
        if self._client is not None:
            self._client.disconnectFromHost()
            if self._client.state() != QAbstractSocket.SocketState.UnconnectedState:
                self._client.waitForDisconnected(200)
            self._client = None
        if self._server.isListening():
            self._server.close()

    def write(self, data: bytes) -> int:
        if not self.has_client or self._client is None:
            self._emit_error("transport_not_open")
            return 0
        return int(self._client.write(bytes(data)))

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    # ── 内部 ───────────────────────────────────────────────────────
    def _handle_new_connection(self) -> None:
        while self._server.hasPendingConnections():
            pending = self._server.nextPendingConnection()
            if pending is None:
                continue
            if self._client is not None and self._client.state() == QAbstractSocket.SocketState.ConnectedState:
                # 已有客户端，拒绝新连接。
                pending.disconnectFromHost()
                continue
            self._client = pending
            self._client.readyRead.connect(self._handle_ready_read)
            self._client.disconnected.connect(self._handle_client_disconnected)
            self._client.errorOccurred.connect(self._handle_socket_error)

    def _handle_ready_read(self) -> None:
        if self._client is None:
            return
        payload = bytes(self._client.readAll())
        if not payload:
            return
        for callback in list(self._bytes_callbacks):
            callback(payload)

    def _handle_client_disconnected(self) -> None:
        if self._client is not None:
            self._client = None

    def _handle_socket_error(self, error: QAbstractSocket.SocketError) -> None:
        if self._client is None:
            return
        if error == QAbstractSocket.SocketError.UnknownSocketError and not self._client.errorString():
            return
        self._emit_error(self._client.errorString() or error.name)

    def _emit_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)


def _parse_listen_port(endpoint: str) -> int:
    """从 endpoint 解析监听端口。

    接受 ":5000"、"0.0.0.0:5000"、"5000" 三种形式。
    """

    text = endpoint.strip()
    if ":" in text:
        _, _, port_text = text.rpartition(":")
    else:
        port_text = text
    try:
        port = int(port_text)
    except ValueError as exc:
        raise ValueError("tcp_server_port_invalid") from exc
    if port < 1 or port > 65535:
        raise ValueError("tcp_server_port_invalid")
    return port


def _parse_listen_host(endpoint: str, default: str = "0.0.0.0") -> str:
    """从 endpoint 解析监听 host，缺省返回 default。"""

    text = endpoint.strip()
    if ":" in text:
        host, separator, _ = text.rpartition(":")
        if separator and host:
            return host
    return default
