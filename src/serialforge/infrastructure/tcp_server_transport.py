"""Standard-library IPv4 TCP listener with bounded accepted clients."""

from __future__ import annotations

import select
import socket
import sys
from collections import deque
from dataclasses import dataclass
from threading import Lock
from uuid import uuid4

from ..domain.errors import (
    ConfigurationError,
    TcpServerNoClientError,
    TcpServerPeerNotConnectedError,
    TransportNotOpenError,
    TransportOpenError,
    TransportReadError,
    TransportWriteError,
)
from ..domain.models import (
    Endpoint,
    PeerAddress,
    PeerId,
    ServerReadResult,
    ServerRejectReason,
    TcpServerPeerSnapshot,
    TcpServerSend,
    TcpServerTransportConfig,
    TransportKind,
)
from ..domain.ports import TcpServerTransportPort


@dataclass(slots=True)
class _PeerSocket:
    peer_id: PeerId
    address: PeerAddress
    handle: socket.socket


@dataclass(slots=True)
class _PendingWrite:
    write: TcpServerSend
    offset: int = 0


class TcpServerTransport(TcpServerTransportPort):
    """An IPv4 listener whose worker owns all socket I/O and final close."""

    def __init__(self, config: TcpServerTransportConfig) -> None:
        self._config = config
        self._listener: socket.socket | None = None
        self._clients: dict[PeerId, _PeerSocket] = {}
        self._pending_writes: dict[PeerId, _PendingWrite] = {}
        self._ready_results: deque[ServerReadResult] = deque()
        self._wake_reader: socket.socket | None = None
        self._wake_writer: socket.socket | None = None
        self._closing = False
        self._round_robin_cursor = 0
        self._lock = Lock()

    @property
    def kind(self) -> TransportKind:
        return TransportKind.TCP_SERVER

    @property
    def endpoint(self) -> Endpoint:
        return self._config.endpoint

    @property
    def peer(self) -> PeerAddress | None:
        """Return a peer only when exactly one client is connected."""

        with self._lock:
            return next(iter(self._clients.values())).address if len(self._clients) == 1 else None

    @property
    def peers(self) -> tuple[TcpServerPeerSnapshot, ...]:
        """Return stable peer snapshots without exposing socket handles."""

        with self._lock:
            return tuple(
                TcpServerPeerSnapshot(
                    peer_id=client.peer_id,
                    address=client.address,
                    queued_items=(1 if client.peer_id in self._pending_writes else 0),
                    queued_bytes=self._pending_bytes(client.peer_id),
                )
                for client in sorted(
                    self._clients.values(),
                    key=lambda item: (item.address.host, item.address.port),
                )
            )

    def open(self) -> None:
        """Bind the listener and create a socketpair used to wake select()."""

        with self._lock:
            if self._listener is not None:
                raise TransportOpenError("TCP Server 监听已经打开。")
            listener: socket.socket | None = None
            wake_reader: socket.socket | None = None
            wake_writer: socket.socket | None = None
            try:
                listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                if sys.platform == "win32":
                    exclusive = getattr(socket, "SO_EXCLUSIVEADDRUSE", None)
                    if exclusive is not None:
                        listener.setsockopt(socket.SOL_SOCKET, exclusive, 1)
                listener.bind((self._config.bind_host, self._config.listen_port))
                listener.listen(self._config.max_clients)
                listener.setblocking(False)

                wake_reader, wake_writer = socket.socketpair()
                wake_reader.setblocking(False)
                wake_writer.setblocking(False)
            except OSError as exc:
                self._close_quietly(listener)
                self._close_quietly(wake_reader)
                self._close_quietly(wake_writer)
                raise TransportOpenError(
                    "无法启动 TCP Server 监听。",
                    detail=(
                        f"bind={self._config.bind_host}:{self._config.listen_port}; "
                        f"{type(exc).__name__}: {exc}"
                    ),
                ) from exc

            self._listener = listener
            self._wake_reader = wake_reader
            self._wake_writer = wake_writer
            self._clients.clear()
            self._pending_writes.clear()
            self._ready_results.clear()
            self._round_robin_cursor = 0
            self._closing = False

    def wake(self) -> None:
        """Wake a blocked worker without closing any worker-owned handle."""

        with self._lock:
            wake_writer = self._wake_writer
            closing = self._closing
        if wake_writer is None or closing:
            return
        try:
            wake_writer.send(b"x")
        except (BlockingIOError, OSError):
            pass

    def receive(self, max_bytes: int) -> ServerReadResult:
        """Wait for one bounded read, write completion, accept, or timeout."""

        if max_bytes <= 0:
            raise ConfigurationError("TCP Server receive max_bytes 必须为正数。")
        with self._lock:
            listener = self._listener
            clients = dict(self._clients)
            pending_ids = frozenset(self._pending_writes)
            wake_reader = self._wake_reader
            closing = self._closing
            if self._ready_results:
                return self._ready_results.popleft()
        if closing:
            return ServerReadResult.timeout()
        if listener is None or wake_reader is None:
            raise TransportNotOpenError("TCP Server 监听尚未打开。")

        readers: list[socket.socket] = [
            wake_reader,
            listener,
            *(client.handle for client in clients.values()),
        ]
        writers = [client.handle for client in clients.values() if client.peer_id in pending_ids]
        timeout = self._config.accept_timeout
        if clients:
            timeout = min(timeout, self._config.read_timeout)
        try:
            ready, writable, _ = select.select(readers, writers, [], timeout)
        except (OSError, ValueError) as exc:
            with self._lock:
                closing = self._closing
            if closing:
                return ServerReadResult.timeout()
            raise TransportReadError(
                "等待 TCP Server 事件失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc
        if not ready and not writable:
            return ServerReadResult.timeout()

        if wake_reader in ready:
            self._drain_wake(wake_reader)
            with self._lock:
                if self._closing:
                    return ServerReadResult.timeout()
                if self._ready_results:
                    return self._ready_results.popleft()

        ordered_clients = self._rotated_clients(clients)
        ready_client = next(
            (client for client in ordered_clients if client.handle in ready),
            None,
        )
        if ready_client is not None:
            return self._read_client(ready_client, max_bytes)

        writable_client = next(
            (
                client
                for client in ordered_clients
                if client.peer_id in pending_ids and client.handle in writable
            ),
            None,
        )
        if writable_client is not None:
            return self._flush_pending(writable_client)

        if listener in ready:
            return self._accept_one(listener)
        return ServerReadResult.timeout()

    def send(self, write: TcpServerSend) -> bool:
        """Start one write; partial data remains owned by this transport."""

        client = self._require_client(write.peer_id)
        with self._lock:
            if write.peer_id in self._pending_writes:
                return False
        try:
            sent = client.handle.send(write.write.payload)
        except BlockingIOError:
            sent = 0
        except OSError as exc:
            error_result = self._queue_client_error(client, exc)
            with self._lock:
                self._ready_results.append(error_result)
            return True

        pending = _PendingWrite(write=write, offset=sent)
        with self._lock:
            if self._clients.get(write.peer_id) is not client:
                return False
            if sent < len(write.write.payload):
                self._pending_writes[write.peer_id] = pending
            else:
                self._ready_results.append(
                    ServerReadResult.write_completed(
                        write.peer_id,
                        client.address,
                        write.write.payload,
                    )
                )
        return True

    def close(self) -> None:
        """Release all sockets; the server worker is the sole caller."""

        with self._lock:
            listener = self._listener
            clients = tuple(self._clients.values())
            wake_reader = self._wake_reader
            wake_writer = self._wake_writer
            self._listener = None
            self._clients.clear()
            self._pending_writes.clear()
            self._ready_results.clear()
            self._wake_reader = None
            self._wake_writer = None
            self._closing = True
        for client in clients:
            self._close_quietly(client.handle)
        self._close_quietly(listener)
        self._close_quietly(wake_reader)
        self._close_quietly(wake_writer)

    def _accept_one(self, listener: socket.socket) -> ServerReadResult:
        accepted = self._accept(listener)
        if accepted is None:
            return ServerReadResult.timeout()
        candidate, address = accepted
        with self._lock:
            closing = self._closing
            is_full = len(self._clients) >= self._config.max_clients
        if closing:
            self._close_quietly(candidate)
            return ServerReadResult.timeout()
        if not self._config.allows(address):
            self._close_quietly(candidate)
            return ServerReadResult.rejected(address, ServerRejectReason.UNAUTHORIZED_PEER)
        if is_full:
            self._close_quietly(candidate)
            return ServerReadResult.rejected(address, ServerRejectReason.BUSY)
        peer_id = uuid4()
        client = _PeerSocket(peer_id=peer_id, address=address, handle=candidate)
        with self._lock:
            if self._closing:
                self._close_quietly(candidate)
                return ServerReadResult.timeout()
            if len(self._clients) >= self._config.max_clients:
                self._close_quietly(candidate)
                return ServerReadResult.rejected(address, ServerRejectReason.BUSY)
            self._clients[peer_id] = client
        return ServerReadResult.client_connected(peer_id, address)

    def _read_client(self, client: _PeerSocket, max_bytes: int) -> ServerReadResult:
        with self._lock:
            if self._clients.get(client.peer_id) is not client:
                return ServerReadResult.timeout()
        try:
            payload = client.handle.recv(min(max_bytes, self._config.read_chunk_size))
        except (BlockingIOError, TimeoutError):
            return ServerReadResult.timeout()
        except OSError:
            self._drop_client(client.peer_id, client.handle)
            return ServerReadResult.client_eof(client.peer_id, client.address)
        if not payload:
            self._drop_client(client.peer_id, client.handle)
            return ServerReadResult.client_eof(client.peer_id, client.address)
        return ServerReadResult.data(client.peer_id, client.address, payload)

    def _flush_pending(self, client: _PeerSocket) -> ServerReadResult:
        with self._lock:
            if self._clients.get(client.peer_id) is not client:
                return ServerReadResult.timeout()
            pending = self._pending_writes.get(client.peer_id)
        if pending is None:
            return ServerReadResult.timeout()
        payload = pending.write.write.payload
        try:
            sent = client.handle.send(payload[pending.offset :])
        except BlockingIOError:
            return ServerReadResult.timeout()
        except OSError as exc:
            return self._queue_client_error(client, exc)
        if sent <= 0:
            return self._queue_client_error(
                client,
                OSError("TCP Server client returned an empty write"),
            )
        with self._lock:
            current = self._pending_writes.get(client.peer_id)
            if current is not pending:
                return ServerReadResult.timeout()
            pending.offset += sent
            if pending.offset < len(payload):
                return ServerReadResult.timeout()
            self._pending_writes.pop(client.peer_id, None)
        return ServerReadResult.write_completed(client.peer_id, client.address, payload)

    def _queue_client_error(
        self,
        client: _PeerSocket,
        exc: OSError,
    ) -> ServerReadResult:
        error = TransportWriteError(
            "写入 TCP Server client 失败。",
            detail=(
                f"peer_id={client.peer_id}; peer={client.address.display}; "
                f"{type(exc).__name__}: {exc}"
            ),
        ).info
        self._drop_client(client.peer_id, client.handle)
        return ServerReadResult.client_error(client.peer_id, client.address, error)

    def _require_client(self, peer_id: PeerId) -> _PeerSocket:
        with self._lock:
            if self._listener is None or self._closing:
                raise TransportNotOpenError("TCP Server 监听尚未打开。")
            client = self._clients.get(peer_id)
            if client is not None:
                return client
            if not self._clients:
                raise TcpServerNoClientError()
        raise TcpServerPeerNotConnectedError(str(peer_id))

    def _drop_client(self, peer_id: PeerId, expected: socket.socket) -> None:
        with self._lock:
            client = self._clients.get(peer_id)
            if client is None or client.handle is not expected:
                return
            del self._clients[peer_id]
            self._pending_writes.pop(peer_id, None)
        self._close_quietly(expected)

    def _pending_bytes(self, peer_id: PeerId) -> int:
        pending = self._pending_writes.get(peer_id)
        if pending is None:
            return 0
        return len(pending.write.write.payload) - pending.offset

    def _rotated_clients(
        self,
        clients: dict[PeerId, _PeerSocket],
    ) -> tuple[_PeerSocket, ...]:
        """Return a round-robin order for one bounded read/write pass."""

        ordered = tuple(clients.values())
        if not ordered:
            return ()
        with self._lock:
            start = self._round_robin_cursor % len(ordered)
            self._round_robin_cursor = (start + 1) % len(ordered)
        return ordered[start:] + ordered[:start]

    @staticmethod
    def _accept(listener: socket.socket) -> tuple[socket.socket, PeerAddress] | None:
        try:
            candidate, address = listener.accept()
        except BlockingIOError:
            return None
        except OSError as exc:
            raise TransportReadError(
                "接受 TCP Server client 失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc
        try:
            candidate.setblocking(False)
            peer = PeerAddress(str(address[0]), int(address[1]))
        except (OSError, TypeError, ValueError) as exc:
            TcpServerTransport._close_quietly(candidate)
            raise TransportReadError(
                "无法识别 TCP Server client 地址。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc
        return candidate, peer

    @staticmethod
    def _drain_wake(handle: socket.socket) -> None:
        while True:
            try:
                payload = handle.recv(64)
            except (BlockingIOError, OSError):
                return
            if not payload:
                return

    @staticmethod
    def _close_quietly(handle: socket.socket | None) -> None:
        if handle is None:
            return
        try:
            handle.close()
        except OSError:
            pass


class TcpServerTransportFactory:
    """Concrete factory kept separate from the M2a client/datagram factory."""

    def create(self, config: TcpServerTransportConfig) -> TcpServerTransport:
        return TcpServerTransport(config)
