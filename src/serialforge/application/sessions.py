"""Cancellable, single-session orchestration independent of Qt and device libs."""

from __future__ import annotations

import logging
import time
from dataclasses import dataclass
from datetime import UTC, datetime
from queue import Empty, Full, Queue
from threading import Event, Lock, RLock, Thread
from uuid import uuid4

from ..domain.errors import (
    ConfigurationError,
    DatagramPeerRejectedError,
    DatagramTooLargeError,
    ErrorCode,
    ErrorInfo,
    OutboundQueueFullError,
    SerialForgeError,
    SessionBusyError,
    SessionNotFoundError,
    SessionNotOpenError,
    SessionShutdownTimeoutError,
    TransportEofError,
)
from ..domain.events import (
    DatagramDataReceivedEvent,
    DatagramDataSentEvent,
    DatagramDroppedEvent,
    SessionErrorEvent,
    SessionEvent,
    SessionStateChangedEvent,
    StreamDataReceivedEvent,
    StreamDataSentEvent,
)
from ..domain.models import (
    DatagramReadKind,
    DatagramReadResult,
    DatagramSend,
    PeerAddress,
    RawRecord,
    RecordDirection,
    RttDownWrite,
    SessionId,
    SessionSnapshot,
    SessionState,
    StreamReadKind,
    StreamReadResult,
    StreamWrite,
    TransportConfig,
    TransportKind,
)
from ..domain.ports import (
    EventSinkPort,
    RawRecorderPort,
    SessionPort,
    TransportFactoryPort,
    TransportPort,
    TransportWrite,
)
from ..domain.timing import GapObservation, TimingQuality

logger = logging.getLogger(__name__)


@dataclass(slots=True)
class _SessionRuntime:
    session_id: SessionId
    transport: TransportPort
    stop_requested: Event
    outbound: Queue[TransportWrite]
    lifecycle_lock: RLock
    inbound_chunk_size: int
    outbound_bytes: int = 0
    state: SessionState = SessionState.OPENING
    thread: Thread | None = None
    last_stream_at: float | None = None


