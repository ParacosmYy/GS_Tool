"""Structured work, diagnostic-log, and audit-event use cases.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Validate cross-client activity payloads and persist privacy-safe events.
Module: Event and observability boundary
"""

from __future__ import annotations

import json
import re
import sqlite3
from typing import Any

from . import db


VALID_DIRECTIONS = {"coding", "debugging", "research", "writing", "planning", "review", "other"}
VALID_OUTCOMES = {"success", "failure", "partial"}
VALID_LOG_LEVELS = {"debug", "info", "warning", "error"}
MAX_REQUEST_ID_LENGTH = 100
_TEXT_SECRET_PATTERN = re.compile(
    r"(?:api[_ -]?key|authorization|proxy-authorization|access[_ -]?token|refresh[_ -]?token)"
    r"\s*(?:[:=]|\s)\s*(?:(?:bearer|token)\s+)?[^\s,;]+"
    r"|token\s*[:=]\s*(?:(?:bearer|token)\s+)?[^\s,;]+"
    r"|\bbearer\s+[^\s,;]+"
    r"|\bsk-[a-z0-9_-]+",
    flags=re.IGNORECASE,
)
_SENSITIVE_KEY_PATTERN = re.compile(
    r"(?:api[_ -]?key|authorization|proxy-authorization|access[_ -]?token|refresh[_ -]?token|token)",
    flags=re.IGNORECASE,
)


class EventValidationError(ValueError):
    """Raised when a client event violates the public event contract."""


def _text(value: Any, field: str, maximum: int, required: bool = False) -> str:
    text = str(value or "").strip()
    if required and not text:
        raise EventValidationError(f"{field} is required")
    if len(text) > maximum:
        raise EventValidationError(f"{field} must be {maximum} characters or fewer")
    return text


def _non_negative(value: Any, field: str, maximum: int | None = None) -> int | None:
    if value is None or value == "":
        return None
    if isinstance(value, bool):
        raise EventValidationError(f"{field} must be a non-negative integer")
    try:
        number = int(str(value).strip())
    except (TypeError, ValueError) as exc:
        raise EventValidationError(f"{field} must be a non-negative integer") from exc
    if number < 0 or (maximum is not None and number > maximum):
        suffix = f" and no greater than {maximum}" if maximum is not None else ""
        raise EventValidationError(f"{field} must be a non-negative integer{suffix}")
    return number


def _code(value: Any, field: str) -> str | None:
    text = _text(value, field, 80)
    if not text:
        return None
    if not re.fullmatch(r"[A-Za-z0-9_.:-]+", text):
        raise EventValidationError(f"{field} contains unsupported characters")
    return text


def _safe_json(value: Any) -> str:
    """Keep logs bounded and redact likely credentials before persistence."""

    if not isinstance(value, dict):
        return "{}"
    limited: dict[str, str] = {}
    for key, item in list(value.items())[:20]:
        key_text = str(key)[:60]
        limited[key_text] = (
            "[REDACTED]"
            if _SENSITIVE_KEY_PATTERN.search(key_text)
            else _redact_text(str(item)[:300])
        )
    encoded = json.dumps(limited, ensure_ascii=False, separators=(",", ":"))
    if len(encoded) <= 4000:
        return encoded

    compact: dict[str, str | bool] = {}
    for key, item in limited.items():
        candidate = dict(compact)
        candidate[key] = item
        if len(json.dumps(candidate, ensure_ascii=False, separators=(",", ":"))) > 3900:
            break
        compact[key] = item
    compact["_truncated"] = True
    return json.dumps(compact, ensure_ascii=False, separators=(",", ":"))


def _redact_text(value: str) -> str:
    """Redact credential-like text while keeping surrounding prose usable."""

    return _TEXT_SECRET_PATTERN.sub("[REDACTED]", value)


def normalize_work_event(payload: dict[str, Any]) -> dict[str, Any]:
    """Validate the public work-event shape without accepting raw prompt data."""

    direction = _text(payload.get("direction"), "direction", 30, required=True).lower()
    if direction not in VALID_DIRECTIONS:
        raise EventValidationError("direction is not supported")
    outcome = _text(payload.get("outcome"), "outcome", 20, required=True).lower()
    if outcome not in VALID_OUTCOMES:
        raise EventValidationError("outcome is not supported")
    duration_ms = _non_negative(payload.get("duration_ms"), "duration_ms", 7_200_000)
    efficiency_score = _non_negative(payload.get("efficiency_score"), "efficiency_score", 100)
    return {
        "direction": direction,
        "outcome": outcome,
        "duration_ms": duration_ms,
        "efficiency_score": efficiency_score,
        "result_code": _code(payload.get("result_code"), "result_code"),
        "error_code": _code(payload.get("error_code"), "error_code"),
        "project": _redact_text(_text(payload.get("project"), "project", 120)),
        "task_type": _redact_text(_text(payload.get("task_type"), "task_type", 80)),
        "note": _redact_text(_text(payload.get("note"), "note", 1000)),
    }


