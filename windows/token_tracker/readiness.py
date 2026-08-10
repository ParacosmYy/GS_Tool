"""Central-service readiness probe for deployment and edge checks.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Distinguish a live HTTP process from a usable SQLite application state.
Module: Application / operational readiness boundary
"""

from __future__ import annotations

import sqlite3
from pathlib import Path


_REQUIRED_TABLE_NAMES = (
    "users",
    "usage_records",
    "auth_tokens",
    "work_events",
    "app_logs",
    "audit_events",
    "rate_limit_buckets",
)
REQUIRED_TABLES = frozenset(_REQUIRED_TABLE_NAMES)
_TABLE_PLACEHOLDERS = ", ".join("?" for _ in _REQUIRED_TABLE_NAMES)


def is_ready(path: str) -> bool:
    """Return whether the core schema is readable without exposing DB details."""

    connection: sqlite3.Connection | None = None
    try:
        database_path = Path(path).expanduser().resolve(strict=True)
        if not database_path.is_file():
            return False
        connection = sqlite3.connect(f"{database_path.as_uri()}?mode=ro", uri=True)
        connection.row_factory = sqlite3.Row
        rows = connection.execute(
            "SELECT name FROM sqlite_master WHERE type = 'table' AND name IN ("
            f"{_TABLE_PLACEHOLDERS})",
            _REQUIRED_TABLE_NAMES,
        ).fetchall()
    except (OSError, sqlite3.Error, TypeError, ValueError):
        return False
    finally:
        if connection is not None:
            connection.close()
    existing = {str(row["name"]) for row in rows}
    return REQUIRED_TABLES.issubset(existing)
