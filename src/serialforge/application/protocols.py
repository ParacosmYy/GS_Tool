"""Application-owned protocol pipeline kept independent of transports and widgets."""

from __future__ import annotations

import logging
import time
from collections.abc import Callable
from dataclasses import replace
from queue import Empty, Full, Queue
from threading import Event, Lock, Thread
from uuid import UUID

from ..domain.errors import ConfigurationError, SessionShutdownTimeoutError
from ..domain.events import (
    ProtocolBackpressureEvent,
    ProtocolFramesDecodedEvent,
    SessionEvent,
    SessionStateChangedEvent,
    StreamDataReceivedEvent,
)
from ..domain.models import SessionState, TransportKind
from ..domain.ports import EventSinkPort, ProtocolPipelinePort
from ..domain.protocols import (
    MAX_PROTOCOL_BUFFER_BYTES,
    DecodedFrame,
    FrameDecoderPort,
    FramingKind,
    ProtocolBoundary,
    ProtocolConfig,
    ProtocolIngressUnit,
    ProtocolSource,
    ProtocolStats,
    StreamingFrameDecoder,
)
from ..domain.stream_boundaries import MavlinkStreamDecoder, ModbusRtuStreamDecoder

logger = logging.getLogger(__name__)


class ProtocolEventBridge:
    """Forward raw stream events into the parser without making Qt the ingress owner."""

    def __init__(self, pipeline: ProtocolPipelinePort) -> None:
        self._pipeline = pipeline

    def observe(self, event: SessionEvent) -> None:
        if isinstance(event, StreamDataReceivedEvent):
            if event.endpoint.transport not in {TransportKind.UART, TransportKind.TCP_STREAM}:
                return
            source = ProtocolSource(
                session_id=event.session_id,
                transport=event.endpoint.transport,
                peer=event.peer,
                peer_id=event.peer_id,
            )
            try:
                self._pipeline.offer(
                    ProtocolIngressUnit(
                        source=source,
                        boundary=ProtocolBoundary.STREAM,
                        payload=event.chunk.payload,
                        occurred_at=event.occurred_at,
                        timing=event.timing,
                    )
                )
            except Exception:
                logger.exception("SerialForge protocol ingress bridge failed")
            return
        if isinstance(event, SessionStateChangedEvent) and event.state in {
            SessionState.CLOSED,
            SessionState.ERROR,
        }:
            if event.endpoint.transport in {TransportKind.UART, TransportKind.TCP_STREAM}:
                # The desktop session router owns one active client session;
                # reset all parser state so peer metadata cannot leave a tail.
                self._pipeline.reset()


class ProtocolPipeline:
    """Keep one bounded decoder state per session/peer/channel source."""

    def __init__(
        self,
        decoder_factory: Callable[[ProtocolConfig], FrameDecoderPort] | None = None,
    ) -> None:
        self._decoder_factory = decoder_factory
        self._config = ProtocolConfig()
        self._decoders: dict[ProtocolSource, FrameDecoderPort] = {}

    @property
    def config(self) -> ProtocolConfig:
        return self._config

    @property
    def stats(self) -> ProtocolStats:
        snapshots = tuple(decoder.stats for decoder in self._decoders.values())
        return ProtocolStats(
            bytes_in=sum(item.bytes_in for item in snapshots),
            frames_valid=sum(item.frames_valid for item in snapshots),
            frames_invalid=sum(item.frames_invalid for item in snapshots),
            frames_incomplete=sum(item.frames_incomplete for item in snapshots),
            dropped_bytes=sum(item.dropped_bytes for item in snapshots),
            buffered_bytes=sum(item.buffered_bytes for item in snapshots),
            gap_boundaries=sum(item.gap_boundaries for item in snapshots),
            resyncs=sum(item.resyncs for item in snapshots),
        )

    def configure(self, config: ProtocolConfig) -> None:
        if not isinstance(config, ProtocolConfig):
            raise ConfigurationError("协议 pipeline 只接受 ProtocolConfig。")
        self._config = config
        self._decoders.clear()

    def feed(self, unit: ProtocolIngressUnit) -> tuple[DecodedFrame, ...]:
        if not isinstance(unit, ProtocolIngressUnit):
            raise ConfigurationError("协议 pipeline 只接受 ProtocolIngressUnit。")
        decoder = self._decoders.get(unit.source)
        if decoder is None:
            decoder = self._new_decoder()
            self._decoders[unit.source] = decoder
        if hasattr(decoder, "feed_unit"):
            frames = list(decoder.feed_unit(unit))
        else:
            frames = list(decoder.feed(unit.payload))
        if unit.boundary is not ProtocolBoundary.STREAM:
            frames.extend(decoder.finish())
        return tuple(replace(frame, source=unit.source) for frame in frames)

    def finish(self, source: ProtocolSource | None = None) -> tuple[DecodedFrame, ...]:
        """Flush one source or all partial parser states before disconnect/reset."""

        sources = (source,) if source is not None else tuple(self._decoders)
        frames: list[DecodedFrame] = []
        for item in sources:
            decoder = self._decoders.get(item)
            if decoder is not None:
                frames.extend(replace(frame, source=item) for frame in decoder.finish())
        return tuple(frames)

    def reset(self, source: ProtocolSource | None = None) -> None:
        if source is None:
            self._decoders.clear()
            return
        self._decoders.pop(source, None)

    def _new_decoder(self) -> FrameDecoderPort:
        if self._decoder_factory is not None:
            return self._decoder_factory(self._config)
        if self._config.framing is FramingKind.MAVLINK_STREAM:
            return MavlinkStreamDecoder(self._config)
        if self._config.framing is FramingKind.MODBUS_RTU_TIMED:
            return ModbusRtuStreamDecoder(self._config)
        return StreamingFrameDecoder(self._config)


