"""Stable, bounded contracts for future OTA transfer orchestration.

This module describes data and ports only. It does not read files, open
transports, flash targets, or implement cryptography.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
from typing import Protocol
from uuid import UUID

MAX_OTA_IMAGE_BYTES = 64 * 1024 * 1024
MAX_OTA_CHUNK_BYTES = 4 * 1024
MAX_OTA_LABEL_CHARS = 256
MAX_OTA_DETAIL_CHARS = 512


class OtaTransferProtocol(StrEnum):
    """Initially reserved OTA protocol families.

    These names reserve adapter boundaries; they do not mean that a protocol
    implementation or target bootloader compatibility exists today.
    """

    XMODEM = "xmodem"
    YMODEM = "ymodem"
    TFTP = "tftp"


class OtaTransferPhase(StrEnum):
    """Observable phases shared by future OTA workers and the UI."""

    PREPARING = "preparing"
    TRANSFERRING = "transferring"
    VERIFYING = "verifying"
    COMMITTING = "committing"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"


@dataclass(frozen=True, slots=True)
class OtaImageDescriptor:
    """Metadata for an image; the descriptor never contains secret material."""

    name: str
    version: str
    target: str
    size_bytes: int
    sha256_hex: str

    def __post_init__(self) -> None:
        if not isinstance(self.size_bytes, int) or isinstance(self.size_bytes, bool):
            raise ValueError("OTA image size must be an integer")
        labels = (("name", self.name), ("version", self.version), ("target", self.target))
        for label, value in labels:
            if not isinstance(value, str) or not value.strip() or len(value) > MAX_OTA_LABEL_CHARS:
                raise ValueError(f"OTA {label} must be non-empty and bounded")
        if not 0 < self.size_bytes <= MAX_OTA_IMAGE_BYTES:
            raise ValueError("OTA image size is outside the bounded range")
        if not isinstance(self.sha256_hex, str):
            raise ValueError("OTA image sha256_hex must be text")
        digest = self.sha256_hex.lower()
        if len(digest) != 64 or any(char not in "0123456789abcdef" for char in digest):
            raise ValueError("OTA image sha256_hex must contain exactly 64 hex characters")


class OtaImageSource(Protocol):
    """Bounded random-access image source owned by the application layer."""

    @property
    def descriptor(self) -> OtaImageDescriptor:
        """Return immutable metadata for the source."""

    def read(self, offset: int, max_bytes: int) -> bytes:
        """Read at most ``max_bytes`` from a validated image offset."""


@dataclass(frozen=True, slots=True)
class OtaTransferRequest:
    """Validated transfer intent passed from application orchestration."""

    image: OtaImageDescriptor
    protocol: OtaTransferProtocol
    chunk_size_bytes: int = 1024

    def __post_init__(self) -> None:
        if not isinstance(self.protocol, OtaTransferProtocol):
            raise ValueError("OTA protocol must be an OtaTransferProtocol")
        if not isinstance(self.chunk_size_bytes, int) or isinstance(self.chunk_size_bytes, bool):
            raise ValueError("OTA chunk size must be an integer")
        if not 0 < self.chunk_size_bytes <= MAX_OTA_CHUNK_BYTES:
            raise ValueError("OTA chunk size is outside the bounded range")


@dataclass(frozen=True, slots=True)
class OtaTransferProgress:
    """Bounded progress snapshot; it never implies target-side acceptance."""

    transfer_id: UUID
    phase: OtaTransferPhase
    bytes_sent: int
    total_bytes: int
    detail: str = ""

    def __post_init__(self) -> None:
        if (
            not isinstance(self.bytes_sent, int)
            or isinstance(self.bytes_sent, bool)
            or not isinstance(self.total_bytes, int)
            or isinstance(self.total_bytes, bool)
        ):
            raise ValueError("OTA progress byte counts must be integers")
        if not 0 <= self.bytes_sent <= self.total_bytes <= MAX_OTA_IMAGE_BYTES:
            raise ValueError("OTA progress is outside the bounded range")
        if not isinstance(self.detail, str) or len(self.detail) > MAX_OTA_DETAIL_CHARS:
            raise ValueError("OTA progress detail is outside the bounded range")


class OtaTransferPort(Protocol):
    """Protocol-specific transfer adapter owned by a future application worker."""

    @property
    def protocol(self) -> OtaTransferProtocol:
        """Return the fixed protocol family implemented by this adapter."""

    def start(self, source: OtaImageSource, request: OtaTransferRequest) -> UUID:
        """Start a bounded transfer without blocking the presentation thread."""

    def cancel(self, transfer_id: UUID) -> None:
        """Request cancellation; already transmitted bytes cannot be recalled."""

    def snapshot(self, transfer_id: UUID | None = None) -> OtaTransferProgress | None:
        """Return the latest progress snapshot, if a transfer exists."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop worker resources within a bounded shutdown budget."""
