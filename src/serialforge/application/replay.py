"""Bounded historical replay worker that feeds the existing protocol pipeline."""

from __future__ import annotations

import logging
import time
from dataclasses import replace
from pathlib import Path
from threading import Condition, Event, Lock, Thread
from uuid import UUID, uuid4

from ..domain.errors import (
    ErrorCode,
    ErrorInfo,
    ReplayBusyError,
    ReplayConfigurationError,
    ReplayShutdownTimeoutError,
)
from ..domain.events import ReplayDataEvent, ReplayStateChangedEvent, SessionEvent
from ..domain.models import RecordDirection, TransportKind
from ..domain.ports import EventSinkPort, ProtocolPipelinePort, ReplayPipelinePort
from ..domain.protocols import (
    DataOrigin,
    ProtocolBoundary,
    ProtocolIngressUnit,
    ProtocolSource,
)
from ..domain.replay import (
    MAX_REPLAY_LINE_BYTES,
    ReplayOptions,
    ReplayRecordCodec,
    ReplaySnapshot,
    ReplayState,
)

logger = logging.getLogger(__name__)


class ReplayPipelineWorker(ReplayPipelinePort):
    """Read RX JSONL incrementally and offer it to the normal parser worker."""

    def __init__(
        self,
        *,
        protocol: ProtocolPipelinePort,
        event_sink: EventSinkPort,
    ) -> None:
        self._protocol = protocol
        self._event_sink = event_sink
        self._lock = Lock()
        self._condition = Condition(self._lock)
        self._snapshot = ReplaySnapshot()
        self._thread: Thread | None = None
        self._stop_requested = Event()
        self._paused = False

    def start(self, path: str, options: ReplayOptions | None = None) -> UUID:
        if not isinstance(path, str) or not path.strip() or len(path) > 4_096:
            raise ReplayConfigurationError("历史回放路径无效。")
        options = options or ReplayOptions()
        if not isinstance(options, ReplayOptions):
            raise ReplayConfigurationError("历史回放 options 类型无效。")
        value = Path(path)
        if not value.is_file():
            raise ReplayConfigurationError("历史回放文件不存在或不是普通文件。")
        with self._condition:
            if self._thread is not None and self._thread.is_alive():
                raise ReplayBusyError()
            replay_id = uuid4()
            self._stop_requested = Event()
            self._paused = False
            self._snapshot = ReplaySnapshot(
                state=ReplayState.PLAYING,
                replay_id=replay_id,
                path=str(value),
            )
            thread = Thread(
                target=self._run,
                args=(value, replay_id, options, self._stop_requested),
                name="serialforge-replay",
                daemon=True,
            )
            self._thread = thread
        try:
            thread.start()
        except RuntimeError as exc:
            with self._condition:
                self._thread = None
                self._snapshot = replace(
                    self._snapshot,
                    state=ReplayState.ERROR,
                    error=_replay_error("无法启动历史回放 worker。", exc),
                )
                snapshot = self._snapshot
            self._publish_state(snapshot)
            raise ReplayConfigurationError(
                snapshot.error.message, detail=snapshot.error.detail
            ) from exc
        self._publish_state(self.snapshot())
        return replay_id

    def pause(self) -> None:
        with self._condition:
            if self._thread is None or not self._thread.is_alive() or self._paused:
                return
            if self._snapshot.state is not ReplayState.PLAYING:
                return
            self._paused = True
            self._snapshot = replace(self._snapshot, state=ReplayState.PAUSED)
            snapshot = self._snapshot
            self._condition.notify_all()
        self._publish_state(snapshot)

    def resume(self) -> None:
        with self._condition:
            if self._thread is None or not self._thread.is_alive() or not self._paused:
                return
            self._paused = False
            self._snapshot = replace(self._snapshot, state=ReplayState.PLAYING)
            snapshot = self._snapshot
            self._condition.notify_all()
        self._publish_state(snapshot)

    def stop(self) -> None:
        with self._condition:
            if self._thread is None or not self._thread.is_alive():
                return
            self._stop_requested.set()
            self._paused = False
            self._condition.notify_all()

    def snapshot(self) -> ReplaySnapshot:
        with self._lock:
            return self._snapshot

    def shutdown(self, timeout: float | None = 3.0) -> None:
        if timeout is not None and timeout < 0:
            raise ValueError("timeout must be non-negative or None")
        self.stop()
        with self._lock:
            thread = self._thread
        if thread is None:
            return
        thread.join(timeout=timeout)
        if thread.is_alive():
            raise ReplayShutdownTimeoutError()

    def _run(
        self,
        path: Path,
        replay_id: UUID,
        options: ReplayOptions,
        stop_requested: Event,
    ) -> None:
        first_capture: float | None = None
        last_capture: float | None = None
        try:
            with path.open("rb") as handle:
                while not stop_requested.is_set():
                    remaining_bytes = options.max_bytes - self._bytes_read()
                    if remaining_bytes <= 0:
                        self._mark_limit("历史回放达到本次字节上限。")
                        break
                    line = handle.readline(min(MAX_REPLAY_LINE_BYTES + 1, remaining_bytes + 1))
                    if not line:
                        break
                    if len(line) > remaining_bytes:
                        self._increment_bytes(remaining_bytes, options.max_bytes)
                        self._mark_limit("历史回放达到本次字节上限。")
                        break
                    if not self._increment_bytes(len(line), options.max_bytes):
                        self._mark_limit("历史回放达到本次字节上限。")
                        break
                    if len(line) > MAX_REPLAY_LINE_BYTES:
                        self._mark_invalid(
                            "历史回放行超过 2 MiB 上限。",
                            detail=f"line_bytes={len(line)}",
                        )
                        break
                    if not self._increment_records_read(options.max_records):
                        self._mark_limit("历史回放达到本次记录数上限。")
                        break
                    try:
                        record = ReplayRecordCodec.loads_line(
                            line.decode("utf-8"), self._records_read()
                        )
                    except (UnicodeDecodeError, ReplayConfigurationError) as exc:
                        self._mark_invalid(
                            "历史回放包含无效记录。",
                            detail=f"{type(exc).__name__}: {exc}",
                        )
                        continue
                    raw = record.raw
                    if (
                        raw.direction is not RecordDirection.RECEIVE
                        or raw.endpoint.transport
                        not in {
                            TransportKind.UART,
                            TransportKind.TCP_STREAM,
                        }
                    ):
                        self._increment_skipped()
                        continue
                    if last_capture is not None and raw.occurred_at < last_capture:
                        self._mark_invalid(
                            "历史回放 occurred_at 非单调，已跳过该记录。",
                            detail=f"previous={last_capture}, current={raw.occurred_at}",
                        )
                        continue
                    if first_capture is None:
                        first_capture = raw.occurred_at
                    elapsed = raw.occurred_at - first_capture
                    delay = (
                        0.0
                        if last_capture is None
                        else (raw.occurred_at - last_capture) / options.speed
                    )
                    if not self._wait_delay(delay, stop_requested):
                        break
                    source = ProtocolSource(
                        session_id=replay_id,
                        transport=raw.endpoint.transport,
                        peer=raw.peer,
                        peer_id=raw.peer_id,
                        channel=raw.channel,
                        origin=DataOrigin.HISTORICAL,
                    )
                    unit = ProtocolIngressUnit(
                        source=source,
                        boundary=ProtocolBoundary.STREAM,
                        payload=raw.payload,
                        occurred_at=elapsed,
                        segment_id=raw.session_id,
                    )
                    if not self._offer_with_backpressure(unit, stop_requested):
                        break
                    self._increment_emitted(elapsed)
                    self._publish(
                        ReplayDataEvent(
                            replay_id=replay_id,
                            record=record,
                            occurred_at=elapsed,
                        )
                    )
                    last_capture = raw.occurred_at
                    if self._records_emitted() >= options.max_records:
                        self._mark_limit("历史回放达到本次记录数上限。")
                        break
            if stop_requested.is_set():
                final_state = ReplayState.STOPPED
            elif self.snapshot().state is ReplayState.ERROR:
                final_state = ReplayState.ERROR
            else:
                final_state = ReplayState.EOF
            self._finish(final_state)
        except OSError as exc:
            self._finish(ReplayState.ERROR, _replay_error("读取历史回放文件失败。", exc))
        except Exception as exc:
            logger.exception("SerialForge replay worker failed")
            self._finish(ReplayState.ERROR, _replay_error("历史回放 worker 发生未处理错误。", exc))

    def _wait_delay(self, delay: float, stop_requested: Event) -> bool:
        remaining = max(0.0, delay)
        last_tick = time.monotonic()
        while remaining > 0 and not stop_requested.is_set():
            with self._condition:
                if self._paused:
                    self._condition.wait(timeout=0.1)
                    last_tick = time.monotonic()
                    continue
                self._condition.wait(timeout=min(remaining, 0.1))
            now = time.monotonic()
            if not self._paused:
                remaining -= max(0.0, now - last_tick)
            last_tick = now
        return not stop_requested.is_set()

    def _offer_with_backpressure(self, unit: ProtocolIngressUnit, stop_requested: Event) -> bool:
        while not stop_requested.is_set():
            if self._protocol.offer(unit):
                return True
            self._increment_blocked()
            if not self._wait_delay(0.02, stop_requested):
                return False
        return False

    def _increment_bytes(self, amount: int, limit: int) -> bool:
        with self._lock:
            if self._snapshot.bytes_read + amount > limit:
                return False
            self._snapshot = replace(self._snapshot, bytes_read=self._snapshot.bytes_read + amount)
            return True

    def _increment_records_read(self, limit: int) -> bool:
        with self._lock:
            if self._snapshot.records_read >= limit:
                return False
            self._snapshot = replace(
                self._snapshot,
                records_read=self._snapshot.records_read + 1,
            )
            return True

    def _increment_skipped(self) -> None:
        with self._lock:
            self._snapshot = replace(
                self._snapshot,
                skipped_records=self._snapshot.skipped_records + 1,
            )

    def _increment_emitted(self, elapsed: float) -> None:
        with self._lock:
            self._snapshot = replace(
                self._snapshot,
                records_emitted=self._snapshot.records_emitted + 1,
                capture_elapsed=elapsed,
            )
            if self._snapshot.records_emitted % 16 == 0:
                snapshot = self._snapshot
            else:
                snapshot = None
        if snapshot is not None:
            self._publish_state(snapshot)

    def _increment_blocked(self) -> None:
        with self._lock:
            self._snapshot = replace(
                self._snapshot,
                blocked_offers=self._snapshot.blocked_offers + 1,
            )

    def _mark_invalid(self, message: str, *, detail: str | None = None) -> None:
        error = ErrorInfo(code=ErrorCode.REPLAY, message=message, recoverable=True, detail=detail)
        with self._lock:
            self._snapshot = replace(
                self._snapshot,
                invalid_records=self._snapshot.invalid_records + 1,
                error=error,
            )
            snapshot = self._snapshot
        if snapshot.invalid_records == 1 or snapshot.invalid_records % 16 == 0:
            self._publish_state(snapshot)

    def _mark_limit(self, message: str) -> None:
        error = ErrorInfo(code=ErrorCode.REPLAY, message=message, recoverable=True)
        with self._lock:
            self._snapshot = replace(self._snapshot, error=error)
            snapshot = self._snapshot
        self._publish_state(snapshot)

    def _finish(self, state: ReplayState, error: ErrorInfo | None = None) -> None:
        with self._condition:
            self._snapshot = replace(
                self._snapshot,
                state=state,
                error=error if error is not None else self._snapshot.error,
            )
            self._thread = None
            self._paused = False
            snapshot = self._snapshot
            self._condition.notify_all()
        self._publish_state(snapshot)

    def _records_read(self) -> int:
        with self._lock:
            return self._snapshot.records_read

    def _bytes_read(self) -> int:
        with self._lock:
            return self._snapshot.bytes_read

    def _records_emitted(self) -> int:
        with self._lock:
            return self._snapshot.records_emitted

    def _publish_state(self, snapshot: ReplaySnapshot) -> None:
        self._publish(
            ReplayStateChangedEvent(
                snapshot=snapshot,
                occurred_at=time.monotonic(),
            )
        )

    def _publish(self, event: SessionEvent) -> None:
        try:
            self._event_sink.publish(event)
        except Exception:
            logger.exception("SerialForge replay event sink failed")


def _replay_error(message: str, error: BaseException) -> ErrorInfo:
    return ErrorInfo(
        code=ErrorCode.REPLAY,
        message=message,
        recoverable=True,
        detail=f"{type(error).__name__}: {error}",
    )


__all__ = ["ReplayPipelineWorker"]
