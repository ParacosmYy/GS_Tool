"""Single-device BLE GATT orchestration on one dedicated asyncio worker."""

from __future__ import annotations

import asyncio
import logging
import time
from dataclasses import dataclass
from datetime import UTC, datetime
from queue import Empty, Full, Queue
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
    TransportEofError,
)
from ..domain.events import (
    BleGattNotificationEvent,
    BleGattReadCompletedEvent,
    BleGattServicesChangedEvent,
    BleGattSubscriptionChangedEvent,
    BleGattWriteCompletedEvent,
    SessionErrorEvent,
    SessionEvent,
    SessionStateChangedEvent,
)
from ..domain.models import (
    BleGattCommand,
    BleGattNotify,
    BleGattRead,
    BleGattTransportConfig,
    BleGattWrite,
    RawRecord,
    RecordDirection,
    SessionId,
    SessionSnapshot,
    SessionState,
)
from ..domain.ports import (
    BleGattTransportFactoryPort,
    BleGattTransportPort,
    EventSinkPort,
    RawRecorderPort,
    SessionPort,
    TransportWrite,
)

logger = logging.getLogger(__name__)


@dataclass(slots=True)
class _GattRuntime:
    session_id: SessionId
    transport: BleGattTransportPort
    stop_requested: Event
    commands: Queue[BleGattCommand | None]
    lifecycle_lock: RLock
    state: SessionState = SessionState.OPENING
    thread: Thread | None = None
    loop: asyncio.AbstractEventLoop | None = None
    task: asyncio.Task[None] | None = None
    disconnect_error: ErrorInfo | None = None


