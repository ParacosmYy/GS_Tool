"""Application-owned bounded transform and dataset processing."""

from __future__ import annotations

import logging
import time
from collections import deque
from pathlib import Path
from queue import Empty, Full, Queue
from threading import Event, Lock, Thread

from ..domain.components import ComponentFrameRow
from ..domain.datasets import (
    MAX_DATASET_CONFIG_BYTES,
    DatasetConfig,
    DatasetConfigurationCodec,
    DatasetSample,
    DatasetStats,
    DatasetValue,
    default_dataset_config,
    sample_from_row,
)
from ..domain.errors import (
    ConfigurationError,
    DatasetConfigurationError,
    SessionShutdownTimeoutError,
)
from ..domain.events import (
    ComponentFramesDecodedEvent,
    DatasetBackpressureEvent,
    DatasetBatchEvent,
    SessionEvent,
)
from ..domain.ports import DatasetPipelinePort, EventSinkPort

logger = logging.getLogger(__name__)

MAX_DATASET_QUEUE_ITEMS = 128
MAX_DATASET_QUEUE_BYTES = 262_144


class DatasetConfigStore:
    """Load one strict, bounded dataset JSON file."""

    def load(self, path: str | Path) -> DatasetConfig:
        if not isinstance(path, (str, Path)):
            raise DatasetConfigurationError("dataset 配置路径类型无效。")
        value = Path(path)
        if not str(value).strip() or len(str(value)) > 4_096:
            raise DatasetConfigurationError("dataset 配置路径无效。")
        try:
            size = value.stat().st_size
            if size > MAX_DATASET_CONFIG_BYTES:
                raise DatasetConfigurationError("dataset 配置文件超过 64 KiB 上限。")
            text = value.read_text(encoding="utf-8")
        except DatasetConfigurationError:
            raise
        except OSError as exc:
            raise DatasetConfigurationError("dataset 配置文件无法读取。", detail=str(exc)) from exc
        try:
            return DatasetConfigurationCodec.loads(text)
        except ConfigurationError as exc:
            raise DatasetConfigurationError(str(exc), detail=exc.info.detail) from exc


class DatasetEventBridge:
    """Forward component batches to the dataset worker without blocking ingress."""

    def __init__(self, pipeline: DatasetPipelinePort) -> None:
        self._pipeline = pipeline

    def observe(self, event: SessionEvent) -> None:
        if isinstance(event, ComponentFramesDecodedEvent):
            try:
                self._pipeline.offer(event)
            except Exception:
                logger.exception("SerialForge dataset ingress bridge failed")


