"""SQLite persistence and local-time reporting helpers.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Own schema, parameterized queries, aggregation, and CSV output.
"""

from __future__ import annotations

import os
import sqlite3
from contextlib import contextmanager
from datetime import date, datetime, time, timedelta
from pathlib import Path
from typing import Any, Iterator

from . import csv_export, schema


PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DB_PATH = PROJECT_ROOT / "data" / "token_tracker.sqlite3"
TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S"
MAX_IDEMPOTENCY_KEY_LENGTH = 160


def get_db_path(path: str | os.PathLike[str] | None = None) -> Path:
    """Return the configured database path and ensure its parent exists."""

    configured = path or os.getenv("TOKEN_TRACKER_DB") or DEFAULT_DB_PATH
    database_path = Path(configured).expanduser().resolve()
    database_path.parent.mkdir(parents=True, exist_ok=True)
    return database_path


def connect(path: str | os.PathLike[str] | None = None) -> sqlite3.Connection:
    connection = sqlite3.connect(str(get_db_path(path)))
    connection.row_factory = sqlite3.Row
    connection.execute("PRAGMA foreign_keys = ON")
    return connection


@contextmanager
def db_session(path: str | os.PathLike[str] | None = None) -> Iterator[sqlite3.Connection]:
    connection = connect(path)
    try:
        yield connection
        connection.commit()
    except Exception:
        connection.rollback()
        raise
    finally:
        connection.close()


def init_db(path: str | os.PathLike[str] | None = None) -> Path:
    """Create the database schema if it does not already exist."""

    database_path = get_db_path(path)
    with db_session(database_path) as connection:
        schema.ensure_schema(connection)
    return database_path


def local_now() -> datetime:
    """Return the current local time as a naive datetime.

    The application deliberately stores local wall-clock time because the product
    requirement is local-time reporting. The README documents this choice.
    """

    return datetime.now().replace(microsecond=0)


def local_now_string() -> str:
    return local_now().strftime(TIMESTAMP_FORMAT)


def normalize_timestamp(value: str | None) -> str:
    """Normalize common ISO/datetime-local values to local wall-clock text."""

    if value is None or not str(value).strip():
        return local_now_string()

    raw = str(value).strip()
    try:
        parsed = datetime.fromisoformat(raw.replace("Z", "+00:00"))
    except ValueError as exc:
        raise ValueError("timestamp must be ISO format, for example 2026-08-10 14:30") from exc

    if parsed.tzinfo is not None:
        parsed = parsed.astimezone().replace(tzinfo=None)
    return parsed.replace(microsecond=0).strftime(TIMESTAMP_FORMAT)


def _row_to_dict(row: sqlite3.Row | None) -> dict[str, Any] | None:
    return dict(row) if row is not None else None


def find_user(username: str, path: str | os.PathLike[str] | None = None) -> dict[str, Any] | None:
    with db_session(path) as connection:
        row = connection.execute(
            "SELECT id, username, password_hash, created_at, role FROM users WHERE username = ?",
            (username,),
        ).fetchone()
    return _row_to_dict(row)


def find_user_by_id(user_id: int, path: str | os.PathLike[str] | None = None) -> dict[str, Any] | None:
    with db_session(path) as connection:
        row = connection.execute(
            "SELECT id, username, password_hash, created_at, role FROM users WHERE id = ?",
            (user_id,),
        ).fetchone()
    return _row_to_dict(row)


def create_user(
    username: str,
    password_hash: str,
    path: str | os.PathLike[str] | None = None,
    role: str = "user",
) -> dict[str, Any]:
    normalized_role = role.strip().lower()
    if normalized_role not in {"user", "admin"}:
        raise ValueError("role must be user or admin")
    created_at = local_now_string()
    with db_session(path) as connection:
        cursor = connection.execute(
            "INSERT INTO users(username, password_hash, created_at, role) VALUES (?, ?, ?, ?)",
            (username, password_hash, created_at, normalized_role),
        )
        user_id = cursor.lastrowid
    return {
        "id": user_id,
        "username": username,
        "password_hash": password_hash,
        "created_at": created_at,
        "role": normalized_role,
    }


def set_user_role(
    username: str,
    role: str,
    path: str | os.PathLike[str] | None = None,
) -> dict[str, Any] | None:
    """Set a role for an explicitly named local account.

    This function is intentionally repository-level; the CLI is the only
    bootstrap surface until an audited administrator-management API exists.
    """

    normalized_role = role.strip().lower()
    if normalized_role not in {"user", "admin"}:
        raise ValueError("role must be user or admin")
    with db_session(path) as connection:
        connection.execute("UPDATE users SET role = ? WHERE username = ?", (normalized_role, username))
        row = connection.execute(
            "SELECT id, username, password_hash, created_at, role FROM users WHERE username = ?",
            (username,),
        ).fetchone()
    return _row_to_dict(row)


