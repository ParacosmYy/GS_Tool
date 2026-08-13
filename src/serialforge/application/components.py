"""Application-owned component profile loading and bounded field decoding."""

from __future__ import annotations

import logging
from pathlib import Path
from queue import Empty, Full, Queue
from threading import Event, Lock, Thread

from ..domain.codecs import (
    ComponentCodecConfig,
    ComponentCodecRouter,
    ComponentConfiguration,
    ComponentConfigurationCodec,
)
from ..domain.components import (
    MAX_COMPONENT_PREVIEW_BYTES,
    MAX_COMPONENT_PROFILE_BYTES,
    ComponentFrameRow,
    ComponentProfile,
    ComponentStats,
    default_component_profile,
)
from ..domain.errors import ComponentProfileError, ConfigurationError, SessionShutdownTimeoutError
from ..domain.events import (
    ComponentBackpressureEvent,
    ComponentFramesDecodedEvent,
    ProtocolFramesDecodedEvent,
    SessionEvent,
)
from ..domain.ports import ComponentCodecPort, ComponentPipelinePort, EventSinkPort

logger = logging.getLogger(__name__)

MAX_COMPONENT_QUEUE_ITEMS = 128
MAX_COMPONENT_QUEUE_BYTES = 262_144


class ComponentProfileStore:
    """Load a strict, bounded JSON profile from a user-selected file."""

    def load(self, path: str | Path) -> ComponentConfiguration:
        if not isinstance(path, (str, Path)):
            raise ComponentProfileError("组件 profile 路径类型无效。")
        value = Path(path)
        if not str(value).strip() or len(str(value)) > 4_096:
            raise ComponentProfileError("组件 profile 路径无效。")
        try:
            size = value.stat().st_size
            if size > MAX_COMPONENT_PROFILE_BYTES:
                raise ComponentProfileError("组件 profile 文件超过 64 KiB 上限。")
            text = value.read_text(encoding="utf-8")
        except ComponentProfileError:
            raise
        except OSError as exc:
            raise ComponentProfileError("组件 profile 文件无法读取。", detail=str(exc)) from exc
        try:
            return ComponentConfigurationCodec.loads(text)
        except ConfigurationError as exc:
            raise ComponentProfileError(str(exc), detail=exc.info.detail) from exc


class ComponentEventBridge:
    """Forward protocol frame batches into the component worker outside Qt."""

    def __init__(self, pipeline: ComponentPipelinePort) -> None:
        self._pipeline = pipeline

    def observe(self, event: SessionEvent) -> None:
        if isinstance(event, ProtocolFramesDecodedEvent):
            try:
                self._pipeline.offer(event)
            except Exception:
                logger.exception("SerialForge component ingress bridge failed")


