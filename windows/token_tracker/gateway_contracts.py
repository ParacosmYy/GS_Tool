"""Shared contracts for the local usage Gateway integration.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep the usage DTO independent from HTTP delivery and storage code.
Module: Local integration / stable internal contract
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class UsageReport:
    """Authoritative, non-sensitive usage facts accepted by central ingest."""

    model: str
    input_tokens: int
    output_tokens: int
    idempotency_key: str
    note: str = "本地 Gateway 自动采集"

    def to_payload(self) -> dict[str, object]:
        """Return the bounded payload shared by HTTP and encrypted storage."""

        return {
            "model": self.model,
            "input_tokens": self.input_tokens,
            "output_tokens": self.output_tokens,
            "idempotency_key": self.idempotency_key,
            "note": self.note,
        }