class ProtocolPipelineWorker(ProtocolPipelinePort):
    """Run parser state off the Qt thread with bounded, source-aware ingress."""

    def __init__(
        self,
        *,
        event_sink: EventSinkPort,
        queue_size: int = 256,
        queue_byte_limit: int = MAX_PROTOCOL_BUFFER_BYTES,
    ) -> None:
        if not 0 < queue_size <= 4_096:
            raise ValueError("queue_size must be between 1 and 4096")
        if not 0 < queue_byte_limit <= MAX_PROTOCOL_BUFFER_BYTES:
            raise ValueError("queue_byte_limit must be between 1 and MAX_PROTOCOL_BUFFER_BYTES")
        self._event_sink = event_sink
        self._pipeline = ProtocolPipeline()
        self._queue: Queue[tuple[ProtocolIngressUnit, int, int] | None] = Queue(maxsize=queue_size)
        self._queue_byte_limit = queue_byte_limit
        self._queued_bytes = 0
        self._segment_by_source: dict[ProtocolSource, UUID | None] = {}
        self._continuity_epoch_by_source: dict[ProtocolSource, int] = {}
        self._processed_epoch_by_source: dict[ProtocolSource, int] = {}
        self._generation = 0
        self._lock = Lock()
        self._stop_requested = Event()
        self._thread = Thread(
            target=self._run,
            name="serialforge-protocol",
            daemon=True,
        )
        self._thread.start()

    @property
    def config(self) -> ProtocolConfig:
        with self._lock:
            return self._pipeline.config

    @property
    def stats(self) -> ProtocolStats:
        with self._lock:
            return self._pipeline.stats

    @property
    def generation(self) -> int:
        with self._lock:
            return self._generation

    def offer(self, unit: ProtocolIngressUnit) -> bool:
        if not isinstance(unit, ProtocolIngressUnit):
            raise ConfigurationError("协议 worker 只接受 ProtocolIngressUnit。")
        dropped = False
        generation = 0
        with self._lock:
            if self._stop_requested.is_set():
                return False
            generation = self._generation
            epoch = self._continuity_epoch_by_source.get(unit.source, 0)
            if self._queued_bytes + len(unit.payload) > self._queue_byte_limit:
                dropped = True
            else:
                try:
                    self._queue.put_nowait((unit, generation, epoch))
                except Full:
                    dropped = True
                else:
                    self._queued_bytes += len(unit.payload)
            if dropped:
                self._continuity_epoch_by_source[unit.source] = epoch + 1
        if dropped:
            self._publish(
                ProtocolBackpressureEvent(
                    source=unit.source,
                    dropped_units=1,
                    dropped_bytes=len(unit.payload),
                    occurred_at=time.monotonic(),
                    generation=generation,
                )
            )
        return not dropped

    def configure(self, config: ProtocolConfig) -> None:
        if not isinstance(config, ProtocolConfig):
            raise ConfigurationError("协议 worker 只接受 ProtocolConfig。")
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            self._pipeline.configure(config)
            self._segment_by_source.clear()
            self._continuity_epoch_by_source.clear()
            self._processed_epoch_by_source.clear()

    def finish(self, source: ProtocolSource | None = None) -> None:
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            frames = self._pipeline.finish(source)
            stats = self._pipeline.stats
            generation = self._generation
            if source is None:
                self._continuity_epoch_by_source.clear()
                self._processed_epoch_by_source.clear()
            else:
                self._continuity_epoch_by_source.pop(source, None)
                self._processed_epoch_by_source.pop(source, None)
        self._publish_frames(frames, stats, source, generation=generation)

    def reset(self, source: ProtocolSource | None = None) -> None:
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            self._pipeline.reset(source)
            if source is None:
                self._segment_by_source.clear()
                self._continuity_epoch_by_source.clear()
                self._processed_epoch_by_source.clear()
            else:
                self._segment_by_source.pop(source, None)
                self._continuity_epoch_by_source.pop(source, None)
                self._processed_epoch_by_source.pop(source, None)

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
            raise SessionShutdownTimeoutError("协议 parser worker 未能在关闭时限内退出。")

    def _run(self) -> None:
        while not self._stop_requested.is_set():
            try:
                item = self._queue.get(timeout=0.1)
            except Empty:
                continue
            if item is None:
                break
            unit, generation, epoch = item
            try:
                with self._lock:
                    self._queued_bytes = max(0, self._queued_bytes - len(unit.payload))
                    if generation != self._generation:
                        continue
                    boundary_batches: list[tuple[tuple[DecodedFrame, ...], ProtocolStats]] = []
                    previous_epoch = self._processed_epoch_by_source.get(unit.source)
                    if previous_epoch is not None and previous_epoch != epoch:
                        continuity_frames = self._pipeline.finish(unit.source)
                        continuity_stats = self._pipeline.stats
                        self._pipeline.reset(unit.source)
                        if continuity_frames:
                            boundary_batches.append((continuity_frames, continuity_stats))
                        self._segment_by_source.pop(unit.source, None)
                    self._processed_epoch_by_source[unit.source] = epoch
                    previous_segment = self._segment_by_source.get(unit.source)
                    has_previous_segment = unit.source in self._segment_by_source
                    if has_previous_segment and previous_segment != unit.segment_id:
                        segment_frames = self._pipeline.finish(unit.source)
                        segment_stats = self._pipeline.stats
                        self._pipeline.reset(unit.source)
                        if segment_frames:
                            boundary_batches.append((segment_frames, segment_stats))
                    self._segment_by_source[unit.source] = unit.segment_id
                    frames = self._pipeline.feed(unit)
                    stats = self._pipeline.stats
            except Exception:
                logger.exception("SerialForge protocol pipeline failed for one ingress unit")
                continue
            for boundary_frames, boundary_stats in boundary_batches:
                self._publish_frames(
                    boundary_frames,
                    boundary_stats,
                    unit.source,
                    occurred_at=unit.occurred_at,
                    generation=generation,
                )
            if frames:
                self._publish_frames(
                    frames,
                    stats,
                    unit.source,
                    occurred_at=unit.occurred_at,
                    generation=generation,
                )

    def _discard_pending_locked(self) -> None:
        while True:
            try:
                item = self._queue.get_nowait()
            except Empty:
                return
            if item is not None:
                self._queued_bytes = max(0, self._queued_bytes - len(item[0].payload))

    def _publish_frames(
        self,
        frames: tuple[DecodedFrame, ...],
        stats: ProtocolStats,
        source: ProtocolSource | None,
        *,
        occurred_at: float | None = None,
        generation: int = 0,
    ) -> None:
        if not frames:
            return
        event_source = source or frames[0].source
        if event_source is None:
            logger.error("Protocol pipeline produced a frame without a source")
            return
        self._publish(
            ProtocolFramesDecodedEvent(
                source=event_source,
                frames=frames,
                stats=stats,
                occurred_at=time.monotonic() if occurred_at is None else occurred_at,
                generation=generation,
            )
        )

    def _publish(self, event: SessionEvent) -> None:
        try:
            self._event_sink.publish(event)
        except Exception:
            logger.exception("SerialForge protocol event sink failed")
