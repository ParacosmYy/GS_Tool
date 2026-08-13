"""Bounded, Qt-free recovery chunk handoff."""

from __future__ import annotations

from collections import deque
from collections.abc import Iterable
from threading import Condition

from ..application.ports import (
    RecoveryChannelAborted,
    RecoveryChannelState,
    RecoveryChunkChannel,
)


class BoundedRecoveryChunkChannel(RecoveryChunkChannel):
    """A non-blocking producer channel bounded by chunks and UTF-8 bytes."""

    def __init__(self, max_chunks: int, max_bytes: int) -> None:
        if type(max_chunks) is not int or max_chunks < 1:
            raise ValueError("max_chunks must be a positive integer")
        if type(max_bytes) is not int or max_bytes < 1:
            raise ValueError("max_bytes must be a positive integer")
        self._max_chunks = max_chunks
        self._max_bytes = max_bytes
        self._condition = Condition()
        self._items: deque[tuple[str, int]] = deque()
        self._queued_bytes = 0
        self._peak_queued_chunks = 0
        self._peak_queued_bytes = 0
        self._state: RecoveryChannelState = "open"
        self._error: Exception | None = None

    @property
    def state(self) -> RecoveryChannelState:
        """Return the current producer/consumer lifecycle state."""
        with self._condition:
            return self._state

    @property
    def error(self) -> Exception | None:
        """Return the producer-side error that caused an abort."""
        with self._condition:
            return self._error

    @property
    def queued_chunks(self) -> int:
        """Return the number of accepted chunks not yet consumed."""
        with self._condition:
            return len(self._items)

    @property
    def queued_bytes(self) -> int:
        """Return the UTF-8 bytes currently held by the channel."""
        with self._condition:
            return self._queued_bytes

    @property
    def peak_queued_chunks(self) -> int:
        """Return the maximum buffered chunk count observed by the channel."""
        with self._condition:
            return self._peak_queued_chunks

    @property
    def peak_queued_bytes(self) -> int:
        """Return the maximum buffered UTF-8 bytes observed by the channel."""
        with self._condition:
            return self._peak_queued_bytes

    def offer(self, chunk: str) -> bool:
        """Accept a chunk without waiting for the worker to consume it."""
        if not isinstance(chunk, str):
            raise TypeError("chunk must be a string")
        chunk_bytes = len(chunk.encode("utf-8"))
        if chunk_bytes > self._max_bytes:
            raise ValueError("chunk exceeds the recovery channel byte bound")
        with self._condition:
            if self._state != "open":
                return False
            if (
                len(self._items) >= self._max_chunks
                or self._queued_bytes + chunk_bytes > self._max_bytes
            ):
                return False
            self._items.append((chunk, chunk_bytes))
            self._queued_bytes += chunk_bytes
            self._peak_queued_chunks = max(self._peak_queued_chunks, len(self._items))
            self._peak_queued_bytes = max(self._peak_queued_bytes, self._queued_bytes)
            self._condition.notify()
            return True

    def finish(self) -> bool:
        """Close production while allowing the worker to drain accepted chunks."""
        with self._condition:
            if self._state != "open":
                return False
            self._state = "finished"
            self._condition.notify_all()
            return True

    def abort(self, reason: Exception | None = None) -> bool:
        """Discard buffered chunks and wake a blocked consumer."""
        with self._condition:
            if self._state != "open":
                return False
            self._state = "aborted"
            self._error = reason
            self._items.clear()
            self._queued_bytes = 0
            self._condition.notify_all()
            return True

    def consume(self) -> Iterable[str]:
        """Yield chunks until finish, or raise a typed error after abort."""
        while True:
            with self._condition:
                while not self._items and self._state == "open":
                    self._condition.wait()
                if self._state == "aborted":
                    raise RecoveryChannelAborted(self._error)
                if self._items:
                    chunk, chunk_bytes = self._items.popleft()
                    self._queued_bytes -= chunk_bytes
                elif self._state == "finished":
                    return
                else:  # pragma: no cover - defensive state guard
                    raise RuntimeError(f"Unexpected recovery channel state: {self._state}")
            yield chunk


__all__ = ["BoundedRecoveryChunkChannel"]
