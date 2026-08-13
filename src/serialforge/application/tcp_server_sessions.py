"""Independent multi-client TCP Server orchestration."""

from __future__ import annotations

import logging
import time
from collections import deque
from dataclasses import dataclass, field
from datetime import UTC, datetime
from threading import Event, Lock, RLock, Thread
from uuid import uuid4

from ..domain.errors import (
    ErrorCode,
    ErrorInfo,
    OutboundQueueFullError,
    SerialForgeError,
    SessionBusyError,
    SessionNotFoundError,
    SessionNotOpenError,
    SessionShutdownTimeoutError,
    TcpServerBusyError,
    TcpServerNoClientError,
    TcpServerPeerNotConnectedError,
    TcpServerPeerRejectedError,
    TransportEofError,
    TransportWriteError,
)
from ..domain.events import (
    SessionErrorEvent,
    SessionEvent,
    SessionStateChangedEvent,
    StreamDataReceivedEvent,
    StreamDataSentEvent,
    TcpServerClientChangedEvent,
)
from ..domain.models import (
    PeerAddress,
    PeerId,
    RawRecord,
    RecordDirection,
    ServerReadKind,
    ServerReadResult,
    ServerRejectReason,
    SessionId,
    SessionSnapshot,
    SessionState,
    StreamWrite,
    TcpServerClientState,
    TcpServerPeerSnapshot,
    TcpServerSend,
    TcpServerTransportConfig,
)
from ..domain.ports import (
    EventSinkPort,
    RawRecorderPort,
    SessionPort,
    TcpServerTransportFactoryPort,
    TcpServerTransportPort,
)

logger = logging.getLogger(__name__)


@dataclass(slots=True)
class _ServerRuntime:
    session_id: SessionId
    transport: TcpServerTransportPort
    stop_requested: Event
    outbound: dict[PeerId, deque[TcpServerSend]]
    outbound_bytes: dict[PeerId, int]
    inflight_bytes: dict[PeerId, int]
    send_order: deque[PeerId]
    lifecycle_lock: RLock
    inbound_chunk_size: int
    max_clients: int
    total_outbound_bytes: int = 0
    state: SessionState = SessionState.OPENING
    peers: dict[PeerId, TcpServerPeerSnapshot] = field(default_factory=dict)
    thread: Thread | None = None


