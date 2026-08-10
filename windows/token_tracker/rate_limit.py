"""Replaceable request throttling primitive.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep rate-limit policy outside HTTP controllers and provide a
         SQLite-backed shared mode for the current small Windows deployment.
Module: Cross-cutting infrastructure / request protection
"""

from __future__ import annotations

import hashlib
import json
import math
import sqlite3
import threading
import time
from pathlib import Path
from time import monotonic

from . import db


_LOCK = threading.Lock()
_BUCKETS: dict[str, list[float]] = {}
_DATABASE: Path | None = None
_PERSISTENT = False
_LAST_PRUNE = 0.0
_PRUNE_INTERVAL_SECONDS = 300.0
_USER_WRITE_POLICIES: dict[str, tuple[int, int]] = {
    "usage-record": (120, 60),
    "work-event": (180, 60),
    "app-log": (300, 60),
    "admin-export": (12, 60),
}


def configure(database: str | Path, *, persistent: bool) -> None:
    """Configure the storage adapter once during application composition.

    Local mode keeps the zero-write in-memory behavior. Shared/production/LAN
    modes use the SQLite table so separate Waitress workers see one window.
    Controllers continue to call only :func:`allow`.
    """

    global _DATABASE, _PERSISTENT, _LAST_PRUNE
    with _LOCK:
        _BUCKETS.clear()
    _DATABASE = Path(database).expanduser().resolve()
    _PERSISTENT = bool(persistent)
    _LAST_PRUNE = 0.0


def allow(bucket: str, identity: str, limit: int, window_seconds: int) -> bool:
    """Return whether a request fits the configured sliding window.

    Persistent mode uses a short SQLite transaction with a hashed bucket key;
    raw IP addresses, user IDs, credentials, and provider keys are not stored.
    Any storage failure fails closed so a locked/corrupt limiter cannot silently
    remove the protection from an authentication or provider endpoint.
    """

    if limit <= 0 or window_seconds <= 0:
        return False
    if _PERSISTENT and _DATABASE is not None:
        return _allow_persistent(bucket, identity, limit, window_seconds, _DATABASE)
    return _allow_memory(bucket, identity, limit, window_seconds)


def allow_user_write(resource: str, user_id: int) -> bool:
    """Apply one shared per-user write budget across Web and API v1.

    The policy is intentionally kept beside the replaceable limiter rather
    than copied into controllers. This prevents a user from bypassing the
    same protection by switching between the browser and Android surfaces.
    Unknown resource names fail closed so a new write route cannot silently
    ship without an explicit budget.
    """

    policy = _USER_WRITE_POLICIES.get(resource)
    if policy is None:
        return False
    try:
        identity = str(int(user_id))
    except (TypeError, ValueError):
        return False
    limit, window_seconds = policy
    return allow(f"user-write:{resource}", identity, limit, window_seconds)


def _allow_memory(bucket: str, identity: str, limit: int, window_seconds: int) -> bool:
    """Apply the original process-local sliding window for local mode."""

    key = f"{bucket}:{identity}"
    now = monotonic()
    with _LOCK:
        recent = [stamp for stamp in _BUCKETS.get(key, []) if now - stamp < window_seconds]
        if len(recent) >= limit:
            _BUCKETS[key] = recent
            return False
        recent.append(now)
        _BUCKETS[key] = recent
    return True


def _allow_persistent(
    bucket: str,
    identity: str,
    limit: int,
    window_seconds: int,
    database: Path,
) -> bool:
    """Apply one SQLite-backed sliding window shared by application workers."""

    storage_key = hashlib.sha256(f"{bucket}:{identity}".encode("utf-8")).hexdigest()
    now = time.time()
    cutoff = now - window_seconds
    try:
        with db.db_session(database) as connection:
            connection.execute("BEGIN IMMEDIATE")
            _prune_persistent(connection, now, window_seconds)
            row = connection.execute(
                "SELECT timestamps_json FROM rate_limit_buckets WHERE key = ?",
                (storage_key,),
            ).fetchone()
            recent = _decode_timestamps(row["timestamps_json"] if row else "[]", cutoff)
            allowed = len(recent) < limit
            if allowed:
                recent.append(now)
            connection.execute(
                """
                INSERT INTO rate_limit_buckets(key, timestamps_json, updated_at)
                VALUES (?, ?, ?)
                ON CONFLICT(key) DO UPDATE SET
                    timestamps_json = excluded.timestamps_json,
                    updated_at = excluded.updated_at
                """,
                (storage_key, json.dumps(recent, separators=(",", ":")), now),
            )
            return allowed
    except (OSError, sqlite3.Error, TypeError, ValueError):
        return False


def _decode_timestamps(value: str, cutoff: float) -> list[float]:
    """Decode only finite numeric timestamps inside the active window."""

    try:
        values = json.loads(value)
    except (TypeError, ValueError):
        return []
    if not isinstance(values, list):
        return []
    current = time.time()
    result: list[float] = []
    for item in values:
        if isinstance(item, bool) or not isinstance(item, (int, float)):
            continue
        timestamp = float(item)
        if math.isfinite(timestamp) and cutoff < timestamp <= current:
            result.append(timestamp)
    return result


def _prune_persistent(connection: sqlite3.Connection, now: float, window_seconds: int) -> None:
    """Bound stale limiter rows without adding a second scheduled worker."""

    global _LAST_PRUNE
    if now - _LAST_PRUNE < _PRUNE_INTERVAL_SECONDS:
        return
    connection.execute(
        "DELETE FROM rate_limit_buckets WHERE updated_at < ?",
        (now - max(window_seconds, _PRUNE_INTERVAL_SECONDS),),
    )
    _LAST_PRUNE = now
