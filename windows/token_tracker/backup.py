"""Local SQLite backup service.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Create verified, atomic backups without introducing a cloud dependency.
Module: Infrastructure / persistence operations

The service opens the source database read-only, uses SQLite's online backup API,
and verifies the resulting file before publishing it. It never copies bearer
tokens or provider keys into a separate export format.
"""

from __future__ import annotations

from contextlib import closing
import os
import sqlite3
from pathlib import Path
import uuid

from . import db


class BackupError(RuntimeError):
    """Raised when a verified local backup cannot be produced."""


def default_backup_dir(database: Path) -> Path:
    """Return the project-local backup directory beside the configured database."""

    return database.parent / "backups"


def create_backup(
    database: str | os.PathLike[str] | Path,
    output_dir: str | os.PathLike[str] | Path | None = None,
) -> Path:
    """Create one timestamped, integrity-checked SQLite backup atomically."""

    source = db.get_db_path(database)
    if not source.is_file():
        raise BackupError(f"数据库不存在：{source}")
    target_dir = Path(output_dir).expanduser().resolve() if output_dir else default_backup_dir(source)
    target_dir.mkdir(parents=True, exist_ok=True)
    stamp = db.local_now().strftime("%Y%m%d-%H%M%S")
    target = target_dir / f"token_tracker-{stamp}.sqlite3"
    while target.exists():
        target = target_dir / f"token_tracker-{stamp}-{os.getpid()}-{uuid.uuid4().hex[:8]}.sqlite3"
    partial = target.with_name(f".{target.name}.partial")
    if partial.exists():
        raise BackupError(f"临时备份文件已存在，请先处理：{partial}")

    try:
        with closing(sqlite3.connect(f"{source.as_uri()}?mode=ro", uri=True)) as source_connection:
            with closing(sqlite3.connect(partial)) as target_connection:
                source_connection.backup(target_connection)
                result = target_connection.execute("PRAGMA integrity_check").fetchone()
                if not result or result[0] != "ok":
                    raise BackupError("备份完整性检查未通过")
        os.replace(partial, target)
    except (BackupError, OSError, sqlite3.Error) as exc:
        try:
            if partial.exists():
                partial.unlink()
        except OSError as cleanup_error:
            raise BackupError(f"备份失败：{exc}；临时文件清理失败：{cleanup_error}") from exc
        if isinstance(exc, BackupError):
            raise
        raise BackupError(f"备份失败：{exc}") from exc
    return target


def verify_backup(backup_path: str | os.PathLike[str] | Path) -> dict[str, object]:
    """Read-only verify one SQLite backup's integrity and required schema."""

    source = Path(backup_path).expanduser().resolve()
    if not source.is_file():
        raise BackupError(f"备份文件不存在：{source}")
    try:
        with closing(sqlite3.connect(f"{source.as_uri()}?mode=ro", uri=True)) as connection:
            integrity = connection.execute("PRAGMA integrity_check").fetchone()
            if not integrity or integrity[0] != "ok":
                raise BackupError("备份完整性检查未通过")
            foreign_keys = connection.execute("PRAGMA foreign_key_check").fetchall()
            tables = {
                row[0]
                for row in connection.execute(
                    "SELECT name FROM sqlite_master WHERE type = 'table'"
                ).fetchall()
            }
    except (BackupError, OSError, sqlite3.Error) as exc:
        if isinstance(exc, BackupError):
            raise
        raise BackupError(f"备份验证失败：{exc}") from exc

    required_tables = {
        "users",
        "usage_records",
        "auth_tokens",
        "usage_ingest_tokens",
        "work_events",
        "app_logs",
        "audit_events",
        "rate_limit_buckets",
    }
    missing_tables = sorted(required_tables - tables)
    if missing_tables:
        raise BackupError(f"备份缺少必要表：{', '.join(missing_tables)}")
    if foreign_keys:
        raise BackupError(f"备份存在外键错误：{len(foreign_keys)} 条")
    return {
        "path": str(source),
        "integrity_check": "ok",
        "foreign_key_errors": 0,
        "tables": sorted(tables),
    }


def restore_backup(
    backup_path: str | os.PathLike[str] | Path,
    target_database: str | os.PathLike[str] | Path,
    *,
    overwrite: bool = False,
) -> dict[str, object]:
    """Restore a verified backup to one explicit target using an atomic publish.

    The source is opened read-only and the target is first written to a sibling
    partial file. Existing targets are never replaced unless the caller opts in
    with ``overwrite=True``; this keeps a mistyped restore command recoverable.
    """

    source = Path(backup_path).expanduser().resolve()
    target = Path(target_database).expanduser().resolve()
    if source == target:
        raise BackupError("恢复目标不能与备份源文件相同")
    static_root = Path(__file__).resolve().parent / "static"
    try:
        target.relative_to(static_root)
    except ValueError:
        pass
    else:
        raise BackupError("恢复目标不能位于网站 static 目录")
    verify_backup(source)
    if target.exists() and not overwrite:
        raise BackupError(f"恢复目标已存在，请明确指定覆盖：{target}")

    target.parent.mkdir(parents=True, exist_ok=True)
    partial = target.with_name(f".{target.name}.restore.partial")
    if partial.exists():
        raise BackupError(f"临时恢复文件已存在，请先处理：{partial}")

    try:
        with closing(sqlite3.connect(f"{source.as_uri()}?mode=ro", uri=True)) as source_connection:
            with closing(sqlite3.connect(partial)) as target_connection:
                source_connection.backup(target_connection)
                target_connection.commit()
        verify_backup(partial)
        os.replace(partial, target)
    except (BackupError, OSError, sqlite3.Error) as exc:
        try:
            if partial.exists():
                partial.unlink()
        except OSError as cleanup_error:
            raise BackupError(f"恢复失败：{exc}；临时文件清理失败：{cleanup_error}") from exc
        if isinstance(exc, BackupError):
            raise
        raise BackupError(f"恢复失败：{exc}") from exc

    return {
        "path": str(target),
        "integrity_check": "ok",
        "overwritten": bool(overwrite),
    }
