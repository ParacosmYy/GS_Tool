"""Bearer-token lifecycle for Android and other API clients.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Issue short-lived access tokens and rotating refresh tokens without
         storing bearer secrets in the SQLite database or application logs.
Module: API v1 authentication boundary
"""

from __future__ import annotations

import hashlib
import secrets
from datetime import timedelta
from typing import Any

from . import db


ACCESS_TTL_SECONDS = 15 * 60
REFRESH_TTL_SECONDS = 30 * 24 * 60 * 60


def _digest(token: str) -> str:
    """Hash a bearer secret before it crosses the persistence boundary."""

    return hashlib.sha256(token.encode("utf-8")).hexdigest()


def _expiry(seconds: int) -> str:
    return (db.local_now() + timedelta(seconds=seconds)).strftime(db.TIMESTAMP_FORMAT)


def _public_user(user: dict[str, Any]) -> dict[str, Any]:
    """Return the cross-client user shape and deliberately omit password data."""

    return {"id": int(user["id"]), "username": user["username"], "role": user.get("role", "user")}


def issue_token_pair(
    user: dict[str, Any],
    path: str,
    access_ttl_seconds: int = ACCESS_TTL_SECONDS,
    refresh_ttl_seconds: int = REFRESH_TTL_SECONDS,
) -> dict[str, Any]:
    """Create an access/refresh pair for a verified account.

    The returned raw secrets are for one HTTPS response only. Callers must not
    persist or log them; the repository receives only SHA-256 digests.
    """

    access_token = secrets.token_urlsafe(32)
    refresh_token = secrets.token_urlsafe(48)
    db.insert_auth_token(user["id"], _digest(access_token), "access", _expiry(access_ttl_seconds), path)
    db.insert_auth_token(user["id"], _digest(refresh_token), "refresh", _expiry(refresh_ttl_seconds), path)
    return {
        "access_token": access_token,
        "refresh_token": refresh_token,
        "token_type": "Bearer",
        "expires_in": access_ttl_seconds,
        "user": _public_user(user),
    }


def resolve_access_user_id(
    authorization_header: str | None,
    path: str,
) -> int | None:
    """Resolve a bearer header to a user id, returning None for invalid input."""

    if not authorization_header:
        return None
    scheme, _, token = authorization_header.partition(" ")
    if scheme.lower() != "bearer" or not token or len(token) > 256:
        return None
    record = db.find_active_auth_token(_digest(token.strip()), "access", path)
    return int(record["user_id"]) if record else None


def revoke_access_token(authorization_header: str | None, path: str) -> bool:
    """Revoke the presented access token without exposing whether it existed."""

    if not authorization_header:
        return False
    scheme, _, token = authorization_header.partition(" ")
    if scheme.lower() != "bearer" or not token or len(token) > 256:
        return False
    return db.revoke_auth_token(_digest(token.strip()), "access", path)


def rotate_refresh_token(refresh_token: str, path: str) -> dict[str, Any] | None:
    """Atomically rotate a refresh token and return a fresh pair.

    Rotation occurs in one SQLite transaction so two concurrent retries cannot
    both redeem the same refresh secret. Only token digests are queried/written.
    """

    if not refresh_token or len(refresh_token) > 256:
        return None
    token_hash = _digest(refresh_token.strip())
    access_token = secrets.token_urlsafe(32)
    next_refresh = secrets.token_urlsafe(48)
    with db.db_session(path) as connection:
        row = connection.execute(
            """
            SELECT id, user_id
            FROM auth_tokens
            WHERE token_hash = ? AND token_type = 'refresh'
              AND revoked_at IS NULL AND expires_at > ?
            """,
            (token_hash, db.local_now_string()),
        ).fetchone()
        if row is None:
            return None
        revoked_at = db.local_now_string()
        connection.execute("UPDATE auth_tokens SET revoked_at = ? WHERE id = ?", (revoked_at, row["id"]))
        now = db.local_now_string()
        connection.executemany(
            """
            INSERT INTO auth_tokens(user_id, token_hash, token_type, created_at, expires_at)
            VALUES (?, ?, ?, ?, ?)
            """,
            [
                (row["user_id"], _digest(access_token), "access", now, _expiry(ACCESS_TTL_SECONDS)),
                (row["user_id"], _digest(next_refresh), "refresh", now, _expiry(REFRESH_TTL_SECONDS)),
            ],
        )
        user_row = connection.execute(
            "SELECT id, username, role FROM users WHERE id = ?", (row["user_id"],)
        ).fetchone()
    if user_row is None:
        return None
    return {
        "access_token": access_token,
        "refresh_token": next_refresh,
        "token_type": "Bearer",
        "expires_in": ACCESS_TTL_SECONDS,
        "user": {"id": user_row["id"], "username": user_row["username"], "role": user_row["role"]},
    }
