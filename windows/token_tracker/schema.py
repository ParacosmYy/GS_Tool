"""SQLite schema creation and additive migration boundary.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep DDL and compatibility migrations separate from query code.
Module: Infrastructure / schema lifecycle
"""

from __future__ import annotations

import sqlite3


def ensure_schema(connection: sqlite3.Connection) -> None:
    """Create current tables and apply only additive compatibility changes."""

    connection.executescript(
        """
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            created_at TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'user'
        );

        CREATE TABLE IF NOT EXISTS usage_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            model TEXT NOT NULL,
            input_tokens INTEGER NOT NULL CHECK (input_tokens >= 0),
            output_tokens INTEGER NOT NULL CHECK (output_tokens >= 0),
            timestamp TEXT NOT NULL,
            note TEXT NOT NULL DEFAULT '',
            source TEXT NOT NULL DEFAULT 'manual',
            idempotency_key TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_usage_user_timestamp
            ON usage_records(user_id, timestamp);
        CREATE INDEX IF NOT EXISTS idx_usage_user_model
            ON usage_records(user_id, model);

        CREATE TABLE IF NOT EXISTS auth_tokens (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            token_hash TEXT NOT NULL UNIQUE,
            token_type TEXT NOT NULL CHECK (token_type IN ('access', 'refresh')),
            created_at TEXT NOT NULL,
            expires_at TEXT NOT NULL,
            revoked_at TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_auth_tokens_hash
            ON auth_tokens(token_hash, token_type, expires_at);

        CREATE TABLE IF NOT EXISTS usage_ingest_tokens (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            token_hash TEXT NOT NULL UNIQUE,
            label TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL,
            expires_at TEXT NOT NULL,
            revoked_at TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_ingest_tokens_user
            ON usage_ingest_tokens(user_id, created_at);
        CREATE INDEX IF NOT EXISTS idx_ingest_tokens_hash
            ON usage_ingest_tokens(token_hash, expires_at);

        CREATE TABLE IF NOT EXISTS work_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            request_id TEXT NOT NULL,
            idempotency_key TEXT,
            direction TEXT NOT NULL,
            outcome TEXT NOT NULL,
            duration_ms INTEGER,
            efficiency_score INTEGER,
            result_code TEXT,
            error_code TEXT,
            project TEXT NOT NULL DEFAULT '',
            task_type TEXT NOT NULL DEFAULT '',
            note TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE (user_id, idempotency_key)
        );

        CREATE INDEX IF NOT EXISTS idx_work_events_user_created
            ON work_events(user_id, created_at);

        CREATE TABLE IF NOT EXISTS app_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            request_id TEXT NOT NULL,
            level TEXT NOT NULL,
            event_type TEXT NOT NULL,
            message TEXT NOT NULL,
            error_code TEXT,
            metadata_json TEXT NOT NULL DEFAULT '{}',
            created_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL
        );

        CREATE INDEX IF NOT EXISTS idx_app_logs_user_created
            ON app_logs(user_id, created_at);

        CREATE TABLE IF NOT EXISTS audit_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            actor_user_id INTEGER,
            target_user_id INTEGER,
            action TEXT NOT NULL,
            resource_type TEXT NOT NULL,
            resource_id TEXT NOT NULL DEFAULT '',
            request_id TEXT NOT NULL,
            metadata_json TEXT NOT NULL DEFAULT '{}',
            created_at TEXT NOT NULL,
            FOREIGN KEY (actor_user_id) REFERENCES users(id) ON DELETE SET NULL,
            FOREIGN KEY (target_user_id) REFERENCES users(id) ON DELETE SET NULL
        );

        CREATE INDEX IF NOT EXISTS idx_audit_events_created
            ON audit_events(created_at);

        CREATE TABLE IF NOT EXISTS rate_limit_buckets (
            key TEXT PRIMARY KEY,
            timestamps_json TEXT NOT NULL,
            updated_at REAL NOT NULL
        );

        CREATE INDEX IF NOT EXISTS idx_rate_limit_buckets_updated
            ON rate_limit_buckets(updated_at);
        """
    )
    _apply_compatibility_migrations(connection)


def _apply_compatibility_migrations(connection: sqlite3.Connection) -> None:
    """Apply additive migrations without rewriting historical rows."""

    user_columns = {row["name"] for row in connection.execute("PRAGMA table_info(users)").fetchall()}
    if "role" not in user_columns:
        connection.execute("ALTER TABLE users ADD COLUMN role TEXT NOT NULL DEFAULT 'user'")

    usage_columns = {row["name"] for row in connection.execute("PRAGMA table_info(usage_records)").fetchall()}
    if "idempotency_key" not in usage_columns:
        connection.execute("ALTER TABLE usage_records ADD COLUMN idempotency_key TEXT")

    connection.execute(
        """
        CREATE UNIQUE INDEX IF NOT EXISTS idx_usage_user_idempotency
            ON usage_records(user_id, idempotency_key)
            WHERE idempotency_key IS NOT NULL
        """
    )