class BleGattSessionManager(SessionPort):
    """Own a single Bleak client and all related asyncio resources."""

    def __init__(
        self,
        *,
        transport_factory: BleGattTransportFactoryPort,
        event_sink: EventSinkPort,
        raw_recorder: RawRecorderPort | None = None,
        outbound_queue_size: int = 64,
    ) -> None:
        if not 0 < outbound_queue_size <= 1_024:
            raise ValueError("outbound_queue_size must be between 1 and 1024")
        self._transport_factory = transport_factory
        self._event_sink = event_sink
        self._raw_recorder = raw_recorder
        self._outbound_queue_size = outbound_queue_size
        self._lock = Lock()
        self._runtime: _GattRuntime | None = None
        self._last_snapshot: SessionSnapshot | None = None

    def open(self, config: BleGattTransportConfig) -> SessionId:
        """Create a lazy BLE worker; all backend I/O starts on that worker."""

        with self._lock:
            if self._runtime is not None:
                raise SessionBusyError()
            transport = self._transport_factory.create(config)
            runtime = _GattRuntime(
                session_id=uuid4(),
                transport=transport,
                stop_requested=Event(),
                commands=Queue(maxsize=self._outbound_queue_size),
                lifecycle_lock=RLock(),
            )
            transport.set_handlers(
                lambda characteristic, payload, item=runtime: self._on_notification(
                    item, characteristic, payload
                ),
                lambda item=runtime: self._on_disconnected(item),
            )
            runtime.thread = Thread(
                target=self._run,
                args=(runtime,),
                name=f"serialforge-ble-{runtime.session_id}",
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
                message="无法启动 BLE GATT 后台 worker。",
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
        """Cancel the asyncio task; the worker closes Bleak objects in its finally path."""

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
                if runtime.state != SessionState.CLOSING:
                    runtime.stop_requested.set()
                    self._transition(runtime, SessionState.CLOSING)
            self._wake_runtime(runtime, cancel_task=True)

    def send(self, session_id: SessionId, write: TransportWrite) -> None:
        """Enqueue one typed GATT command without crossing into the BLE loop."""

        if not isinstance(write, (BleGattRead, BleGattWrite, BleGattNotify)):
            raise TypeError("BLE GATT 会话必须使用 BleGattCommand。")
        with self._lock:
            runtime = self._runtime
            if runtime is None or runtime.session_id != session_id:
                raise SessionNotFoundError()
            if runtime.state != SessionState.OPEN:
                raise SessionNotOpenError()
            try:
                runtime.commands.put_nowait(write)
            except Full as exc:
                raise OutboundQueueFullError("BLE GATT 操作队列已满，请稍后重试。") from exc

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the BLE worker within the caller's bounded shutdown budget."""

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
                raise SessionShutdownTimeoutError("BLE GATT worker 未能在关闭时限内退出。")

    def snapshot(self) -> SessionSnapshot | None:
        with self._lock:
            return self._last_snapshot

    def _run(self, runtime: _GattRuntime) -> None:
        try:
            asyncio.run(self._run_async(runtime))
        except asyncio.CancelledError:
            pass
        except Exception as exc:
            if not runtime.stop_requested.is_set():
                error = self._error_info(exc, fallback_message="BLE GATT 会话启动或运行失败。")
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
            with self._lock:
                disconnect_error = runtime.disconnect_error
            if disconnect_error is not None:
                self._publish(
                    SessionErrorEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        error=disconnect_error,
                        occurred_at=time.monotonic(),
                    )
                )
                self._transition(runtime, SessionState.ERROR, disconnect_error)
            elif runtime.stop_requested.is_set() and runtime.state not in {
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

    async def _run_async(self, runtime: _GattRuntime) -> None:
        runtime.loop = asyncio.get_running_loop()
        runtime.task = asyncio.current_task()
        try:
            services = await runtime.transport.open()
            if runtime.stop_requested.is_set():
                return
            self._transition(runtime, SessionState.OPEN)
            self._publish(
                BleGattServicesChangedEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    services=services,
                    mtu_size=runtime.transport.mtu_size,
                    occurred_at=time.monotonic(),
                )
            )
            while not runtime.stop_requested.is_set():
                try:
                    command = await asyncio.to_thread(runtime.commands.get, True, 0.1)
                except Empty:
                    continue
                if command is None:
                    break
                await self._execute(runtime, command)
        except asyncio.CancelledError:
            if not runtime.stop_requested.is_set():
                raise
        finally:
            await runtime.transport.close()

    async def _execute(self, runtime: _GattRuntime, command: BleGattCommand) -> None:
        try:
            if isinstance(command, BleGattRead):
                payload = await runtime.transport.read(command.characteristic)
                self._capture_raw(
                    runtime,
                    RecordDirection.RECEIVE,
                    payload,
                    channel=command.characteristic.key,
                )
                self._publish(
                    BleGattReadCompletedEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        characteristic=command.characteristic,
                        payload=payload,
                        occurred_at=time.monotonic(),
                    )
                )
            elif isinstance(command, BleGattWrite):
                await runtime.transport.write(command)
                self._capture_raw(
                    runtime,
                    RecordDirection.SEND,
                    command.payload,
                    channel=command.characteristic.key,
                )
                self._publish(
                    BleGattWriteCompletedEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        write=command,
                        occurred_at=time.monotonic(),
                    )
                )
            else:
                await runtime.transport.set_notify(command.characteristic, command.enabled)
                self._publish(
                    BleGattSubscriptionChangedEvent(
                        session_id=runtime.session_id,
                        endpoint=runtime.transport.endpoint,
                        characteristic=command.characteristic,
                        enabled=command.enabled,
                        occurred_at=time.monotonic(),
                    )
                )
        except SerialForgeError as exc:
            self._publish(
                SessionErrorEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    error=exc.info,
                    occurred_at=time.monotonic(),
                    characteristic=command.characteristic,
                )
            )
        except Exception as exc:
            error = self._error_info(exc, fallback_message="BLE GATT 操作失败。")
            self._publish(
                SessionErrorEvent(
                    session_id=runtime.session_id,
                    endpoint=runtime.transport.endpoint,
                    error=error,
                    occurred_at=time.monotonic(),
                    characteristic=command.characteristic,
                )
            )

    def _on_notification(self, runtime: _GattRuntime, characteristic, payload: bytes) -> None:
        if runtime.stop_requested.is_set():
            return
        self._capture_raw(
            runtime,
            RecordDirection.RECEIVE,
            payload,
            channel=characteristic.key,
        )
        self._publish(
            BleGattNotificationEvent(
                session_id=runtime.session_id,
                endpoint=runtime.transport.endpoint,
                characteristic=characteristic,
                payload=bytes(payload),
                occurred_at=time.monotonic(),
            )
        )

    def _on_disconnected(self, runtime: _GattRuntime) -> None:
        if runtime.stop_requested.is_set():
            return
        error = TransportEofError("BLE 设备已断开连接，请手动重新扫描并连接。")
        with self._lock:
            runtime.disconnect_error = error.info
            runtime.stop_requested.set()
        try:
            runtime.commands.put_nowait(None)
        except Full:
            pass

    def _wake_runtime(self, runtime: _GattRuntime, *, cancel_task: bool) -> None:
        try:
            runtime.commands.put_nowait(None)
        except Full:
            pass
        loop = runtime.loop
        task = runtime.task
        if cancel_task and loop is not None and task is not None:
            loop.call_soon_threadsafe(task.cancel)

    def _capture_raw(
        self,
        runtime: _GattRuntime,
        direction: RecordDirection,
        payload: bytes,
        *,
        channel: str,
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
                    channel=channel,
                )
            )
        except Exception:
            logger.exception("SerialForge raw recorder rejected a BLE GATT record")

    def _publish_state(
        self,
        runtime: _GattRuntime,
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
        runtime: _GattRuntime,
        state: SessionState,
        error: ErrorInfo | None = None,
    ) -> None:
        with runtime.lifecycle_lock:
            with self._lock:
                if self._runtime is not runtime or runtime.state == state:
                    return
                runtime.state = state
                previous_error = self._last_snapshot.error if self._last_snapshot else None
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
            logger.exception("SerialForge BLE GATT event sink failed")

    @staticmethod
    def _error_info(
        exc: Exception,
        *,
        fallback_message: str,
    ) -> ErrorInfo:
        if isinstance(exc, SerialForgeError):
            return exc.info
        return ErrorInfo(
            code=ErrorCode.UNKNOWN,
            message=fallback_message,
            recoverable=False,
            detail=f"{type(exc).__name__}: {exc}",
        )