class ComponentPipelineWorker(ComponentPipelinePort):
    """Decode protocol frames on a dedicated bounded worker."""

    def __init__(
        self,
        *,
        event_sink: EventSinkPort,
        codec: ComponentCodecPort | None = None,
        queue_size: int = MAX_COMPONENT_QUEUE_ITEMS,
        queue_byte_limit: int = MAX_COMPONENT_QUEUE_BYTES,
    ) -> None:
        if not 0 < queue_size <= MAX_COMPONENT_QUEUE_ITEMS:
            raise ValueError(f"queue_size must be between 1 and {MAX_COMPONENT_QUEUE_ITEMS}")
        if not 0 < queue_byte_limit <= MAX_COMPONENT_QUEUE_BYTES:
            raise ValueError(f"queue_byte_limit must be between 1 and {MAX_COMPONENT_QUEUE_BYTES}")
        self._event_sink = event_sink
        self._codec = codec or ComponentCodecRouter()
        self._profile: ComponentConfiguration = default_component_profile()
        self._queue: Queue[tuple[ProtocolFramesDecodedEvent, int] | None] = Queue(
            maxsize=queue_size
        )
        self._queue_byte_limit = queue_byte_limit
        self._queued_bytes = 0
        self._generation = 0
        self._minimum_protocol_generation = 0
        self._frames_in = 0
        self._rows_out = 0
        self._field_errors = 0
        self._codec_errors = 0
        self._dropped_frames = 0
        self._lock = Lock()
        self._stop_requested = Event()
        self._thread = Thread(
            target=self._run,
            name="serialforge-component",
            daemon=True,
        )
        self._thread.start()

    @property
    def profile(self) -> ComponentConfiguration:
        with self._lock:
            return self._profile

    @property
    def stats(self) -> ComponentStats:
        with self._lock:
            return ComponentStats(
                frames_in=self._frames_in,
                rows_out=self._rows_out,
                field_errors=self._field_errors,
                codec_errors=self._codec_errors,
                dropped_frames=self._dropped_frames,
                buffered_bytes=self._queued_bytes,
            )

    @property
    def generation(self) -> int:
        with self._lock:
            return self._generation

    def configure(
        self,
        profile: ComponentConfiguration,
        *,
        protocol_generation: int | None = None,
    ) -> None:
        if not isinstance(profile, (ComponentProfile, ComponentCodecConfig)):
            raise ConfigurationError(
                "组件 worker 只接受 ComponentProfile 或 ComponentCodecConfig。"
            )
        _validate_generation(protocol_generation, "protocol_generation")
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            self._profile = profile
            if protocol_generation is not None:
                self._minimum_protocol_generation = protocol_generation
            self._reset_counters_locked()

    def offer(self, event: SessionEvent) -> bool:
        if not isinstance(event, ProtocolFramesDecodedEvent):
            raise ConfigurationError("组件 worker 只接受 ProtocolFramesDecodedEvent。")
        frame_bytes = sum(len(frame.payload) for frame in event.frames)
        frame_count = len(event.frames)
        dropped = False
        generation = 0
        with self._lock:
            if self._stop_requested.is_set():
                return False
            if event.generation < self._minimum_protocol_generation:
                return False
            generation = self._generation
            if self._queued_bytes + frame_bytes > self._queue_byte_limit:
                dropped = True
            else:
                try:
                    self._queue.put_nowait((event, self._generation))
                except Full:
                    dropped = True
                else:
                    self._queued_bytes += frame_bytes
        if dropped:
            with self._lock:
                self._dropped_frames += frame_count
            self._publish(
                ComponentBackpressureEvent(
                    source=event.source,
                    dropped_frames=frame_count,
                    dropped_bytes=frame_bytes,
                    occurred_at=event.occurred_at,
                    generation=generation,
                )
            )
        return not dropped

    def reset(self, *, protocol_generation: int | None = None) -> None:
        _validate_generation(protocol_generation, "protocol_generation")
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            if protocol_generation is not None:
                self._minimum_protocol_generation = protocol_generation
            self._reset_counters_locked()

    def shutdown(self, timeout: float | None = 3.0) -> None:
        if timeout is not None and timeout < 0:
            raise ValueError("timeout must be non-negative or None")
        self._stop_requested.set()
        try:
            self._queue.put_nowait(None)
        except Full:
            pass
        self._thread.join(timeout=timeout)
        if self._thread.is_alive():
            raise SessionShutdownTimeoutError("组件 worker 未能在关闭时限内退出。")

    def _run(self) -> None:
        while not self._stop_requested.is_set():
            try:
                item = self._queue.get(timeout=0.1)
            except Empty:
                continue
            if item is None:
                break
            event, generation = item
            with self._lock:
                frame_bytes = sum(len(frame.payload) for frame in event.frames)
                self._queued_bytes = max(0, self._queued_bytes - frame_bytes)
                if generation != self._generation:
                    continue
                profile = self._profile
            rows_list: list[ComponentFrameRow] = []
            for frame in event.frames:
                try:
                    rows_list.append(
                        self._codec.decode(
                            frame,
                            profile,
                            event.source,
                            event.occurred_at,
                        )
                    )
                except Exception as exc:
                    logger.exception("SerialForge component pipeline failed for one frame")
                    rows_list.append(
                        _codec_error_row(
                            frame,
                            event.source,
                            event.occurred_at,
                            exc,
                        )
                    )
            rows = tuple(rows_list)
            with self._lock:
                if generation != self._generation:
                    continue
                self._frames_in += len(event.frames)
                self._rows_out += len(rows)
                self._field_errors += sum(
                    sum(field.error is not None for field in row.fields) for row in rows
                )
                self._codec_errors += sum(row.codec_error is not None for row in rows)
                stats = ComponentStats(
                    frames_in=self._frames_in,
                    rows_out=self._rows_out,
                    field_errors=self._field_errors,
                    codec_errors=self._codec_errors,
                    dropped_frames=self._dropped_frames,
                    buffered_bytes=self._queued_bytes,
                )
            if rows:
                self._publish(
                    ComponentFramesDecodedEvent(
                        source=event.source,
                        profile=profile,
                        rows=rows,
                        stats=stats,
                        occurred_at=event.occurred_at,
                        generation=generation,
                        protocol_generation=event.generation,
                    )
                )

    def _discard_pending_locked(self) -> None:
        while True:
            try:
                item = self._queue.get_nowait()
            except Empty:
                self._queued_bytes = 0
                return
            if item is not None:
                self._queued_bytes = max(
                    0,
                    self._queued_bytes - sum(len(frame.payload) for frame in item[0].frames),
                )

    def _reset_counters_locked(self) -> None:
        self._frames_in = 0
        self._rows_out = 0
        self._field_errors = 0
        self._codec_errors = 0
        self._dropped_frames = 0

    def _publish(self, event: SessionEvent) -> None:
        try:
            self._event_sink.publish(event)
        except Exception:
            logger.exception("SerialForge component event sink failed")


def _validate_generation(value: int | None, label: str) -> None:
    if value is not None and (isinstance(value, bool) or not isinstance(value, int) or value < 0):
        raise ConfigurationError(f"{label} 必须是非负整数或 None。")


__all__ = ["ComponentEventBridge", "ComponentPipelineWorker", "ComponentProfileStore"]


def _codec_error_row(
    frame: object,
    source: object,
    occurred_at: float,
    error: BaseException,
) -> ComponentFrameRow:
    from ..domain.protocols import DecodedFrame, ProtocolSource

    if not isinstance(frame, DecodedFrame) or not isinstance(source, ProtocolSource):
        raise ConfigurationError("组件 codec fallback 输入类型无效。") from error
    payload = frame.payload[:MAX_COMPONENT_PREVIEW_BYTES]
    payload_hex = payload.hex(" ").upper()
    if len(frame.payload) > MAX_COMPONENT_PREVIEW_BYTES:
        payload_hex += " …"
    return ComponentFrameRow(
        source=source,
        sequence=frame.sequence,
        status=frame.status,
        payload_length=len(frame.payload),
        payload_hex=payload_hex,
        error=frame.error,
        codec_error=f"组件 codec 异常：{type(error).__name__}。",
        occurred_at=occurred_at,
    )
