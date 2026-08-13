"""A bounded, non-blocking event bus for worker-to-UI communication."""

from __future__ import annotations

import logging
import time
from collections import deque
from collections.abc import Callable, Iterable
from threading import Lock

from ..domain.events import (
    ComponentFramesDecodedEvent,
    DatagramDataReceivedEvent,
    DatagramDataSentEvent,
    DatasetBatchEvent,
    EventBackpressureEvent,
    ProtocolFramesDecodedEvent,
    ReplayDataEvent,
    SessionEvent,
    StreamDataReceivedEvent,
    StreamDataSentEvent,
)
from ..domain.ports import EventPort

logger = logging.getLogger(__name__)


class BoundedEventBus(EventPort):
    """Keep event delivery bounded and make preview loss observable.

    Stream preview events may be dropped under pressure. Lifecycle and error
    events first evict an older preview event; if no preview event is available,
    the control event is counted and the next drain reports the loss.
    """

    def __init__(
        self,
        *,
        capacity: int = 256,
        observers: Iterable[Callable[[SessionEvent], None]] = (),
    ) -> None:
        if capacity < 2:
            raise ValueError("event capacity must be at least 2")
        self._capacity = capacity
        self._events: deque[SessionEvent] = deque()
        self._lock = Lock()
        self._dropped_stream_events = 0
        self._dropped_control_events = 0
        self._observers = list(observers)

    def add_observer(self, observer: Callable[[SessionEvent], None]) -> None:
        """Register a non-blocking application observer before session start."""

        if not callable(observer):
            raise TypeError("event observer must be callable")
        with self._lock:
            self._observers.append(observer)

    def publish(self, event: SessionEvent) -> bool:
        """Publish without blocking the device worker."""

        with self._lock:
            observers = tuple(self._observers)
            if len(self._events) >= self._capacity:
                if self._is_preview(event):
                    self._dropped_stream_events += 1
                    accepted = False
                else:
                    preview_index = next(
                        (
                            index
                            for index, queued in enumerate(self._events)
                            if self._is_preview(queued)
                        ),
                        None,
                    )
                    if preview_index is None:
                        self._dropped_control_events += 1
                        accepted = False
                    else:
                        queued_events = list(self._events)
                        del queued_events[preview_index]
                        self._events = deque(queued_events)
                        self._dropped_stream_events += 1
                        self._events.append(event)
                        accepted = True
            else:
                self._events.append(event)
                accepted = True

        for observer in observers:
            try:
                observer(event)
            except Exception:
                logger.exception("SerialForge application event observer failed")
        return accepted

    def publish_many(self, events: Iterable[SessionEvent]) -> int:
        """Publish a batch and return the number accepted by the queue."""

        accepted = 0
        for event in events:
            accepted += int(self.publish(event))
        return accepted

    def drain(self, max_events: int = 64) -> tuple[SessionEvent, ...]:
        """Return a bounded batch and report any prior queue pressure."""

        if max_events <= 0:
            raise ValueError("max_events must be positive")

        with self._lock:
            dropped_stream = self._dropped_stream_events
            dropped_control = self._dropped_control_events
            self._dropped_stream_events = 0
            self._dropped_control_events = 0

            result: list[SessionEvent] = []
            if dropped_stream or dropped_control:
                result.append(
                    EventBackpressureEvent(
                        dropped_stream_events=dropped_stream,
                        dropped_control_events=dropped_control,
                        occurred_at=time.monotonic(),
                    )
                )

            remaining = max_events - len(result)
            for _ in range(max(0, remaining)):
                if not self._events:
                    break
                result.append(self._events.popleft())

            return tuple(result)

    @staticmethod
    def _is_preview(event: SessionEvent) -> bool:
        """Is preview."""
        return isinstance(
            event,
            (
                StreamDataReceivedEvent,
                StreamDataSentEvent,
                DatagramDataReceivedEvent,
                DatagramDataSentEvent,
                ProtocolFramesDecodedEvent,
                ComponentFramesDecodedEvent,
                DatasetBatchEvent,
                ReplayDataEvent,
            ),
        )
