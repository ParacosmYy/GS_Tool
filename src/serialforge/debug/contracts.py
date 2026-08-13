"""Stable contracts for future RTT/J-Link debug print adapters."""

from __future__ import annotations

import math
from dataclasses import dataclass
from enum import StrEnum
from typing import Protocol

MAX_DEBUG_LOG_CHUNK_BYTES = 64 * 1024
MAX_DEBUG_CHANNEL_CHARS = 128


class DebugLogBackend(StrEnum):
    """Backend names; vendor SDKs remain outside the core package."""

    RTT = "rtt"
    JLINK_TELNET = "jlink_telnet"


@dataclass(frozen=True, slots=True)
class DebugLogRecord:
    """One bounded raw debug record with no parser or UI dependency."""

    backend: DebugLogBackend
    channel: str
    payload: bytes
    occurred_at: float

    def __post_init__(self) -> None:
        if not isinstance(self.backend, DebugLogBackend):
            raise ValueError("debug backend must be a DebugLogBackend")
        if (
            not isinstance(self.channel, str)
            or not self.channel.strip()
            or len(self.channel) > MAX_DEBUG_CHANNEL_CHARS
        ):
            raise ValueError("debug channel must be non-empty and bounded")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ValueError("debug payload must be bytes-like")
        payload = bytes(self.payload)
        if len(payload) > MAX_DEBUG_LOG_CHUNK_BYTES:
            raise ValueError("debug log chunk exceeds the bounded limit")
        if not isinstance(self.occurred_at, (int, float)) or isinstance(self.occurred_at, bool):
            raise ValueError("debug occurred_at must be numeric")
        if not math.isfinite(self.occurred_at):
            raise ValueError("debug occurred_at must be finite")
        object.__setattr__(self, "payload", payload)


class DebugLogPort(Protocol):
    """Attach-only raw debug I/O, consumed by an application worker."""

    @property
    def backend(self) -> DebugLogBackend:
        """Return the fixed backend family."""

    def open(self) -> None:
        """Attach to an already-authorized local endpoint."""

    def receive(self, max_bytes: int) -> DebugLogRecord | None:
        """Read one bounded raw record without parsing it in the adapter."""

    def send(self, channel: str, payload: bytes) -> None:
        """Perform an explicit bounded Down/write operation when supported."""

    def close(self) -> None:
        """Close the attach-only endpoint idempotently."""
