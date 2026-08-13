"""Bounded application services shared by workers and presentation adapters."""

from __future__ import annotations

import json
import logging
import time
from base64 import b64encode
from collections import OrderedDict, deque
from pathlib import Path
from queue import Empty, Full, Queue
from threading import Event, Lock, Thread

from ..domain.errors import (
    ConfigurationError,
    ErrorCode,
    ErrorInfo,
    RecordingBusyError,
    RecordingShutdownTimeoutError,
    SerialForgeError,
)
from ..domain.events import RecordingBackpressureEvent, RecordingStateChangedEvent
from ..domain.models import (
    MAX_COMMAND_HISTORY_ITEMS,
    MAX_ENDPOINT_IDENTITIES,
    MAX_QUICK_COMMAND_ITEMS,
    MAX_RAW_RECORD_BYTES,
    MAX_RECORD_FILE_BYTES,
    MAX_RECORD_PATH_LENGTH,
    MAX_RECORD_QUEUE_BYTES,
    CommandEntry,
    Endpoint,
    RawRecord,
    RecordingSnapshot,
    RecordingState,
)
from ..domain.ports import (
    CommandHistoryPort,
    EndpointIdentityPort,
    EventSinkPort,
    RawRecorderPort,
)

logger = logging.getLogger(__name__)


