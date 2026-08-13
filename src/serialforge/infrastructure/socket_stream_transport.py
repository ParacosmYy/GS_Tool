"""Reusable bounded TCP byte-stream adapter for TCP and RTT bridges."""

from __future__ import annotations

import socket
from threading import Lock
from typing import Protocol

from ..domain.errors import (
    ConfigurationError,
    TransportCloseError,
    TransportNotOpenError,
    TransportOpenError,
    TransportReadError,
    TransportWriteError,
)
from ..domain.models import (
    Endpoint,
    PeerAddress,
    StreamReadResult,
    StreamWrite,
    TransportKind,
)
from ..domain.ports import StreamTransportPort


class SocketStreamConfig(Protocol):
    """Minimal configuration contract shared by TCP-like stream bridges."""

    remote_peer: PeerAddress
    connect_timeout: float
    read_timeout: float
    write_timeout: float
    read_chunk_size: int

    @property
    def endpoint(self) -> Endpoint:
        """Return the typed endpoint exposed to the application."""


class SocketStreamTransport(StreamTransportPort):
    """Own one socket and keep all blocking operations behind the session worker."""

    def __init__(
        self,
        config: SocketStreamConfig,
        *,
        adapter_name: str,
        connect_preamble: bytes = b"",
    ) -> None:
        self._config = config
        self._adapter_name = adapter_name
        self._connect_preamble = bytes(connect_preamble)
        self._socket: socket.socket | None = None
        self._lock = Lock()

    @property
    def kind(self) -> TransportKind:
        """Kind."""
        return self._config.endpoint.transport

    @property
    def endpoint(self) -> Endpoint:
        """Endpoint."""
        return self._config.endpoint

    @property
    def peer(self) -> PeerAddress:
        """Peer."""
        return self._config.remote_peer

    def open(self) -> None:
        """Open."""
        with self._lock:
            if self._socket is not None:
                raise TransportOpenError(f"{self._adapter_name} 连接已经打开。")

        handle: socket.socket | None = None
        try:
            handle = socket.create_connection(
                (self._config.remote_peer.host, self._config.remote_peer.port),
                timeout=self._config.connect_timeout,
            )
            if self._connect_preamble:
                handle.settimeout(self._config.write_timeout)
                handle.sendall(self._connect_preamble)
            handle.settimeout(self._config.read_timeout)
        except TimeoutError as exc:
            self._close_quietly(handle)
            phase = "连接" if not self._connect_preamble else "连接/握手"
            raise TransportOpenError(
                f"{self._adapter_name} {phase}超时。",
                detail=(
                    f"peer={self._config.remote_peer.display}; "
                    f"timeout={self._config.connect_timeout}"
                ),
            ) from exc
        except OSError as exc:
            self._close_quietly(handle)
            phase = "连接" if not self._connect_preamble else "连接/握手"
            raise TransportOpenError(
                f"无法建立 {self._adapter_name} {phase}。",
                detail=(f"peer={self._config.remote_peer.display}; {type(exc).__name__}: {exc}"),
            ) from exc

        with self._lock:
            self._socket = handle

    def receive(self, max_bytes: int) -> StreamReadResult:
        """Receive."""
        if max_bytes <= 0:
            raise ConfigurationError(f"{self._adapter_name} receive max_bytes 必须为正数。")
        handle = self._require_open()
        try:
            payload = handle.recv(min(max_bytes, self._config.read_chunk_size))
        except TimeoutError:
            return StreamReadResult.timeout(self._config.remote_peer)
        except OSError as exc:
            raise TransportReadError(
                f"读取 {self._adapter_name} 数据失败。",
                detail=(f"peer={self._config.remote_peer.display}; {type(exc).__name__}: {exc}"),
            ) from exc
        if not payload:
            return StreamReadResult.eof(self._config.remote_peer)
        return StreamReadResult.data(payload, self._config.remote_peer)

    def send(self, write: StreamWrite) -> None:
        """Send."""
        handle = self._require_open()
        try:
            handle.settimeout(self._config.write_timeout)
            handle.sendall(write.payload)
        except TimeoutError as exc:
            raise TransportWriteError(
                f"{self._adapter_name} 写入超时。",
                detail=(
                    f"peer={self._config.remote_peer.display}; timeout={self._config.write_timeout}"
                ),
            ) from exc
        except OSError as exc:
            raise TransportWriteError(
                f"写入 {self._adapter_name} 数据失败。",
                detail=(f"peer={self._config.remote_peer.display}; {type(exc).__name__}: {exc}"),
            ) from exc
        finally:
            try:
                handle.settimeout(self._config.read_timeout)
            except OSError:
                pass

    def close(self) -> None:
        """Close."""
        with self._lock:
            handle = self._socket
            self._socket = None
        if handle is None:
            return
        try:
            try:
                handle.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            handle.close()
        except OSError as exc:
            raise TransportCloseError(
                f"关闭 {self._adapter_name} 连接失败。",
                detail=(f"peer={self._config.remote_peer.display}; {type(exc).__name__}: {exc}"),
            ) from exc

    def _require_open(self) -> socket.socket:
        """Require open."""
        with self._lock:
            handle = self._socket
        if handle is None:
            raise TransportNotOpenError()
        return handle

    @staticmethod
    def _close_quietly(handle: socket.socket | None) -> None:
        """Close quietly."""
        if handle is None:
            return
        try:
            handle.close()
        except OSError:
            pass
