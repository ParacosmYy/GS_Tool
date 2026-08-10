"""External usage-ingest token lifecycle.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Issue and resolve revocable per-user usage collection credentials.
Module: API authentication / external ingest boundary
"""

from __future__ import annotations

import hashlib
import secrets
from datetime import timedelta
from typing import Any

from . import db


INGEST_TOKEN_HEADER = "X-AI-Tracker-Ingest-Token"
MAX_TOKEN_LENGTH = 256
MAX_LABEL_LENGTH = 80
MIN_EXPIRY_DAYS = 1
MAX_EXPIRY_DAYS = 3650


def issue_token(
    user_id: int,
    label: Any,
    expires_days: Any,
    path: str,
) -> dict[str, Any]:
    """Create a one-time-display token and persist only its digest."""

    label_value = _label(label)
    days = _expiry_days(expires_days)
    raw_token = f"ait_{secrets.token_urlsafe(32)}"
    expires_at = (db.local_now() + timedelta(days=days)).strftime(db.TIMESTAMP_FORMAT)
    token_id = db.insert_ingest_token(user_id, _digest(raw_token), label_value, expires_at, path)
    return {
        "id": token_id,
        "label": label_value,
        "expires_at": expires_at,
        "token": raw_token,
    }


def resolve_user_id(header_value: str | None, path: str) -> int | None:
    """Resolve a header token without exposing whether a digest exists."""

    token = str(header_value or "").strip()
    if not token or len(token) > MAX_TOKEN_LENGTH:
        return None
    record = db.find_active_ingest_token(_digest(token), path)
    return int(record["user_id"]) if record else None


def _digest(token: str) -> str:
    return hashlib.sha256(token.encode("utf-8")).hexdigest()


def _label(value: Any) -> str:
    label = str(value or "").strip()
    if len(label) > MAX_LABEL_LENGTH:
        raise ValueError(f"label must be {MAX_LABEL_LENGTH} characters or fewer")
    return label or "external-client"


def _expiry_days(value: Any) -> int:
    try:
        days = int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError("expires_days must be an integer") from exc
    if not MIN_EXPIRY_DAYS <= days <= MAX_EXPIRY_DAYS:
        raise ValueError(f"expires_days must be between {MIN_EXPIRY_DAYS} and {MAX_EXPIRY_DAYS}")
    return days
