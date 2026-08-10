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
from datetime import datetime
import os
import sqlite3
from pathlib import Path
import stat
import uuid

from . import db


class BackupError(RuntimeError):
    """Raised when a verified local backup cannot be produced."""


MAX_INVENTORY_FILES = 512
BACKUP_NAME_PREFIX = "token_tracker-"
BACKUP_SUFFIX = ".sqlite3"


def default_backup_dir(database: Path) -> Path:
    """Return the project-local backup directory beside the configured database."""

    return database.parent / "backups"


def inventory_backups(
    output_dir: str | os.PathLike[str] | Path,
    *,
    min_count: int | None = None,
    max_age_days: int | None = None,
    max_size_mib: int | None = None,
    verify: bool = False,
) -> dict[str, object]:
    """Read backup metadata and evaluate optional retention policies.

    The inventory deliberately operates on directory metadata and, when
    requested, the existing read-only verifier. It never creates the target
    directory, opens the configured live database, recursively traverses
    folders, deletes files, or returns SQLite business rows.
    """

    _validate_inventory_policy(min_count, max_age_days, max_size_mib)
    directory = Path(output_dir).expanduser().resolve()
    if directory.exists() and not directory.is_dir():
        raise BackupError(f"备份清单目录不是文件夹：{directory}")

    candidates = _inventory_candidates(directory)

    now = db.local_now()
    files: list[dict[str, object]] = []
    age_seconds_values: list[float] = []
    verification_failures = 0
    for candidate in sorted(candidates, key=lambda item: item.name):
        try:
            metadata = candidate.stat()
            modified = datetime.fromtimestamp(metadata.st_mtime).replace(microsecond=0)
        except (OSError, OverflowError, ValueError) as exc:
            raise BackupError(f"读取备份元数据失败：{candidate.name}") from exc
        age_seconds = max(0.0, (now - modified).total_seconds())
        age_seconds_values.append(age_seconds)
        item: dict[str, object] = {
            "name": candidate.name,
            "size_bytes": metadata.st_size,
            "modified_at": modified.strftime(db.TIMESTAMP_FORMAT),
            "age_days": round(age_seconds / 86400, 2),
            "integrity": "not_checked",
        }
        if verify:
            try:
                verify_backup(candidate)
            except BackupError as exc:
                verification_failures += 1
                item["integrity"] = "failed"
                item["verification_error"] = str(exc)[:200]
            else:
                item["integrity"] = "ok"
        files.append(item)

    total_size_bytes = sum(int(item["size_bytes"]) for item in files)
    modified_values = [str(item["modified_at"]) for item in files]
    violations: list[dict[str, object]] = []
    if not directory.exists():
        violations.append({"code": "BACKUP_DIRECTORY_MISSING", "message": "备份目录不存在"})
    elif not files:
        violations.append({"code": "NO_BACKUPS", "message": "目录中没有可识别的 SQLite 备份"})
    if min_count is not None and len(files) < min_count:
        violations.append({"code": "MIN_COUNT", "message": f"备份数量 {len(files)} 小于最小值 {min_count}"})
    if max_age_days is not None and any(age > max_age_days * 86400 for age in age_seconds_values):
        violations.append({"code": "MAX_AGE_DAYS", "message": f"存在超过 {max_age_days} 天的备份"})
    if max_size_mib is not None and total_size_bytes > max_size_mib * 1024 * 1024:
        violations.append({"code": "MAX_SIZE_MIB", "message": f"备份总容量超过 {max_size_mib} MiB"})
    if verification_failures:
        violations.append({"code": "INTEGRITY_CHECK", "message": f"有 {verification_failures} 个备份完整性验证失败"})

    return {
        "directory": str(directory),
        "status": "pass" if not violations else "attention",
        "policy": {
            "min_count": min_count,
            "max_age_days": max_age_days,
            "max_size_mib": max_size_mib,
            "verify": verify,
        },
        "summary": {
            "count": len(files),
            "total_size_bytes": total_size_bytes,
            "latest_modified_at": max(modified_values) if modified_values else None,
            "oldest_modified_at": min(modified_values) if modified_values else None,
            "verification_failures": verification_failures,
        },
        "violations": violations,
        "files": files,
    }


def _validate_inventory_policy(
    min_count: int | None,
    max_age_days: int | None,
    max_size_mib: int | None,
) -> None:
    """Reject negative inventory policy values before touching the filesystem."""

    for label, value in (
        ("最小备份数量", min_count),
        ("最大备份年龄", max_age_days),
        ("最大容量", max_size_mib),
    ):
        if value is not None and value < 0:
            raise BackupError(f"{label}不能为负数")


def _inventory_candidates(directory: Path) -> list[Path]:
    """Return bounded, direct-child regular SQLite backup files only."""

    if not directory.exists():
        return []
    try:
        entries = directory.iterdir()
    except OSError as exc:
        raise BackupError(f"读取备份目录失败：{directory}") from exc
    candidates: list[Path] = []
    for entry in entries:
        if not entry.name.startswith(BACKUP_NAME_PREFIX) or not entry.name.endswith(BACKUP_SUFFIX):
            continue
        try:
            is_regular = not entry.is_symlink() and stat.S_ISREG(entry.stat().st_mode)
        except OSError:
            is_regular = False
        if is_regular:
            candidates.append(entry)
            if len(candidates) > MAX_INVENTORY_FILES:
                raise BackupError(f"备份文件数量超过只读扫描上限：{MAX_INVENTORY_FILES}")
    return candidates


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