class TcpServerSessionManager(SessionPort):
    """Run one listener worker with bounded peer state and explicit targets."""

    def __init__(
        self,
        *,
        transport_factory: TcpServerTransportFactoryPort,
        event_sink: EventSinkPort,
        raw_recorder: RawRecorderPort | None = None,
        inbound_chunk_size: int = 4_096,
        outbound_queue_size: int = 64,
        outbound_byte_limit: int = 8 * 1024 * 1024,
        total_outbound_byte_limit: int = 16 * 1024 * 1024,
    ) -> None:
        if not 0 < inbound_chunk_size <= 65_536:
            raise ValueError("inbound_chunk_size must be between 1 and 65536")
        if not 0 < outbound_queue_size <= 1_024:
            raise ValueError("outbound_queue_size must be between 1 and 1024")
        if not 0 < outbound_byte_limit <= 64 * 1024 * 1024:
            raise ValueError("outbound_byte_limit must be between 1 and 64 MiB")
        if not 0 < total_outbound_byte_limit <= 64 * 1024 * 1024:
            raise ValueError("total_outbound_byte_limit must be between 1 and 64 MiB")
        self._transport_factory = transport_factory
        self._event_sink = event_sink
        self._raw_recorder = raw_recorder
        self._inbound_chunk_size = inbound_chunk_size
        self._outbound_queue_size = outbound_queue_size
        self._outbound_byte_limit = outbound_byte_limit
        self._total_outbound_byte_limit = total_outbound_byte_limit
        self._lock = Lock()
        self._runtime: _ServerRuntime | None = None
        self._last_snapshot: SessionSnapshot | None = None

    def open(self, config: TcpServerTransportConfig) -> SessionId:
        """Create a server worker; bind and listen happen on that worker."""

        with self._lock:
            if self._runtime is not None:
                raise SessionBusyError()
            transport = self._transport_factory.create(config)
            runtime = _ServerRuntime(
                session_id=uuid4(),
                transport=transport,
                stop_requested=Event(),
                outbound={},
                outbound_bytes={},
                inflight_bytes={},
                send_order=deque(),
                lifecycle_lock=RLock(),
                inbound_chunk_size=getattr(config, "read_chunk_size", self._inbound_chunk_size),
                max_clients=config.max_clients,
            )
            runtime.thread = Thread(
                target=self._run,
                args=(runtime,),
                name=f"serialforge-tcp-server-{runtime.session_id}",
                daemon=True,
            )
            self._runtime = runtime
            self._last_snapshot = SessionSnapshot(
                session_id=runtime.session_id,
                endpoint=runtime.transport.endpoint,
                state=SessionState.OPENING,
            )

        self._publish_state(runtime, SessionState.OPENING)
        try:
            runtime.thread.start()
        except RuntimeError as exc:
            error = ErrorInfo(
                code=ErrorCode.UNKNOWN,
                message="无法启动 TCP Server 后台 worker。",
                recoverable=False,
                detail=str(exc),
            )
            self._transition(runtime, SessionState.ERROR, error)
            with self._lock:
                if self._runtime is runtime:
                    self._runtime = None
            raise
        return runtime.session_id

    def close(self, session_id: SessionId) -> None:
        """Request listener cancellation and wake any blocked accept/select."""

        with self._lock:
            runtime = self._runtime
            if runtime is None or runtime.session_id != session_id:
                raise SessionNotFoundError()

        with runtime.lifecycle_lock:
            with self._lock:
                if self._runtime is not runtime:
                    raise SessionNotFoundError()
                if runtime.state in {SessionState.CLOSED, SessionState.ERROR}:
                    return
                if runtime.state == SessionState.CLOSING:
                    return
                runtime.stop_requested.set()
            self._transition(runtime, SessionState.CLOSING)

        runtime.transport.wake()

    def send(self, session_id: SessionId, write: TcpServerSend) -> None:
        """Queue a stream write for one currently connected peer."""

        if not isinstance(write, TcpServerSend):
            raise TypeError("TCP Server 会话必须使用 TcpServerSend。")
        with self._lock:
            runtime = self._runtime
            if runtime is None or runtime.session_id != session_id:
                raise SessionNotFoundError()
            if runtime.state != SessionState.OPEN:
                raise SessionNotOpenError()
            if not runtime.peers:
                raise TcpServerNoClientError()
            target = runtime.peers.get(write.peer_id)
            if target is None:
                raise TcpServerPeerNotConnectedError(str(write.peer_id))
            queue = runtime.outbound.setdefault(write.peer_id, deque())
            queued_bytes = runtime.outbound_bytes.get(write.peer_id, 0)
            inflight_bytes = runtime.inflight_bytes.get(write.peer_id, 0)
            if len(queue) + bool(inflight_bytes) >= self._outbound_queue_size:
                raise OutboundQueueFullError(f"TCP Server client {target.display} 的发送队列已满。")
            peer_byte_limit = min(
                self._outbound_byte_limit,
                self._total_outbound_byte_limit // runtime.max_clients,
            )
            if queued_bytes + inflight_bytes + len(write.write.payload) > peer_byte_limit:
                raise OutboundQueueFullError(
                    f"TCP Server client {target.display} 的发送字节上限已满。"
                )
            if (
                runtime.total_outbound_bytes + len(write.write.payload)
                > self._total_outbound_byte_limit
            ):
                raise OutboundQueueFullError("TCP Server 总发送队列字节上限已满。")
            queue.append(write)
            runtime.outbound_bytes[write.peer_id] = queued_bytes + len(write.write.payload)
            runtime.total_outbound_bytes += len(write.write.payload)
        runtime.transport.wake()

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the listener worker within a bounded shutdown budget."""

        if timeout is not None and timeout < 0:
            raise ValueError("timeout must be non-negative or None")
        with self._lock:
            runtime = self._runtime
        if runtime is None:
            return
        try:
            self.close(runtime.session_id)
        except SessionNotFoundError:
            return
        if runtime.thread is not None:
            runtime.thread.join(timeout=timeout)
            if runtime.thread.is_alive():
                raise SessionShutdownTimeoutError("TCP Server worker 未能在关闭时限内退出。")

    def snapshot(self) -> SessionSnapshot | None:
        with self._lock:
            return self._last_snapshot

    def _run(self, runtime: _ServerRuntime) -> None:
        try:
            if runtime.stop_requested.is_set():
                return
            runtime.transport.open()
            with runtime.lifecycle_lock:
                if runtime.stop_requested.is_set():
                    return
                self._transition(runtime, SessionState.OPEN)
            while not runtime.stop_requested.is_set():
                self._send_one(runtime)
                if runtime.stop_requested.is_set():
                    break
                result = runtime.transport.receive(runtime.inbound_chunk_size)
                self._handle_read(runtime, result)
        except Exception as exc:
            if not runtime.stop_requested.is_set():
                error = self._error_info(exc)
                self._publish(
                    SessionErrorEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        error=error,
                        occurred_at=time.monotonic(),
                        peer=None,
                    )
                )
                self._transition(runtime, SessionState.ERROR, error)
        finally:
            try:
                runtime.transport.close()
            except Exception as exc:
                error = self._error_info(exc, fallback_message="关闭 TCP Server 时发生错误。")
                self._publish(
                    SessionErrorEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        error=error,
                        occurred_at=time.monotonic(),
                        peer=None,
                    )
                )
                self._transition(runtime, SessionState.ERROR, error)

            with runtime.lifecycle_lock, self._lock:
                state = runtime.state
            if runtime.stop_requested.is_set() and state not in {
                SessionState.ERROR,
                SessionState.CLOSED,
            }:
                self._transition(runtime, SessionState.CLOSED)

            with self._lock:
                if self._runtime is runtime:
                    self._last_snapshot = SessionSnapshot(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        state=runtime.state,
                        error=self._last_snapshot.error if self._last_snapshot else None,
                    )
                    self._runtime = None

    def _handle_read(self, runtime: _ServerRuntime, result: ServerReadResult) -> None:
        if result.kind is ServerReadKind.TIMEOUT:
            return
        if result.peer is None:
            raise ValueError("TCP Server read 结果缺少 peer。")
        if result.kind is ServerReadKind.CLIENT_CONNECTED:
            if result.peer_id is None:
                raise ValueError("TCP Server connected 结果缺少 peer_id。")
            self._add_peer(runtime, result.peer_id, result.peer)
            self._publish(
                TcpServerClientChangedEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    state=TcpServerClientState.CONNECTED,
                    peer_id=result.peer_id,
                    peer=result.peer,
                    occurred_at=time.monotonic(),
                    peers=self._peer_snapshot(runtime),
                )
            )
            return
        if result.kind is ServerReadKind.PEER_REJECTED:
            if result.reason is None:
                raise ValueError("TCP Server reject 结果缺少 reason。")
            error = (
                TcpServerPeerRejectedError(result.peer.display)
                if result.reason is ServerRejectReason.UNAUTHORIZED_PEER
                else TcpServerBusyError(result.peer.display)
            )
            self._publish(
                SessionErrorEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    error=error.info,
                    occurred_at=time.monotonic(),
                    peer=result.peer,
                )
            )
            return
        if result.kind is ServerReadKind.CLIENT_EOF:
            if result.peer_id is None:
                raise ValueError("TCP Server EOF 结果缺少 peer_id。")
            self._remove_peer(runtime, result.peer_id)
            error = TransportEofError(
                message="TCP client 已断开，监听器继续等待下一个客户端。",
                detail=f"peer={result.peer.display}",
            )
            self._publish(
                TcpServerClientChangedEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    state=TcpServerClientState.DISCONNECTED,
                    peer_id=result.peer_id,
                    peer=result.peer,
                    occurred_at=time.monotonic(),
                    error=error.info,
                    peers=self._peer_snapshot(runtime),
                )
            )
            return
        if result.kind is ServerReadKind.CLIENT_ERROR:
            if result.peer_id is None or result.error is None:
                raise ValueError("TCP Server client error 结果缺少 peer_id/error。")
            self._remove_peer(runtime, result.peer_id)
            self._publish(
                SessionErrorEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    error=result.error,
                    occurred_at=time.monotonic(),
                    peer=result.peer,
                    peer_id=result.peer_id,
                )
            )
            self._publish(
                TcpServerClientChangedEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    state=TcpServerClientState.DISCONNECTED,
                    peer_id=result.peer_id,
                    peer=result.peer,
                    occurred_at=time.monotonic(),
                    error=result.error,
                    peers=self._peer_snapshot(runtime),
                )
            )
            return
        if result.kind is ServerReadKind.WRITE_COMPLETED:
            if result.peer_id is None or result.chunk is None:
                raise ValueError("TCP Server write completed 结果缺少 peer_id/payload。")
            payload = result.chunk.payload
            with self._lock:
                inflight_bytes = runtime.inflight_bytes.pop(result.peer_id, 0)
                runtime.total_outbound_bytes = max(
                    0,
                    runtime.total_outbound_bytes - inflight_bytes,
                )
            self._capture_raw(
                runtime,
                RecordDirection.SEND,
                payload,
                peer=result.peer,
                peer_id=result.peer_id,
            )
            self._publish(
                StreamDataSentEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    write=StreamWrite(payload),
                    occurred_at=time.monotonic(),
                    peer=result.peer,
                    peer_id=result.peer_id,
                )
            )
            return
        if result.kind is not ServerReadKind.DATA or result.chunk is None:
            raise ValueError("TCP Server DATA 结果缺少 payload。")
        if result.peer_id is None:
            raise ValueError("TCP Server DATA 结果缺少 peer_id。")
        self._capture_raw(
            runtime,
            RecordDirection.RECEIVE,
            result.chunk.payload,
            peer=result.peer,
            peer_id=result.peer_id,
        )
        self._publish(
            StreamDataReceivedEvent(
                session_id=runtime.session_id,
                endpoint=runtime.transport.endpoint,
                chunk=result.chunk,
                occurred_at=time.monotonic(),
                peer=result.peer,
                peer_id=result.peer_id,
            )
        )

    def _send_one(self, runtime: _ServerRuntime) -> None:
        attempts = 0
        while True:
            with self._lock:
                write = None
                peer_count = len(runtime.send_order)
                if attempts >= peer_count:
                    return
                for _ in range(peer_count):
                    peer_id = runtime.send_order.popleft()
                    runtime.send_order.append(peer_id)
                    queue = runtime.outbound.get(peer_id)
                    if queue:
                        write = queue.popleft()
                        runtime.outbound_bytes[peer_id] -= len(write.write.payload)
                        runtime.total_outbound_bytes -= len(write.write.payload)
                        break
            if write is None:
                return
            if runtime.stop_requested.is_set():
                return
            peer_id = write.peer_id
            with self._lock:
                peer_snapshot = runtime.peers.get(peer_id)
            peer = peer_snapshot.address if peer_snapshot is not None else None
            try:
                accepted = runtime.transport.send(write)
            except (
                TcpServerNoClientError,
                TcpServerPeerNotConnectedError,
                TransportWriteError,
            ) as exc:
                self._publish(
                    SessionErrorEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        error=exc.info,
                        occurred_at=time.monotonic(),
                        peer=peer,
                        peer_id=peer_id,
                    )
                )
                if isinstance(exc, (TcpServerPeerNotConnectedError, TransportWriteError)):
                    self._remove_peer(runtime, peer_id)
                    if peer is not None:
                        self._publish(
                            TcpServerClientChangedEvent(
                                session_id=runtime.session_id,
                                endpoint=runtime.transport.endpoint,
                                state=TcpServerClientState.DISCONNECTED,
                                peer_id=peer_id,
                                peer=peer,
                                occurred_at=time.monotonic(),
                                error=exc.info,
                                peers=self._peer_snapshot(runtime),
                            )
                        )
                return
            if accepted:
                with self._lock:
                    if peer_id in runtime.peers:
                        runtime.inflight_bytes[peer_id] = runtime.inflight_bytes.get(
                            peer_id, 0
                        ) + len(write.write.payload)
                        runtime.total_outbound_bytes += len(write.write.payload)
                return
            with self._lock:
                if peer_id in runtime.peers:
                    queue = runtime.outbound.setdefault(peer_id, deque())
                    queue.appendleft(write)
                    runtime.outbound_bytes[peer_id] = runtime.outbound_bytes.get(peer_id, 0) + len(
                        write.write.payload
                    )
                    runtime.total_outbound_bytes += len(write.write.payload)
            attempts += 1

    def _add_peer(self, runtime: _ServerRuntime, peer_id: PeerId, address: PeerAddress) -> None:
        with self._lock:
            if peer_id in runtime.peers:
                return
            runtime.peers[peer_id] = TcpServerPeerSnapshot(peer_id=peer_id, address=address)
            runtime.outbound[peer_id] = deque()
            runtime.outbound_bytes[peer_id] = 0
            runtime.inflight_bytes[peer_id] = 0
            runtime.send_order.append(peer_id)

    def _remove_peer(self, runtime: _ServerRuntime, peer_id: PeerId) -> None:
        with self._lock:
            runtime.peers.pop(peer_id, None)
            pending_bytes = runtime.outbound_bytes.pop(peer_id, 0)
            inflight_bytes = runtime.inflight_bytes.pop(peer_id, 0)
            runtime.outbound.pop(peer_id, None)
            runtime.total_outbound_bytes = max(
                0,
                runtime.total_outbound_bytes - pending_bytes - inflight_bytes,
            )
            try:
                runtime.send_order.remove(peer_id)
            except ValueError:
                pass

    def _peer_snapshot(self, runtime: _ServerRuntime) -> tuple[TcpServerPeerSnapshot, ...]:
        with self._lock:
            return tuple(
                TcpServerPeerSnapshot(
                    peer_id=peer_id,
                    address=peer.address,
                    state=peer.state,
                    queued_items=len(runtime.outbound.get(peer_id, ()))
                    + bool(runtime.inflight_bytes.get(peer_id, 0)),
                    queued_bytes=runtime.outbound_bytes.get(peer_id, 0)
                    + runtime.inflight_bytes.get(peer_id, 0),
                )
                for peer_id, peer in sorted(
                    runtime.peers.items(),
                    key=lambda item: (item[1].address.host, item[1].address.port),
                )
            )

    def _capture_raw(
        self,
        runtime: _ServerRuntime,
        direction: RecordDirection,
        payload: bytes,
        *,
        peer: PeerAddress | None,
        peer_id: PeerId | None,
    ) -> None:
        if self._raw_recorder is None:
            return
        try:
            self._raw_recorder.record(
                RawRecord(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    direction=direction,
                    payload=payload,
                    occurred_at=time.monotonic(),
                    wall_time=datetime.now(UTC),
                    peer=peer,
                    peer_id=peer_id,
                )
            )
        except Exception:
            logger.exception("SerialForge raw recorder rejected a TCP Server record")

    def _publish_state(
        self,
        runtime: _ServerRuntime,
        state: SessionState,
        error: ErrorInfo | None = None,
    ) -> None:
        self._publish(
            SessionStateChangedEvent(
                session_id=runtime.session_id,
                endpoint=runtime.transport.endpoint,
                state=state,
                occurred_at=time.monotonic(),
                error=error,
            )
        )

    def _transition(
        self,
        runtime: _ServerRuntime,
        state: SessionState,
        error: ErrorInfo | None = None,
    ) -> None:
        with runtime.lifecycle_lock:
            with self._lock:
                if self._runtime is not runtime:
                    return
                if runtime.state == state:
                    return
                runtime.state = state
                previous_error = (
                    self._last_snapshot.error if self._last_snapshot is not None else None
                )
                self._last_snapshot = SessionSnapshot(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    state=state,
                    error=error if error is not None else previous_error,
                )
            self._publish_state(runtime, state, error)

    def _publish(self, event: SessionEvent) -> None:
        try:
            self._event_sink.publish(event)
        except Exception:
            logger.exception("SerialForge event sink failed")

    @staticmethod
    def _error_info(
        exc: Exception,
        *,
        fallback_message: str = "TCP Server 会话因未处理错误停止。",
    ) -> ErrorInfo:
        if isinstance(exc, SerialForgeError):
            return exc.info
        return ErrorInfo(
            code=ErrorCode.UNKNOWN,
            message=fallback_message,
            recoverable=False,
            detail=f"{type(exc).__name__}: {exc}",
        )
