"""Request-correlation identifier policy for HTTP boundaries.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Normalize safe client correlation IDs or generate server-owned IDs.
Module: Cross-cutting observability infrastructure
"""

from __future__ import annotations

import re
import secrets


HEADER_NAME = "X-Request-ID"
MAX_LENGTH = 64
_SAFE_ID = re.compile(r"[A-Za-z0-9][A-Za-z0-9._:-]{0,63}\Z")


def resolve(candidate: str | None) -> str:
    """Return a bounded correlation ID without treating it as an identity."""

    value = str(candidate or "").strip()
    if _SAFE_ID.fullmatch(value):
        return value
    return secrets.token_hex(16)