def insert_work_event(
    user_id: int,
    payload: dict[str, Any],
    request_id: str,
    idempotency_key: str | None,
    path: str,
) -> tuple[dict[str, Any], bool]:
    """Insert an event or return the existing row for a repeated sync key."""

    normalized = normalize_work_event(payload)
    request_value = _text(request_id, "request_id", MAX_REQUEST_ID_LENGTH, required=True)
    idempotency_value = _text(idempotency_key, "idempotency_key", 160) or None
    created_at = db.local_now_string()
    columns = (
        "user_id", "request_id", "idempotency_key", "direction", "outcome", "duration_ms",
        "efficiency_score", "result_code", "error_code", "project", "task_type", "note", "created_at",
    )
    values = (
        user_id, request_value, idempotency_value, normalized["direction"], normalized["outcome"],
        normalized["duration_ms"], normalized["efficiency_score"], normalized["result_code"],
        normalized["error_code"], normalized["project"], normalized["task_type"], normalized["note"], created_at,
    )
    try:
        with db.db_session(path) as connection:
            if idempotency_value:
                existing = connection.execute(
                    "SELECT * FROM work_events WHERE user_id = ? AND idempotency_key = ?",
                    (user_id, idempotency_value),
                ).fetchone()
                if existing:
                    return dict(existing), True
            placeholders = ", ".join("?" for _ in columns)
            connection.execute(
                f"INSERT INTO work_events({', '.join(columns)}) VALUES ({placeholders})", values
            )
            row = connection.execute("SELECT * FROM work_events WHERE id = last_insert_rowid()").fetchone()
    except sqlite3.IntegrityError:
        if idempotency_value:
            with db.db_session(path) as connection:
                existing = connection.execute(
                    "SELECT * FROM work_events WHERE user_id = ? AND idempotency_key = ?",
                    (user_id, idempotency_value),
                ).fetchone()
            if existing:
                return dict(existing), True
        raise
    return dict(row), False


def recent_work_events(user_id: int, limit: int, offset: int, path: str) -> dict[str, Any]:
    """Return a bounded, stable page for Android and future clients."""

    safe_limit = max(1, min(int(limit), 200))
    safe_offset = max(0, int(offset))
    with db.db_session(path) as connection:
        total = connection.execute(
            "SELECT COUNT(*) AS count FROM work_events WHERE user_id = ?", (user_id,)
        ).fetchone()["count"]
        rows = connection.execute(
            """
            SELECT * FROM work_events
            WHERE user_id = ?
            ORDER BY created_at DESC, id DESC
            LIMIT ? OFFSET ?
            """,
            (user_id, safe_limit, safe_offset),
        ).fetchall()
    return {
        "items": [dict(row) for row in rows],
        "pagination": {"limit": safe_limit, "offset": safe_offset, "total": int(total)},
    }


def insert_app_log(
    user_id: int | None,
    payload: dict[str, Any],
    request_id: str,
    path: str,
) -> dict[str, Any]:
    """Persist a bounded diagnostic log after redacting credential-like values."""

    level = _text(payload.get("level", "info"), "level", 12).lower()
    event_type = _redact_text(_text(payload.get("event_type"), "event_type", 80, required=True))
    message = _text(payload.get("message"), "message", 2000, required=True)
    if level not in VALID_LOG_LEVELS:
        raise EventValidationError("level is not supported")
    safe_message = _redact_text(message)
    error_code = _code(payload.get("error_code"), "error_code")
    request_value = _text(payload.get("request_id") or request_id, "request_id", MAX_REQUEST_ID_LENGTH, True)
    created_at = db.local_now_string()
    metadata_json = _safe_json(payload.get("metadata"))
    with db.db_session(path) as connection:
        cursor = connection.execute(
            """
            INSERT INTO app_logs(user_id, request_id, level, event_type, message, error_code, metadata_json, created_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (user_id, request_value, level, event_type, safe_message, error_code, metadata_json, created_at),
        )
        log_id = cursor.lastrowid
    return {
        "id": log_id, "user_id": user_id, "request_id": request_value, "level": level,
        "event_type": event_type, "message": safe_message, "error_code": error_code,
        "metadata": json.loads(metadata_json), "created_at": created_at,
    }


def recent_app_logs(user_id: int, limit: int, offset: int, path: str) -> dict[str, Any]:
    """Return a bounded diagnostic-log page for one authenticated user."""

    safe_limit = max(1, min(int(limit), 200))
    safe_offset = max(0, int(offset))
    with db.db_session(path) as connection:
        total = connection.execute(
            "SELECT COUNT(*) AS count FROM app_logs WHERE user_id = ?", (user_id,)
        ).fetchone()["count"]
        rows = connection.execute(
            """
            SELECT id, request_id, level, event_type, message, error_code, metadata_json, created_at
            FROM app_logs
            WHERE user_id = ?
            ORDER BY created_at DESC, id DESC
            LIMIT ? OFFSET ?
            """,
            (user_id, safe_limit, safe_offset),
        ).fetchall()
    return {
        "items": [dict(row) for row in rows],
        "pagination": {"limit": safe_limit, "offset": safe_offset, "total": int(total)},
    }


def insert_audit_event(
    actor_user_id: int | None,
    action: str,
    resource_type: str,
    resource_id: str,
    request_id: str,
    path: str,
    target_user_id: int | None = None,
    metadata: dict[str, Any] | None = None,
) -> dict[str, Any]:
    """Record an administrative/security action without sensitive payloads."""

    action_value = _text(action, "action", 80, required=True)
    resource_value = _text(resource_type, "resource_type", 80, required=True)
    resource_id_value = _text(resource_id, "resource_id", 120)
    request_value = _text(request_id, "request_id", MAX_REQUEST_ID_LENGTH, required=True)
    created_at = db.local_now_string()
    metadata_json = _safe_json(metadata)
    with db.db_session(path) as connection:
        cursor = connection.execute(
            """
            INSERT INTO audit_events(
                actor_user_id, target_user_id, action, resource_type, resource_id,
                request_id, metadata_json, created_at
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (actor_user_id, target_user_id, action_value, resource_value, resource_id_value,
             request_value, metadata_json, created_at),
        )
        audit_id = cursor.lastrowid
    return {"id": audit_id, "action": action_value, "resource_type": resource_value, "created_at": created_at}