class SessionManager(SessionPort):
    """Run one transport on a dedicated worker and expose asynchronous commands."""

    def __init__(
        self,
        *,
        transport_factory: TransportFactoryPort,
        event_sink: EventSinkPort,
        raw_recorder: RawRecorderPort | None = None,
        inbound_chunk_size: int = 4_096,
        outbound_queue_size: int = 64,
        outbound_byte_limit: int = 8 * 1024 * 1024,
    ) -> None:
        if not 0 < inbound_chunk_size <= 65_536:
            raise ValueError("inbound_chunk_size must be between 1 and 65536")
        if not 0 < outbound_queue_size <= 1_024:
            raise ValueError("outbound_queue_size must be between 1 and 1024")
        if not 0 < outbound_byte_limit <= 64 * 1024 * 1024:
            raise ValueError("outbound_byte_limit must be between 1 and 64 MiB")
        self._transport_factory = transport_factory
        self._event_sink = event_sink
        self._raw_recorder = raw_recorder
        self._inbound_chunk_size = inbound_chunk_size
        self._outbound_queue_size = outbound_queue_size
        self._outbound_byte_limit = outbound_byte_limit
        self._lock = Lock()
        self._runtime: _SessionRuntime | None = None
        self._last_snapshot: SessionSnapshot | None = None

    def open(self, config: TransportConfig) -> SessionId:
        """Create a worker and return before adapter I/O begins."""

        with self._lock:
            if self._runtime is not None:
                # Keep the slot occupied until the old worker has completed
                # its final cleanup.  A not-yet-started Thread is still busy.
                raise SessionBusyError()
            # Factory creation is deliberately inside the slot lock. It must
            # only validate/build an adapter; actual device I/O starts below
            # on the dedicated worker.
            transport = self._transport_factory.create(config)
            runtime = _SessionRuntime(
                session_id=uuid4(),
                transport=transport,
                stop_requested=Event(),
                outbound=Queue(maxsize=self._outbound_queue_size),
                lifecycle_lock=RLock(),
                inbound_chunk_size=getattr(config, "read_chunk_size", self._inbound_chunk_size),
            )
            runtime.thread = Thread(
                target=self._run,
                args=(runtime,),
                name=f"serialforge-{runtime.session_id}",
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
                message="无法启动会话后台 worker。",
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
        """Request cancellation without waiting on transport I/O."""

        with self._lock:
            runtime = self._runtime
            if runtime is None or runtime.session_id != session_id:
                raise SessionNotFoundError()

        # Hold the lifecycle lock while setting the stop flag and publishing
        # CLOSING.  The worker cannot publish CLOSED until this transition is
        # complete, even when close races with receive() returning.
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

        # Closing is the adapter's idempotent cancellation primitive. It is
        # deliberately best-effort here so a long network read is woken
        # promptly; the worker repeats the close in its finally block and
        # remains the owner of the final CLOSED transition.
        try:
            runtime.transport.close()
        except Exception:
            logger.exception("SerialForge transport cancellation close failed")

    def send(self, session_id: SessionId, write: TransportWrite) -> None:
        """Enqueue a write with a hard bound; no device call occurs here."""

        with self._lock:
            runtime = self._runtime
            if runtime is None or runtime.session_id != session_id:
                raise SessionNotFoundError()
            if runtime.state != SessionState.OPEN:
                raise SessionNotOpenError()
            if runtime.transport.kind is TransportKind.UDP_DATAGRAM:
                if not isinstance(write, DatagramSend):
                    raise ConfigurationError("UDP 会话必须使用 DatagramSend。")
            elif runtime.transport.kind is TransportKind.RTT:
                if not isinstance(write, RttDownWrite):
                    raise ConfigurationError("RTT 会话必须使用 RttDownWrite。")
                channel = dict(runtime.transport.endpoint.metadata).get("channel")
                if channel != str(write.channel):
                    raise ConfigurationError("RTT Down write channel 与当前会话不匹配。")
            elif not isinstance(write, StreamWrite):
                raise ConfigurationError("UART/TCP 会话必须使用 StreamWrite。")
            if runtime.outbound_bytes + len(write.payload) > self._outbound_byte_limit:
                raise OutboundQueueFullError()
            try:
                runtime.outbound.put_nowait(write)
            except Full as exc:
                raise OutboundQueueFullError() from exc
            runtime.outbound_bytes += len(write.payload)

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the active worker, bounded by the caller's shutdown budget."""

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
                raise SessionShutdownTimeoutError()

    def snapshot(self) -> SessionSnapshot | None:
        """Return a lock-protected immutable snapshot."""

        with self._lock:
            return self._last_snapshot

    def _run(self, runtime: _SessionRuntime) -> None:
        try:
            if runtime.stop_requested.is_set():
                return

            runtime.transport.open()
            # Closing may race with a successful adapter open. Re-check the
            # cancellation flag while holding the same lifecycle lock used by
            # close(), so CLOSING can never be followed by OPEN.
            with runtime.lifecycle_lock:
                if runtime.stop_requested.is_set():
                    return
                self._transition(runtime, SessionState.OPEN)
            while not runtime.stop_requested.is_set():
                self._send_one(runtime)
                if runtime.stop_requested.is_set():
                    break

                result = runtime.transport.receive(runtime.inbound_chunk_size)
                if isinstance(result, StreamReadResult):
                    self._handle_stream_read(runtime, result)
                elif isinstance(result, DatagramReadResult):
                    self._handle_datagram_read(runtime, result)
                else:
                    raise ConfigurationError("传输适配器返回了未知的读取结果。")
        except Exception as exc:
            if not runtime.stop_requested.is_set():
                error = self._error_info(exc)
                self._publish(
                    SessionErrorEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        error=error,
                        occurred_at=time.monotonic(),
                    )
                )
                self._transition(runtime, SessionState.ERROR, error)
        finally:
            try:
                runtime.transport.close()
            except Exception as exc:
                error = self._error_info(exc, fallback_message="关闭传输时发生错误。")
                self._publish(
                    SessionErrorEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        error=error,
                        occurred_at=time.monotonic(),
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

    def _handle_stream_read(
        self,
        runtime: _SessionRuntime,
        result: StreamReadResult,
    ) -> None:
        if result.kind is StreamReadKind.TIMEOUT:
            return
        if result.kind is StreamReadKind.EOF:
            raise TransportEofError(
                detail=(f"peer={result.peer.display}" if result.peer is not None else None)
            )
        if result.chunk is None:
            raise ConfigurationError("DATA stream read 缺少 payload。")
        payload = result.chunk.payload
        occurred_at = time.monotonic()
        previous_stream_at = runtime.last_stream_at
        runtime.last_stream_at = occurred_at
        gap_before = (
            None if previous_stream_at is None else max(0.0, occurred_at - previous_stream_at)
        )
        self._capture_raw(runtime, RecordDirection.RECEIVE, payload, peer=result.peer)
        self._publish(
            StreamDataReceivedEvent(
                session_id=runtime.session_id,
                endpoint=runtime.transport.endpoint,
                chunk=result.chunk,
                occurred_at=occurred_at,
                peer=result.peer,
                timing=GapObservation(
                    gap_before=gap_before,
                    quality=(
                        TimingQuality.HOST_READ_GAP
                        if gap_before is not None
                        else TimingQuality.NONE
                    ),
                ),
            )
        )

    def _handle_datagram_read(
        self,
        runtime: _SessionRuntime,
        result: DatagramReadResult,
    ) -> None:
        if result.kind is DatagramReadKind.TIMEOUT:
            return
        if result.kind is DatagramReadKind.DROPPED:
            if result.peer is None or result.reason is None:
                raise ConfigurationError("DROPPED UDP read 缺少 peer 或 reason。")
            self._publish(
                DatagramDroppedEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    peer=result.peer,
                    reason=result.reason,
                    occurred_at=time.monotonic(),
                )
            )
            return
        if result.datagram is None:
            raise ConfigurationError("DATA UDP read 缺少 datagram。")
        datagram = result.datagram
        if datagram.payload:
            self._capture_raw(
                runtime,
                RecordDirection.RECEIVE,
                datagram.payload,
                peer=datagram.peer,
            )
        self._publish(
            DatagramDataReceivedEvent(
                session_id=runtime.session_id,
                endpoint=runtime.transport.endpoint,
                datagram=datagram,
                occurred_at=time.monotonic(),
            )
        )

    def _send_one(self, runtime: _SessionRuntime) -> None:
        with self._lock:
            try:
                write = runtime.outbound.get_nowait()
            except Empty:
                return
            runtime.outbound_bytes -= len(write.payload)

        if runtime.stop_requested.is_set():
            return
        adapter_write = write.write if isinstance(write, RttDownWrite) else write
        try:
            runtime.transport.send(adapter_write)
        except (DatagramPeerRejectedError, DatagramTooLargeError) as exc:
            # A rejected UDP command must be visible but must not tear down a
            # healthy socket or create a partial raw record.
            self._publish(
                SessionErrorEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    error=exc.info,
                    occurred_at=time.monotonic(),
                )
            )
            return

        peer = (
            write.peer
            if isinstance(write, DatagramSend)
            else getattr(runtime.transport, "peer", None)
        )
        self._capture_raw(runtime, RecordDirection.SEND, adapter_write.payload, peer=peer)
        if isinstance(write, DatagramSend):
            self._publish(
                DatagramDataSentEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    write=adapter_write,
                    occurred_at=time.monotonic(),
                )
            )
        else:
            self._publish(
                StreamDataSentEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    write=adapter_write,
                    occurred_at=time.monotonic(),
                    peer=peer,
                )
            )

    def _capture_raw(
        self,
        runtime: _SessionRuntime,
        direction: RecordDirection,
        payload: bytes,
        *,
        peer: PeerAddress | None = None,
    ) -> None:
        """Offer raw bytes to the independent recorder queue before preview delivery."""

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
                )
            )
        except Exception:
            logger.exception("SerialForge raw recorder rejected a stream record")

    def _publish_state(
        self,
        runtime: _SessionRuntime,
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
        runtime: _SessionRuntime,
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
        """Keep event delivery failures from stopping device I/O."""

        try:
            self._event_sink.publish(event)
        except Exception:
            logger.exception("SerialForge event sink failed")

    @staticmethod
    def _error_info(
        exc: Exception,
        *,
        fallback_message: str = "会话因未处理错误停止。",
    ) -> ErrorInfo:
        if isinstance(exc, SerialForgeError):
            return exc.info
        return ErrorInfo(
            code=ErrorCode.UNKNOWN,
            message=fallback_message,
            recoverable=False,
            detail=f"{type(exc).__name__}: {exc}",
        )
