"""Authenticated OTA security contracts.

Only authenticated-encryption candidates are exposed. Implementations must
come from an approved, maintained cryptographic backend and must fail closed;
this package intentionally contains no AES implementation.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
from typing import Protocol
from uuid import UUID

from ..contracts import OtaImageDescriptor

MAX_KEY_ID_CHARS = 128
MAX_SECURITY_DETAIL_CHARS = 512


class OtaSecuritySuite(StrEnum):
    """Authenticated-encryption suites reserved for target-specific adapters."""

    AES_256_GCM = "aes-256-gcm"
    AES_128_CCM = "aes-128-ccm"


@dataclass(frozen=True, slots=True)
class OtaSecurityProfile:
    """Key references and policy flags; raw keys never enter this DTO."""

    suite: OtaSecuritySuite
    key_id: str
    require_signature: bool = True
    signature_key_id: str | None = None
    require_anti_rollback: bool = True

    def __post_init__(self) -> None:
        if not isinstance(self.suite, OtaSecuritySuite):
            raise ValueError("OTA security suite must be an OtaSecuritySuite")
        if not isinstance(self.require_signature, bool) or not self.require_signature:
            raise ValueError("OTA signature verification cannot be disabled")
        if not isinstance(self.require_anti_rollback, bool) or not self.require_anti_rollback:
            raise ValueError("OTA anti-rollback cannot be disabled")
        if (
            not isinstance(self.key_id, str)
            or not self.key_id.strip()
            or len(self.key_id) > MAX_KEY_ID_CHARS
        ):
            raise ValueError("OTA key_id must be non-empty and bounded")
        if self.require_signature and (
            not isinstance(self.signature_key_id, str)
            or not self.signature_key_id.strip()
            or len(self.signature_key_id) > MAX_KEY_ID_CHARS
        ):
            raise ValueError("signature_key_id is required when signature verification is enabled")


class OtaVerificationStatus(StrEnum):
    """Fail-closed outcome categories for a future verification worker."""

    UNVERIFIED = "unverified"
    VERIFIED = "verified"
    REJECTED = "rejected"


@dataclass(frozen=True, slots=True)
class OtaVerificationResult:
    """Explicit integrity/authenticity outcome for UI and audit logging."""

    status: OtaVerificationStatus
    digest_matches: bool
    signature_valid: bool
    anti_rollback_allowed: bool
    detail: str = ""

    def __post_init__(self) -> None:
        if not isinstance(self.status, OtaVerificationStatus):
            raise ValueError("OTA verification status must be an OtaVerificationStatus")
        if not isinstance(self.detail, str) or len(self.detail) > MAX_SECURITY_DETAIL_CHARS:
            raise ValueError("OTA verification detail is outside the bounded range")


class OtaSecurityPort(Protocol):
    """Streaming security adapter kept outside transport and presentation."""

    def begin(self, image: OtaImageDescriptor, profile: OtaSecurityProfile) -> UUID:
        """Create a verification/decryption session without exposing raw keys."""

    def transform(self, session_id: UUID, encrypted_chunk: bytes, *, final: bool = False) -> bytes:
        """Authenticate/decrypt one bounded chunk or raise on invalid input."""

    def finalize(self, session_id: UUID) -> OtaVerificationResult:
        """Verify digest, signature, and anti-rollback policy before activation."""

    def cancel(self, session_id: UUID) -> None:
        """Discard sensitive session state and any partial output."""