class BoundedRawRecorder(RawRecorderPort):
    """Write raw records on a bounded worker queue without touching Qt."""

    def __init__(
        self,
        *,
        event_sink: EventSinkPort | None = None,
        queue_capacity: int = 256,
        queue_byte_limit: int = MAX_RECORD_QUEUE_BYTES,
        max_file_bytes: int = MAX_RECORD_FILE_BYTES,
        max_record_bytes: int = MAX_RAW_RECORD_BYTES,
    ) -> None:
        if not 0 < queue_capacity <= 4_096:
            raise ValueError("record queue capacity must be between 1 and 4096")
        if not 0 < queue_byte_limit <= MAX_RECORD_QUEUE_BYTES:
            raise ValueError("queue_byte_limit must be between 1 and 16 MiB")
        if not 0 < max_file_bytes <= MAX_RECORD_FILE_BYTES:
            raise ValueError("max_file_bytes must be between 1 and 64 MiB")
        if not 0 < max_record_bytes <= MAX_RAW_RECORD_BYTES:
            raise ValueError("max_record_bytes must be between 1 byte and 1 MiB")
        self._event_sink = event_sink
        self._queue_capacity = queue_capacity
        self._queue_byte_limit = queue_byte_limit
        self._max_file_bytes = max_file_bytes
        self._max_record_bytes = max_record_bytes
        self._lock = Lock()
        self._queue: Queue[RawRecord] = Queue(maxsize=queue_capacity)
        self._queued_bytes = 0
        self._stop_requested = Event()
        self._thread: Thread | None = None
        self._state = RecordingState.STOPPED
        self._path: str | None = None
        self._written_records = 0
        self._dropped_records = 0
        self._bytes_written = 0
        self._error: ErrorInfo | None = None
        self._backpressure_notified = False

    def start(self, path: str) -> None:
        """Start a new recording; file I/O is performed by the writer thread."""

        if not isinstance(path, str):
            raise ConfigurationError("原始记录文件路径必须是字符串。")
        path_text = path.strip()
        if not path_text:
            raise ConfigurationError("原始记录文件路径不能为空。")
        if len(path_text) > MAX_RECORD_PATH_LENGTH:
            raise ConfigurationError("原始记录文件路径超过长度上限。")

        with self._lock:
            if self._thread is not None and self._thread.is_alive():
                raise RecordingBusyError()
            self._queue = Queue(maxsize=self._queue_capacity)
            self._queued_bytes = 0
            self._stop_requested = Event()
            self._state = RecordingState.STARTING
            self._path = path_text
            self._written_records = 0
            self._dropped_records = 0
            self._bytes_written = 0
            self._error = None
            self._backpressure_notified = False
            thread = Thread(
                target=self._run,
                args=(Path(path_text), self._queue, self._stop_requested),
                name="serialforge-recorder",
                daemon=True,
            )
            self._thread = thread
            snapshot = self._snapshot_locked()

        self._publish_snapshot(snapshot)
        try:
            thread.start()
        except RuntimeError as exc:
            error = ErrorInfo(
                code=ErrorCode.RECORDING,
                message="无法启动原始记录 worker。",
                recoverable=False,
                detail=str(exc),
            )
            with self._lock:
                self._state = RecordingState.ERROR
                self._error = error
                self._thread = None
                snapshot = self._snapshot_locked()
            self._publish_snapshot(snapshot)
            raise SerialForgeError(
                error.message,
                code=error.code,
                recoverable=error.recoverable,
                detail=error.detail,
            ) from exc

    def stop(self) -> None:
        """Request a flush-and-stop; the caller never waits on file I/O."""

        with self._lock:
            if self._thread is None or not self._thread.is_alive():
                return
            if self._state in {RecordingState.STARTING, RecordingState.ACTIVE}:
                self._state = RecordingState.STOPPING
            self._stop_requested.set()
            snapshot = self._snapshot_locked()
        self._publish_snapshot(snapshot)

    def record(self, record: RawRecord) -> bool:
        """Enqueue a complete record without blocking the transport worker."""

        if not isinstance(record, RawRecord):
            raise ConfigurationError("记录器只接受 RawRecord。")
        notify = False
        accepted = True
        dropped = 0
        with self._lock:
            if self._state not in {RecordingState.STARTING, RecordingState.ACTIVE}:
                return False
            if (
                len(record.payload) > self._max_record_bytes
                or self._queued_bytes + len(record.payload) > self._queue_byte_limit
            ):
                accepted = False
                self._dropped_records += 1
                dropped = self._dropped_records
                if not self._backpressure_notified:
                    self._backpressure_notified = True
                    notify = True
            else:
                try:
                    self._queue.put_nowait(record)
                except Full:
                    accepted = False
                    self._dropped_records += 1
                    dropped = self._dropped_records
                    if not self._backpressure_notified:
                        self._backpressure_notified = True
                        notify = True
                else:
                    self._queued_bytes += len(record.payload)

        if notify:
            self._publish(
                RecordingBackpressureEvent(
                    dropped_records=dropped,
                    occurred_at=time.monotonic(),
                )
            )
        return accepted

    def snapshot(self) -> RecordingSnapshot:
        """Return an immutable state snapshot safe for polling from Qt."""

        with self._lock:
            return self._snapshot_locked()

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the writer within a bounded application-shutdown budget."""

        if timeout is not None and timeout < 0:
            raise ValueError("timeout must be non-negative or None")
        self.stop()
        with self._lock:
            thread = self._thread
        if thread is None:
            return
        thread.join(timeout=timeout)
        if thread.is_alive():
            raise RecordingShutdownTimeoutError()

    def _run(self, path: Path, queue: Queue[RawRecord], stop_requested: Event) -> None:
        """Run."""
        error: ErrorInfo | None = None
        try:
            path.parent.mkdir(parents=True, exist_ok=True)
            with path.open("ab") as handle:
                existing_size = handle.tell()
                if existing_size > self._max_file_bytes:
                    error = ErrorInfo(
                        code=ErrorCode.RECORDING,
                        message="原始记录文件已超过 64 MiB 上限。",
                        recoverable=True,
                        detail=f"size={existing_size}, limit={self._max_file_bytes}",
                    )
                else:
                    with self._lock:
                        self._bytes_written = existing_size
                        if self._state == RecordingState.STARTING:
                            self._state = RecordingState.ACTIVE
                        snapshot = self._snapshot_locked()
                    self._publish_snapshot(snapshot)

                while error is None:
                    try:
                        record = queue.get(timeout=0.1)
                    except Empty:
                        if stop_requested.is_set():
                            break
                        continue

                    with self._lock:
                        self._queued_bytes = max(0, self._queued_bytes - len(record.payload))

                    line = self._serialize(record)
                    with self._lock:
                        if self._bytes_written + len(line) > self._max_file_bytes:
                            error = ErrorInfo(
                                code=ErrorCode.RECORDING,
                                message="原始记录文件已达到 64 MiB 上限。",
                                recoverable=True,
                                detail=(
                                    f"size={self._bytes_written}, next={len(line)}, "
                                    f"limit={self._max_file_bytes}"
                                ),
                            )
                            self._dropped_records += 1
                            break

                    try:
                        written = handle.write(line)
                        if written != len(line):
                            raise OSError(f"partial write: {written}/{len(line)} bytes")
                        handle.flush()
                    except OSError as exc:
                        error = ErrorInfo(
                            code=ErrorCode.RECORDING,
                            message="写入原始记录文件失败。",
                            recoverable=True,
                            detail=f"{type(exc).__name__}: {exc}",
                        )
                        break

                    with self._lock:
                        self._bytes_written += len(line)
                        self._written_records += 1

                    if stop_requested.is_set() and queue.empty():
                        break
        except OSError as exc:
            error = ErrorInfo(
                code=ErrorCode.RECORDING,
                message="无法打开原始记录文件。",
                recoverable=True,
                detail=f"{type(exc).__name__}: {exc}",
            )
        except Exception as exc:
            logger.exception("SerialForge raw recorder failed")
            error = ErrorInfo(
                code=ErrorCode.RECORDING,
                message="原始记录 worker 发生未处理错误。",
                recoverable=False,
                detail=f"{type(exc).__name__}: {exc}",
            )
        finally:
            with self._lock:
                remaining = 0
                while True:
                    try:
                        queued_record = queue.get_nowait()
                        self._queued_bytes = max(
                            0,
                            self._queued_bytes - len(queued_record.payload),
                        )
                        remaining += 1
                    except Empty:
                        break
                self._dropped_records += remaining
                if error is not None:
                    self._state = RecordingState.ERROR
                    self._error = error
                else:
                    self._state = RecordingState.STOPPED
                self._thread = None
                snapshot = self._snapshot_locked()
            self._publish_snapshot(snapshot)

    def _snapshot_locked(self) -> RecordingSnapshot:
        """Snapshot locked."""
        return RecordingSnapshot(
            state=self._state,
            path=self._path,
            queued_records=self._queue.qsize(),
            written_records=self._written_records,
            dropped_records=self._dropped_records,
            bytes_written=self._bytes_written,
            error=self._error,
        )

    @staticmethod
    def _serialize(record: RawRecord) -> bytes:
        """Serialize."""
        line = {
            "wall_time": record.wall_time.isoformat(),
            "occurred_at": record.occurred_at,
            "session_id": str(record.session_id),
            "transport": record.endpoint.transport.value,
            "address": record.endpoint.address,
            "identity": record.endpoint.identity,
            "peer": (
                {"host": record.peer.host, "port": record.peer.port}
                if record.peer is not None
                else None
            ),
            "peer_id": str(record.peer_id) if record.peer_id is not None else None,
            "channel": record.channel,
            "direction": record.direction.value,
            "payload_b64": b64encode(record.payload).decode("ascii"),
        }
        return (json.dumps(line, ensure_ascii=False, separators=(",", ":")) + "\n").encode("utf-8")

    def _publish_snapshot(self, snapshot: RecordingSnapshot) -> None:
        """Publish snapshot."""
        self._publish(
            RecordingStateChangedEvent(
                snapshot=snapshot,
                occurred_at=time.monotonic(),
            )
        )

    def _publish(self, event: object) -> None:
        """Publish."""
        if self._event_sink is None:
            return
        try:
            self._event_sink.publish(event)  # type: ignore[arg-type]
        except Exception:
            logger.exception("SerialForge recorder event sink failed")


class CommandHistoryService(CommandHistoryPort):
    """Keep recent sends and quick commands in bounded process memory."""

    def __init__(
        self,
        *,
        history_capacity: int = MAX_COMMAND_HISTORY_ITEMS,
        quick_capacity: int = MAX_QUICK_COMMAND_ITEMS,
    ) -> None:
        if not 0 < history_capacity <= MAX_COMMAND_HISTORY_ITEMS:
            raise ValueError("history capacity must be between 1 and 100")
        if not 0 < quick_capacity <= MAX_QUICK_COMMAND_ITEMS:
            raise ValueError("quick command capacity must be between 1 and 64")
        self._history: deque[CommandEntry] = deque(maxlen=history_capacity)
        self._quick: list[CommandEntry] = []
        self._quick_capacity = quick_capacity
        self._lock = Lock()

    def add_history(self, entry: CommandEntry) -> None:
        """Add history."""
        if not isinstance(entry, CommandEntry):
            raise ConfigurationError("历史记录只接受 CommandEntry。")
        with self._lock:
            self._history = deque(
                (existing for existing in self._history if not self._same_command(existing, entry)),
                maxlen=self._history.maxlen,
            )
            self._history.appendleft(entry)

    def history(self) -> tuple[CommandEntry, ...]:
        """History."""
        with self._lock:
            return tuple(self._history)

    def add_quick_command(self, entry: CommandEntry) -> None:
        """Add quick command."""
        if not isinstance(entry, CommandEntry):
            raise ConfigurationError("快捷命令只接受 CommandEntry。")
        with self._lock:
            self._quick = [existing for existing in self._quick if existing.name != entry.name]
            self._quick.insert(0, entry)
            del self._quick[self._quick_capacity :]

    def quick_commands(self) -> tuple[CommandEntry, ...]:
        """Quick commands."""
        with self._lock:
            return tuple(self._quick)

    def remove_quick_command(self, index: int) -> None:
        """Remove quick command."""
        with self._lock:
            if not 0 <= index < len(self._quick):
                raise ConfigurationError("快捷命令索引无效。")
            del self._quick[index]

    def clear_history(self) -> None:
        """Clear history."""
        with self._lock:
            self._history.clear()

    @staticmethod
    def _same_command(left: CommandEntry, right: CommandEntry) -> bool:
        """Same command."""
        return (
            left.payload == right.payload
            and left.mode == right.mode
            and left.append_newline == right.append_newline
        )


class EndpointIdentityService(EndpointIdentityPort):
    """Remember stable identities for future reconnect work without reconnecting now."""

    def __init__(self, *, capacity: int = MAX_ENDPOINT_IDENTITIES) -> None:
        if not 0 < capacity <= MAX_ENDPOINT_IDENTITIES:
            raise ValueError("identity capacity must be between 1 and 64")
        self._capacity = capacity
        self._endpoints: OrderedDict[str, Endpoint] = OrderedDict()
        self._lock = Lock()

    def remember(self, endpoint: Endpoint) -> None:
        """Remember."""
        if not isinstance(endpoint, Endpoint):
            raise ConfigurationError("端点身份服务只接受 Endpoint。")
        identity = endpoint.identity
        if identity is None:
            return
        with self._lock:
            self._endpoints.pop(identity, None)
            self._endpoints[identity] = endpoint
            while len(self._endpoints) > self._capacity:
                self._endpoints.popitem(last=False)

    def resolve(self, identity: str) -> Endpoint | None:
        """Resolve."""
        normalized = identity.strip()
        if not normalized:
            return None
        with self._lock:
            return self._endpoints.get(normalized)

    def recent(self) -> tuple[Endpoint, ...]:
        """Recent."""
        with self._lock:
            return tuple(reversed(tuple(self._endpoints.values())))