def insert_auth_token(
    user_id: int,
    token_hash: str,
    token_type: str,
    expires_at: str,
    path: str | os.PathLike[str] | None = None,
) -> int:
    """Persist only a token digest; the bearer secret never enters SQLite."""

    if token_type not in {"access", "refresh"}:
        raise ValueError("token_type must be access or refresh")
    with db_session(path) as connection:
        cursor = connection.execute(
            """
            INSERT INTO auth_tokens(user_id, token_hash, token_type, created_at, expires_at)
            VALUES (?, ?, ?, ?, ?)
            """,
            (user_id, token_hash, token_type, local_now_string(), expires_at),
        )
    return int(cursor.lastrowid)


def find_active_auth_token(
    token_hash: str,
    token_type: str,
    path: str | os.PathLike[str] | None = None,
) -> dict[str, Any] | None:
    with db_session(path) as connection:
        row = connection.execute(
            """
            SELECT id, user_id, token_type, expires_at
            FROM auth_tokens
            WHERE token_hash = ? AND token_type = ? AND revoked_at IS NULL AND expires_at > ?
            """,
            (token_hash, token_type, local_now_string()),
        ).fetchone()
    return _row_to_dict(row)


def revoke_auth_token(
    token_hash: str,
    token_type: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> bool:
    with db_session(path) as connection:
        if token_type:
            cursor = connection.execute(
                "UPDATE auth_tokens SET revoked_at = ? WHERE token_hash = ? AND token_type = ? AND revoked_at IS NULL",
                (local_now_string(), token_hash, token_type),
            )
        else:
            cursor = connection.execute(
                "UPDATE auth_tokens SET revoked_at = ? WHERE token_hash = ? AND revoked_at IS NULL",
                (local_now_string(), token_hash),
            )
    return cursor.rowcount > 0


def get_or_create_cli_user(
    username: str = "local",
    path: str | os.PathLike[str] | None = None,
) -> dict[str, Any]:
    """Get a CLI-only user without creating a usable web password."""

    existing = find_user(username, path)
    if existing:
        return existing
    return create_user(username, "!cli-only-user", path)


def insert_record(
    user_id: int,
    model: str,
    input_tokens: int,
    output_tokens: int,
    timestamp: str | None = None,
    note: str = "",
    source: str = "manual",
    path: str | os.PathLike[str] | None = None,
    idempotency_key: str | None = None,
) -> dict[str, Any]:
    """Insert a usage record while preserving the legacy dict return shape."""

    record, _ = insert_record_result(
        user_id=user_id,
        model=model,
        input_tokens=input_tokens,
        output_tokens=output_tokens,
        timestamp=timestamp,
        note=note,
        source=source,
        idempotency_key=idempotency_key,
        path=path,
    )
    return record


def insert_record_result(
    user_id: int,
    model: str,
    input_tokens: int,
    output_tokens: int,
    timestamp: str | None = None,
    note: str = "",
    source: str = "manual",
    path: str | os.PathLike[str] | None = None,
    idempotency_key: str | None = None,
) -> tuple[dict[str, Any], bool]:
    """Insert or replay one user-scoped usage record by idempotency key."""

    normalized_timestamp = normalize_timestamp(timestamp)
    normalized_key = str(idempotency_key or "").strip() or None
    if normalized_key and len(normalized_key) > MAX_IDEMPOTENCY_KEY_LENGTH:
        raise ValueError(
            f"idempotency_key must be {MAX_IDEMPOTENCY_KEY_LENGTH} characters or fewer"
        )
    with db_session(path) as connection:
        if normalized_key:
            existing = connection.execute(
                """
                SELECT id, user_id, model, input_tokens, output_tokens,
                       timestamp, note, source
                FROM usage_records
                WHERE user_id = ? AND idempotency_key = ?
                """,
                (user_id, normalized_key),
            ).fetchone()
            if existing:
                return _usage_record_projection(existing), True
        try:
            cursor = connection.execute(
                """
                INSERT INTO usage_records(
                    user_id, model, input_tokens, output_tokens, timestamp, note,
                    source, idempotency_key
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (
                    user_id,
                    model,
                    input_tokens,
                    output_tokens,
                    normalized_timestamp,
                    note,
                    source,
                    normalized_key,
                ),
            )
        except sqlite3.IntegrityError:
            # A concurrent request may win the unique index between the read
            # above and INSERT. Return that winner instead of double-counting.
            if not normalized_key:
                raise
            existing = connection.execute(
                """
                SELECT id, user_id, model, input_tokens, output_tokens,
                       timestamp, note, source
                FROM usage_records
                WHERE user_id = ? AND idempotency_key = ?
                """,
                (user_id, normalized_key),
            ).fetchone()
            if not existing:
                raise
            return _usage_record_projection(existing), True
        row = connection.execute(
            """
            SELECT id, user_id, model, input_tokens, output_tokens,
                   timestamp, note, source
            FROM usage_records
            WHERE id = ?
            """,
            (cursor.lastrowid,),
        ).fetchone()
        return _usage_record_projection(row), False


def _usage_record_projection(row: sqlite3.Row) -> dict[str, Any]:
    """Return a public record projection without exposing the retry key."""

    input_tokens = int(row["input_tokens"])
    output_tokens = int(row["output_tokens"])
    return {
        "id": int(row["id"]),
        "user_id": int(row["user_id"]),
        "model": row["model"],
        "input_tokens": input_tokens,
        "output_tokens": output_tokens,
        "total_tokens": input_tokens + output_tokens,
        "timestamp": row["timestamp"],
        "note": row["note"],
        "source": row["source"],
    }


def _where_clause(
    user_id: int,
    start: str | None = None,
    end: str | None = None,
) -> tuple[str, list[Any]]:
    clauses = ["user_id = ?"]
    params: list[Any] = [user_id]
    if start:
        clauses.append("timestamp >= ?")
        params.append(start)
    if end:
        clauses.append("timestamp < ?")
        params.append(end)
    return " AND ".join(clauses), params


def summary_by_model(
    user_id: int,
    start: str | None = None,
    end: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> list[dict[str, Any]]:
    where, params = _where_clause(user_id, start, end)
    with db_session(path) as connection:
        rows = connection.execute(
            f"""
            SELECT
                model,
                COUNT(*) AS calls,
                COALESCE(SUM(input_tokens), 0) AS input_tokens,
                COALESCE(SUM(output_tokens), 0) AS output_tokens,
                COALESCE(SUM(input_tokens + output_tokens), 0) AS total_tokens
            FROM usage_records
            WHERE {where}
            GROUP BY model
            ORDER BY total_tokens DESC, model ASC
            """,
            params,
        ).fetchall()
    return [dict(row) for row in rows]


def summary_totals(
    user_id: int,
    start: str | None = None,
    end: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> dict[str, int]:
    where, params = _where_clause(user_id, start, end)
    with db_session(path) as connection:
        row = connection.execute(
            f"""
            SELECT
                COUNT(*) AS calls,
                COALESCE(SUM(input_tokens), 0) AS input_tokens,
                COALESCE(SUM(output_tokens), 0) AS output_tokens,
                COALESCE(SUM(input_tokens + output_tokens), 0) AS total_tokens
            FROM usage_records
            WHERE {where}
            """,
            params,
        ).fetchone()
    return {key: int(row[key] or 0) for key in ("calls", "input_tokens", "output_tokens", "total_tokens")}


def recent_records(
    user_id: int,
    limit: int = 20,
    start: str | None = None,
    end: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> list[dict[str, Any]]:
    where, params = _where_clause(user_id, start, end)
    safe_limit = max(1, min(int(limit), 200))
    with db_session(path) as connection:
        rows = connection.execute(
            f"""
            SELECT id, model, input_tokens, output_tokens,
                   input_tokens + output_tokens AS total_tokens,
                   timestamp, note, source
            FROM usage_records
            WHERE {where}
            ORDER BY timestamp DESC, id DESC
            LIMIT ?
            """,
            [*params, safe_limit],
        ).fetchall()
    return [dict(row) for row in rows]


def recent_records_page(
    user_id: int,
    limit: int = 20,
    offset: int = 0,
    start: str | None = None,
    end: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> dict[str, Any]:
    """Return a bounded, user-isolated usage-record page for versioned APIs."""

    where, params = _where_clause(user_id, start, end)
    safe_limit = max(1, min(int(limit), 200))
    safe_offset = max(0, int(offset))
    with db_session(path) as connection:
        total = connection.execute(
            f"SELECT COUNT(*) AS count FROM usage_records WHERE {where}", params
        ).fetchone()["count"]
        rows = connection.execute(
            f"""
            SELECT id, model, input_tokens, output_tokens,
                   input_tokens + output_tokens AS total_tokens,
                   timestamp, note, source
            FROM usage_records
            WHERE {where}
            ORDER BY timestamp DESC, id DESC
            LIMIT ? OFFSET ?
            """,
            [*params, safe_limit, safe_offset],
        ).fetchall()
    return {
        "items": [dict(row) for row in rows],
        "pagination": {"limit": safe_limit, "offset": safe_offset, "total": int(total)},
    }


def _date_strings(start_date: date, end_date: date) -> list[str]:
    cursor = start_date
    result: list[str] = []
    while cursor < end_date:
        result.append(cursor.isoformat())
        cursor += timedelta(days=1)
    return result


def daily_trend(
    user_id: int,
    start: str | None,
    end: str | None,
    path: str | os.PathLike[str] | None = None,
) -> list[dict[str, int | str]]:
    """Return a zero-filled local-date series for charts."""

    if start:
        start_date = datetime.strptime(start, TIMESTAMP_FORMAT).date()
    else:
        start_date = local_now().date() - timedelta(days=29)
    if end:
        end_date = datetime.strptime(end, TIMESTAMP_FORMAT).date()
    else:
        end_date = local_now().date() + timedelta(days=1)

    where, params = _where_clause(user_id, start, end)
    with db_session(path) as connection:
        rows = connection.execute(
            f"""
            SELECT substr(timestamp, 1, 10) AS day,
                   COALESCE(SUM(input_tokens), 0) AS input_tokens,
                   COALESCE(SUM(output_tokens), 0) AS output_tokens,
                   COALESCE(SUM(input_tokens + output_tokens), 0) AS total_tokens
            FROM usage_records
            WHERE {where}
            GROUP BY substr(timestamp, 1, 10)
            ORDER BY day ASC
            """,
            params,
        ).fetchall()

    by_day = {row["day"]: dict(row) for row in rows}
    result: list[dict[str, int | str]] = []
    for day in _date_strings(start_date, end_date):
        item = by_day.get(day)
        result.append(
            {
                "day": day,
                "input_tokens": int(item["input_tokens"]) if item else 0,
                "output_tokens": int(item["output_tokens"]) if item else 0,
                "total_tokens": int(item["total_tokens"]) if item else 0,
            }
        )
    return result


def get_period_bounds(
    period: str = "day",
    date_from: str | None = None,
    date_to: str | None = None,
) -> tuple[str | None, str | None]:
    """Build half-open local-time bounds from a period or inclusive dates."""

    if date_from or date_to:
        start_date = date.fromisoformat(date_from) if date_from else None
        end_date = date.fromisoformat(date_to) if date_to else start_date
        if start_date is None and end_date is not None:
            start_date = end_date
        if start_date is not None and end_date is not None and end_date < start_date:
            raise ValueError("date_to cannot be earlier than date_from")
        start = datetime.combine(start_date, time.min).strftime(TIMESTAMP_FORMAT) if start_date else None
        exclusive_end = end_date + timedelta(days=1) if end_date else None
        end = datetime.combine(exclusive_end, time.min).strftime(TIMESTAMP_FORMAT) if exclusive_end else None
        return start, end

    today = local_now().date()
    normalized_period = (period or "day").lower()
    if normalized_period == "all":
        return None, None
    if normalized_period == "day":
        start_date, end_date = today, today + timedelta(days=1)
    elif normalized_period == "week":
        start_date = today - timedelta(days=today.weekday())
        end_date = start_date + timedelta(days=7)
    elif normalized_period == "month":
        start_date = today.replace(day=1)
        if start_date.month == 12:
            end_date = date(start_date.year + 1, 1, 1)
        else:
            end_date = date(start_date.year, start_date.month + 1, 1)
    else:
        raise ValueError("period must be one of: day, week, month, all")
    return (
        datetime.combine(start_date, time.min).strftime(TIMESTAMP_FORMAT),
        datetime.combine(end_date, time.min).strftime(TIMESTAMP_FORMAT),
    )


def summary_bundle(
    user_id: int,
    start: str | None = None,
    end: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> dict[str, Any]:
    trend_start, trend_end = start, end
    if trend_start is None and trend_end is None:
        trend_start = (local_now() - timedelta(days=29)).replace(hour=0, minute=0, second=0).strftime(TIMESTAMP_FORMAT)
        trend_end = (local_now() + timedelta(days=1)).replace(hour=0, minute=0, second=0).strftime(TIMESTAMP_FORMAT)
    return {
        "totals": summary_totals(user_id, start, end, path),
        "by_model": summary_by_model(user_id, start, end, path),
        "trend": daily_trend(user_id, trend_start, trend_end, path),
        "records": recent_records(user_id, 30, start, end, path),
    }


def export_csv(
    user_id: int,
    start: str | None = None,
    end: str | None = None,
    path: str | os.PathLike[str] | None = None,
) -> bytes:
    where, params = _where_clause(user_id, start, end)
    with db_session(path) as connection:
        rows = connection.execute(
            f"""
            SELECT id, model, input_tokens, output_tokens,
                   input_tokens + output_tokens AS total_tokens,
                   timestamp, note, source
            FROM usage_records
            WHERE {where}
            ORDER BY timestamp ASC, id ASC
            """,
            params,
        )
        columns = ("id", "model", "input_tokens", "output_tokens", "total_tokens", "timestamp", "note", "source")
        return csv_export.export_rows(columns, rows)