class DatasetPipelineWorker(DatasetPipelinePort):
    """Transform typed component values on a bounded dedicated worker."""

    def __init__(
        self,
        *,
        event_sink: EventSinkPort,
        queue_size: int = MAX_DATASET_QUEUE_ITEMS,
        queue_byte_limit: int = MAX_DATASET_QUEUE_BYTES,
    ) -> None:
        if not 0 < queue_size <= MAX_DATASET_QUEUE_ITEMS:
            raise ValueError(f"queue_size must be between 1 and {MAX_DATASET_QUEUE_ITEMS}")
        if not 0 < queue_byte_limit <= MAX_DATASET_QUEUE_BYTES:
            raise ValueError(f"queue_byte_limit must be between 1 and {MAX_DATASET_QUEUE_BYTES}")
        self._event_sink = event_sink
        self._config = default_dataset_config()
        self._queue: Queue[tuple[ComponentFramesDecodedEvent, int] | None] = Queue(
            maxsize=queue_size
        )
        self._queue_byte_limit = queue_byte_limit
        self._queued_bytes = 0
        self._generation = 0
        self._minimum_component_generation = 0
        self._minimum_protocol_generation = 0
        self._frames_in = 0
        self._samples_out = 0
        self._transform_errors = 0
        self._dropped_batches = 0
        self._dropped_samples = 0
        self._samples: deque[DatasetSample] = deque()
        self._lock = Lock()
        self._stop_requested = Event()
        self._thread = Thread(
            target=self._run,
            name="serialforge-dataset",
            daemon=True,
        )
        self._thread.start()

    @property
    def config(self) -> DatasetConfig:
        with self._lock:
            return self._config

    @property
    def stats(self) -> DatasetStats:
        with self._lock:
            return self._stats_locked()

    @property
    def generation(self) -> int:
        with self._lock:
            return self._generation

    @property
    def samples(self) -> tuple[DatasetSample, ...]:
        with self._lock:
            return tuple(self._samples)

    def configure(
        self,
        config: DatasetConfig,
        *,
        component_generation: int | None = None,
        protocol_generation: int | None = None,
    ) -> None:
        if not isinstance(config, DatasetConfig):
            raise ConfigurationError("dataset worker 只接受 DatasetConfig。")
        _validate_generation(component_generation, "component_generation")
        _validate_generation(protocol_generation, "protocol_generation")
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            self._config = config
            if component_generation is not None:
                self._minimum_component_generation = component_generation
            if protocol_generation is not None:
                self._minimum_protocol_generation = protocol_generation
            self._reset_counters_locked()
            self._samples.clear()

    def offer(self, event: ComponentFramesDecodedEvent) -> bool:
        if not isinstance(event, ComponentFramesDecodedEvent):
            raise ConfigurationError("dataset worker 只接受 ComponentFramesDecodedEvent。")
        frame_bytes = sum(row.payload_length for row in event.rows)
        frame_count = len(event.rows)
        dropped = False
        generation = 0
        with self._lock:
            if self._stop_requested.is_set():
                return False
            if (
                event.generation < self._minimum_component_generation
                or event.protocol_generation < self._minimum_protocol_generation
            ):
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
            series_count = len(self._config.series)
        if dropped:
            with self._lock:
                self._dropped_batches += 1
                self._dropped_samples += frame_count * series_count
            self._publish(
                DatasetBackpressureEvent(
                    source=event.source,
                    dropped_frames=frame_count,
                    dropped_bytes=frame_bytes,
                    occurred_at=time.monotonic(),
                    generation=generation,
                )
            )
        return not dropped

    def reset(
        self,
        *,
        component_generation: int | None = None,
        protocol_generation: int | None = None,
    ) -> None:
        _validate_generation(component_generation, "component_generation")
        _validate_generation(protocol_generation, "protocol_generation")
        with self._lock:
            self._generation += 1
            self._discard_pending_locked()
            if component_generation is not None:
                self._minimum_component_generation = component_generation
            if protocol_generation is not None:
                self._minimum_protocol_generation = protocol_generation
            self._reset_counters_locked()
            self._samples.clear()

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
            raise SessionShutdownTimeoutError("dataset worker 未能在关闭时限内退出。")

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
                frame_bytes = sum(row.payload_length for row in event.rows)
                self._queued_bytes = max(0, self._queued_bytes - frame_bytes)
                if generation != self._generation:
                    continue
                config = self._config
            samples: list[DatasetSample] = []
            for row in event.rows:
                try:
                    samples.append(sample_from_row(row, config))
                except Exception as exc:
                    logger.exception("SerialForge dataset pipeline failed for one row")
                    samples.append(_error_sample(row, config, exc))
            if not config.series:
                continue
            with self._lock:
                if generation != self._generation:
                    continue
                self._frames_in += len(event.rows)
                self._samples_out += len(samples)
                self._transform_errors += sum(
                    sum(value.error is not None for value in sample.values) for sample in samples
                )
                self._samples.extend(samples)
                while len(self._samples) > config.capacity:
                    self._samples.popleft()
                stats = self._stats_locked()
            if samples:
                self._publish(
                    DatasetBatchEvent(
                        source=event.source,
                        samples=tuple(samples),
                        stats=stats,
                        occurred_at=event.occurred_at,
                        config=config,
                        generation=generation,
                        component_generation=event.generation,
                        protocol_generation=event.protocol_generation,
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
                    self._queued_bytes - sum(row.payload_length for row in item[0].rows),
                )

    def _reset_counters_locked(self) -> None:
        self._frames_in = 0
        self._samples_out = 0
        self._transform_errors = 0
        self._dropped_batches = 0
        self._dropped_samples = 0

    def _stats_locked(self) -> DatasetStats:
        return DatasetStats(
            frames_in=self._frames_in,
            samples_out=self._samples_out,
            transform_errors=self._transform_errors,
            dropped_batches=self._dropped_batches,
            dropped_samples=self._dropped_samples,
            retained_samples=len(self._samples),
            buffered_bytes=self._queued_bytes,
        )

    def _publish(self, event: SessionEvent) -> None:
        try:
            self._event_sink.publish(event)
        except Exception:
            logger.exception("SerialForge dataset event sink failed")


def _error_sample(
    row: ComponentFrameRow, config: DatasetConfig, error: BaseException
) -> DatasetSample:
    message = f"dataset 处理异常：{type(error).__name__}。"
    return DatasetSample(
        source=row.source,
        sequence=row.sequence,
        occurred_at=row.occurred_at,
        payload_hex=row.payload_hex,
        values=tuple(
            DatasetValue(
                field_name=series.field_name,
                input_value=None,
                value=None,
                display="<处理错误>",
                unit=series.unit,
                error=message,
            )
            for series in config.series
        ),
    )


def _validate_generation(value: int | None, label: str) -> None:
    if value is not None and (isinstance(value, bool) or not isinstance(value, int) or value < 0):
        raise ConfigurationError(f"{label} 必须是非负整数或 None。")


__all__ = ["DatasetConfigStore", "DatasetEventBridge", "DatasetPipelineWorker"]
