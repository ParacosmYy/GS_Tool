"""Administrator-only read models and sanitized export queries.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep team-wide aggregation separate from personal usage queries.
Module: Admin reporting repository
"""

from __future__ import annotations

from typing import Any

from . import csv_export, db


def _safe_limit(value: Any, maximum: int = 200) -> int:
    try:
        parsed = int(value)
    except (TypeError, ValueError):
        parsed = 50
    return max(1, min(parsed, maximum))


def overview(path: str) -> dict[str, Any]:
    """Return aggregate metrics that contain no credentials or raw prompts."""

    with db.db_session(path) as connection:
        users = connection.execute("SELECT COUNT(*) AS count FROM users").fetchone()["count"]
        usage = connection.execute(
            """
            SELECT COUNT(*) AS calls,
                   COALESCE(SUM(input_tokens), 0) AS input_tokens,
                   COALESCE(SUM(output_tokens), 0) AS output_tokens,
                   COALESCE(SUM(input_tokens + output_tokens), 0) AS total_tokens
            FROM usage_records
            """
        ).fetchone()
        events = connection.execute(
            """
            SELECT COUNT(*) AS total,
                   COALESCE(SUM(CASE WHEN outcome = 'success' THEN 1 ELSE 0 END), 0) AS success,
                   COALESCE(SUM(CASE WHEN outcome = 'failure' THEN 1 ELSE 0 END), 0) AS failure
            FROM work_events
            """
        ).fetchone()
        logs = connection.execute("SELECT COUNT(*) AS total FROM app_logs").fetchone()["total"]
        models = connection.execute(
            """
            SELECT model, COUNT(*) AS calls,
                   COALESCE(SUM(input_tokens), 0) AS input_tokens,
                   COALESCE(SUM(output_tokens), 0) AS output_tokens,
                   COALESCE(SUM(input_tokens + output_tokens), 0) AS total_tokens
            FROM usage_records
            GROUP BY model
            ORDER BY total_tokens DESC, model ASC
            LIMIT 100
            """
        ).fetchall()
        trend = connection.execute(
            """
            SELECT substr(timestamp, 1, 10) AS day,
                   COALESCE(SUM(input_tokens), 0) AS input_tokens,
                   COALESCE(SUM(output_tokens), 0) AS output_tokens,
                   COALESCE(SUM(input_tokens + output_tokens), 0) AS total_tokens
            FROM usage_records
            GROUP BY substr(timestamp, 1, 10)
            ORDER BY day DESC
            LIMIT 30
            """
        ).fetchall()
    return {
        "users": int(users),
        "usage": {key: int(usage[key] or 0) for key in ("calls", "input_tokens", "output_tokens", "total_tokens")},
        "work_events": {key: int(events[key] or 0) for key in ("total", "success", "failure")},
        "logs": int(logs),
        "by_model": [dict(row) for row in models],
        "trend": list(reversed([dict(row) for row in trend])),
    }


def list_users(path: str, limit: Any = 50, offset: Any = 0) -> dict[str, Any]:
    """Return paged per-user aggregates with a stable, non-secret projection."""

    safe_limit = _safe_limit(limit)
    try:
        safe_offset = max(0, int(offset))
    except (TypeError, ValueError):
        safe_offset = 0
    with db.db_session(path) as connection:
        total = connection.execute("SELECT COUNT(*) AS count FROM users").fetchone()["count"]
        rows = connection.execute(
            """
            SELECT u.id, u.username, u.role, u.created_at,
                   COUNT(ur.id) AS calls,
                   COALESCE(SUM(ur.input_tokens), 0) AS input_tokens,
                   COALESCE(SUM(ur.output_tokens), 0) AS output_tokens,
                   COALESCE(SUM(ur.input_tokens + ur.output_tokens), 0) AS total_tokens,
                   (SELECT COUNT(*) FROM work_events we WHERE we.user_id = u.id) AS work_events,
                   (SELECT COUNT(*) FROM app_logs al WHERE al.user_id = u.id) AS logs
            FROM users u
            LEFT JOIN usage_records ur ON ur.user_id = u.id
            GROUP BY u.id
            ORDER BY CASE WHEN u.role = 'admin' THEN 0 ELSE 1 END, u.username ASC
            LIMIT ? OFFSET ?
            """,
            (safe_limit, safe_offset),
        ).fetchall()
    return {"items": [dict(row) for row in rows], "limit": safe_limit, "offset": safe_offset, "total": int(total)}


def user_activity(user_id: int, path: str, limit: Any = 100) -> dict[str, Any]:
    """Read one student's activity for an already authorized administrator."""

    safe_limit = _safe_limit(limit, 200)
    with db.db_session(path) as connection:
        user = connection.execute("SELECT id, username, role, created_at FROM users WHERE id = ?", (user_id,)).fetchone()
        records = connection.execute(
            """
            SELECT id, model, input_tokens, output_tokens,
                   input_tokens + output_tokens AS total_tokens, timestamp, note, source
            FROM usage_records WHERE user_id = ? ORDER BY timestamp DESC, id DESC LIMIT ?
            """,
            (user_id, safe_limit),
        ).fetchall()
        events = connection.execute(
            "SELECT * FROM work_events WHERE user_id = ? ORDER BY created_at DESC, id DESC LIMIT ?",
            (user_id, safe_limit),
        ).fetchall()
        logs = connection.execute(
            """
            SELECT id, request_id, level, event_type, message, error_code, metadata_json, created_at
            FROM app_logs WHERE user_id = ? ORDER BY created_at DESC, id DESC LIMIT ?
            """,
            (user_id, safe_limit),
        ).fetchall()
    if user is None:
        return {"user": None, "records": [], "events": [], "logs": []}
    return {
        "user": dict(user),
        "records": [dict(row) for row in records],
        "events": [dict(row) for row in events],
        "logs": [dict(row) for row in logs],
    }


def _export_projection(connection: Any, kind: str) -> tuple[list[str], Any]:
    """Build a fixed-column query; the returned cursor is consumed by the caller."""

    if kind == "usage":
        columns = ["id", "user_id", "model", "input_tokens", "output_tokens", "total_tokens", "timestamp", "note", "source"]
        query = """
            SELECT id, user_id, model, input_tokens, output_tokens,
                   input_tokens + output_tokens AS total_tokens, timestamp, note, source
            FROM usage_records ORDER BY timestamp ASC, id ASC
        """
    elif kind == "events":
        columns = ["id", "user_id", "request_id", "direction", "outcome", "duration_ms", "efficiency_score", "result_code", "error_code", "project", "task_type", "note", "created_at"]
        query = "SELECT " + ", ".join(columns) + " FROM work_events ORDER BY created_at ASC, id ASC"
    elif kind == "logs":
        columns = ["id", "user_id", "request_id", "level", "event_type", "message", "error_code", "metadata_json", "created_at"]
        query = "SELECT " + ", ".join(columns) + " FROM app_logs ORDER BY created_at ASC, id ASC"
    else:
        raise ValueError("kind must be usage, events, or logs")
    return columns, connection.execute(query)


def export_csv(kind: str, path: str) -> bytes:
    """Export one fixed projection with cursor iteration and explicit output bounds."""

    with db.db_session(path) as connection:
        columns, rows = _export_projection(connection, kind)
        return csv_export.export_rows(columns, rows)
