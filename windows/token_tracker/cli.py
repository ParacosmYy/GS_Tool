"""Command-line interface for AI Token Tracker.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Parse terminal commands and delegate to application services.
"""

from __future__ import annotations

import argparse
import os
import secrets
import sys
from pathlib import Path
from typing import Iterable

from . import backup, csv_export, db, deployment_checks, events, release_audit
from .services import UsageValidationError, add_usage, query_range


def non_negative_int(value: str) -> int:
    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("must be a non-negative integer") from exc
    if parsed < 0:
        raise argparse.ArgumentTypeError("must be a non-negative integer")
    return parsed


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="ai-token-tracker",
        description="记录、汇总并导出不同 AI 模型的 token 用量。时间按本地时间处理。",
    )
    parser.add_argument(
        "--db",
        default=None,
        help="SQLite 文件路径，默认使用 TOKEN_TRACKER_DB 或 data/token_tracker.sqlite3",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    init_parser = subparsers.add_parser("init", help="初始化 SQLite 数据库")
    init_parser.set_defaults(handler=cmd_init)

    add_parser = subparsers.add_parser("add", help="添加一条 token 用量记录")
    add_parser.add_argument("--model", "-m", required=True, help="模型名称，例如 kimi-code、gpt-4o")
    add_parser.add_argument("--input", "-i", dest="input_tokens", required=True, type=non_negative_int, help="输入 token 数")
    add_parser.add_argument("--output", "-o", dest="output_tokens", required=True, type=non_negative_int, help="输出 token 数")
    add_parser.add_argument("--timestamp", help="本地时间，例如 2026-08-10 14:30；省略则使用当前时间")
    add_parser.add_argument("--note", default="", help="备注")
    add_parser.add_argument("--user", default="local", help="CLI 用户名，默认 local")
    add_parser.set_defaults(handler=cmd_add)

    summary_parser = subparsers.add_parser("summary", help="查看 token 汇总")
    add_range_arguments(summary_parser)
    summary_parser.add_argument("--user", default="local", help="CLI 用户名，默认 local")
    summary_parser.set_defaults(handler=cmd_summary)

    export_parser = subparsers.add_parser("export", help="导出 CSV 文件")
    add_range_arguments(export_parser)
    export_parser.add_argument("--output", default="token_usage.csv", help="目标 CSV 文件，默认 token_usage.csv")
    export_parser.add_argument("--user", default="local", help="CLI 用户名，默认 local")
    export_parser.set_defaults(handler=cmd_export)

    backup_parser = subparsers.add_parser("backup", help="创建并校验 SQLite 本地备份")
    backup_parser.add_argument(
        "--output-dir",
        default=None,
        help="备份目录，默认数据库旁的 backups/",
    )
    backup_parser.set_defaults(handler=cmd_backup)

    verify_backup_parser = subparsers.add_parser("verify-backup", help="只读验证 SQLite 备份")
    verify_backup_parser.add_argument("--path", required=True, help="待验证的 SQLite 备份路径")
    verify_backup_parser.set_defaults(handler=cmd_verify_backup)

    restore_backup_parser = subparsers.add_parser(
        "restore-backup",
        help="将已验证备份恢复到明确的 staging/目标路径",
    )
    restore_backup_parser.add_argument("--path", required=True, help="待恢复的 SQLite 备份路径")
    restore_backup_parser.add_argument("--target", required=True, help="明确的恢复目标 SQLite 路径")
    restore_backup_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="允许原子替换已存在的目标文件；默认拒绝覆盖",
    )
    restore_backup_parser.set_defaults(handler=cmd_restore_backup)

    preflight_parser = subparsers.add_parser("preflight", help="只读检查共享/生产部署配置")
    preflight_parser.add_argument("--production", action="store_true", help="按 HTTPS 生产门禁检查")
    preflight_parser.set_defaults(handler=cmd_preflight)

    audit_parser = subparsers.add_parser("audit", help="只读检查当前 checkout 的发布就绪状态")
    audit_parser.add_argument("--json", action="store_true", help="以 JSON 输出稳定的检查结果")
    audit_parser.add_argument("--strict", action="store_true", help="将未具备的外部工具链门禁视为失败")
    audit_parser.set_defaults(handler=cmd_audit)

    serve_parser = subparsers.add_parser("serve", help="启动 Web 仪表盘")
    serve_parser.add_argument("--host", default=None, help="监听地址，默认 TOKEN_TRACKER_HOST 或 127.0.0.1")
    serve_parser.add_argument("--port", type=int, default=None, help="监听端口，默认 TOKEN_TRACKER_PORT 或 5000")
    serve_mode = serve_parser.add_mutually_exclusive_group()
    serve_mode.add_argument("--debug", action="store_true", help="开启 Flask 调试模式，仅建议本机开发使用")
    serve_mode.add_argument("--production", action="store_true", help="HTTPS 生产模式：Waitress + 严格 HTTPS 预检")
    serve_mode.add_argument("--lan-preview", action="store_true", help="可信局域网 HTTP 预览：Waitress + 显式 LAN 模式")
    serve_parser.set_defaults(handler=cmd_serve)

    admin_parser = subparsers.add_parser("admin", help="管理本地账户角色（首次部署使用）")
    admin_subparsers = admin_parser.add_subparsers(dest="admin_command", required=True)
    role_parser = admin_subparsers.add_parser("set-role", help="将指定账户设为 user 或 admin")
    role_parser.add_argument("--username", required=True, help="已注册账户名")
    role_parser.add_argument("--role", choices=("user", "admin"), default="admin", help="目标角色，默认 admin")
    role_parser.set_defaults(handler=cmd_admin_set_role)

    return parser


def add_range_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--period",
        choices=("day", "week", "month", "all"),
        default="day",
        help="统计周期，默认 day",
    )
    parser.add_argument("--from", dest="date_from", help="起始日期，格式 YYYY-MM-DD，包含当天")
    parser.add_argument("--to", dest="date_to", help="结束日期，格式 YYYY-MM-DD，包含当天")


def get_user(args: argparse.Namespace, database: Path) -> dict:
    db.init_db(database)
    return db.get_or_create_cli_user(args.user, database)


def cmd_init(args: argparse.Namespace) -> int:
    database = db.init_db(args.db)
    print(f"数据库已初始化：{database}")
    return 0


def cmd_add(args: argparse.Namespace) -> int:
    database = db.get_db_path(args.db)
    user = get_user(args, database)
    try:
        record = add_usage(
            user_id=user["id"],
            model=args.model,
            input_tokens=args.input_tokens,
            output_tokens=args.output_tokens,
            timestamp=args.timestamp,
            note=args.note,
            path=str(database),
        )
    except UsageValidationError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2
    print(
        f"已添加 #{record['id']}：{record['model']}，"
        f"输入 {record['input_tokens']} + 输出 {record['output_tokens']} = "
        f"{record['total_tokens']} tokens（{record['timestamp']}）"
    )
    return 0


def format_number(value: int) -> str:
    return f"{int(value):,}"


def print_table(rows: Iterable[dict], columns: list[tuple[str, str]]) -> None:
    materialized = [[str(row[key]) for key, _ in columns] for row in rows]
    headers = [title for _, title in columns]
    widths = [len(header) for header in headers]
    for row in materialized:
        for index, value in enumerate(row):
            widths[index] = max(widths[index], len(value))
    print("  ".join(header.ljust(widths[index]) for index, header in enumerate(headers)))
    print("  ".join("-" * width for width in widths))
    for row in materialized:
        print("  ".join(value.ljust(widths[index]) for index, value in enumerate(row)))


def cmd_summary(args: argparse.Namespace) -> int:
    database = db.get_db_path(args.db)
    user = get_user(args, database)
    try:
        start, end = query_range(args.period, args.date_from, args.date_to)
    except UsageValidationError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2

    totals = db.summary_totals(user["id"], start, end, database)
    rows = db.summary_by_model(user["id"], start, end, database)
    range_label = f"{args.date_from or '最早'} 至 {args.date_to or '今天'}" if (args.date_from or args.date_to) else args.period
    print(f"用户：{user['username']}    范围：{range_label}")
    print(
        f"合计调用 {format_number(totals['calls'])} 次，"
        f"输入 {format_number(totals['input_tokens'])}，"
        f"输出 {format_number(totals['output_tokens'])}，"
        f"总 token {format_number(totals['total_tokens'])}"
    )
    if not rows:
        print("暂无记录。")
        return 0
    print()
    print_table(
        [
            {
                **row,
                "calls": format_number(row["calls"]),
                "input_tokens": format_number(row["input_tokens"]),
                "output_tokens": format_number(row["output_tokens"]),
                "total_tokens": format_number(row["total_tokens"]),
            }
            for row in rows
        ],
        [
            ("model", "模型"),
            ("calls", "调用次数"),
            ("input_tokens", "输入"),
            ("output_tokens", "输出"),
            ("total_tokens", "总 token"),
        ],
    )
    return 0


def cmd_export(args: argparse.Namespace) -> int:
    database = db.get_db_path(args.db)
    user = get_user(args, database)
    try:
        start, end = query_range(args.period, args.date_from, args.date_to)
    except UsageValidationError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2
    output_path = Path(args.output).expanduser().resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        payload = db.export_csv(user["id"], start, end, database)
    except csv_export.ExportTooLargeError:
        print("错误：导出结果超过安全边界，请缩小范围或分批导出", file=sys.stderr)
        return 2
    output_path.write_bytes(payload)
    print(f"已导出 {output_path}")
    return 0


def cmd_backup(args: argparse.Namespace) -> int:
    """Create a verified backup without exposing database contents to stdout."""

    try:
        target = backup.create_backup(db.get_db_path(args.db), args.output_dir)
    except backup.BackupError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2
    print(f"已创建并校验备份：{target}")
    return 0


def cmd_verify_backup(args: argparse.Namespace) -> int:
    """Verify a backup without opening the configured database for writing."""

    try:
        result = backup.verify_backup(args.path)
    except backup.BackupError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2
    print(f"备份验证通过：{result['path']}")
    print(f"完整性：{result['integrity_check']}；外键错误：{result['foreign_key_errors']}")
    return 0


def cmd_restore_backup(args: argparse.Namespace) -> int:
    """Restore only to an explicit target after read-only source verification."""

    try:
        result = backup.restore_backup(args.path, args.target, overwrite=args.overwrite)
    except backup.BackupError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2
    print(f"备份恢复并校验通过：{result['path']}")
    print(f"完整性：{result['integrity_check']}；覆盖已有目标：{result['overwritten']}")
    return 0


def cmd_preflight(args: argparse.Namespace) -> int:
    """Run read-only deployment checks before a shared service is started."""

    from .settings import build_settings

    if args.production:
        os.environ["TOKEN_TRACKER_RUNTIME_MODE"] = "production"
    try:
        database = db.get_db_path(args.db, ensure_parent=False)
        settings = build_settings(database)
        deployment_checks.assert_valid(settings, require_https=args.production)
    except (deployment_checks.DeploymentCheckError, RuntimeError, ValueError) as exc:
        print(f"部署预检失败：{exc}", file=sys.stderr)
        return 2
    mode = settings.get("RUNTIME_MODE", "local")
    print(f"部署预检通过：mode={mode}；HTTPS 门禁={'on' if args.production else 'off'}")
    return 0


def cmd_audit(args: argparse.Namespace) -> int:
    """Print release evidence without touching the configured database."""

    checks = release_audit.run_audit()
    print(release_audit.format_report(checks, as_json=args.json))
    return release_audit.exit_code(checks, strict=args.strict)


def cmd_serve(args: argparse.Namespace) -> int:
    from .web import create_app
    from .settings import build_settings

    database = db.get_db_path(args.db)
    if args.production or args.lan_preview:
        # Shared modes must not silently fall back to an ephemeral session key;
        # local mode keeps the zero-config personal experience.
        os.environ["TOKEN_TRACKER_RUNTIME_MODE"] = "production" if args.production else "lan"
        try:
            settings = build_settings(database)
            deployment_checks.assert_valid(settings, require_https=args.production)
        except (deployment_checks.DeploymentCheckError, RuntimeError, ValueError) as exc:
            mode_label = "生产" if args.production else "LAN 预览"
            print(f"{mode_label}服务启动被拒绝：{exc}", file=sys.stderr)
            return 2
    app = create_app(database)
    host = args.host or app.config.get("TOKEN_TRACKER_HOST", "127.0.0.1")
    port = args.port or int(app.config.get("TOKEN_TRACKER_PORT", 5000))
    print(f"Web 仪表盘：http://{host}:{port}")
    if args.production or args.lan_preview:
        from waitress import serve

        serve(app, host=host, port=port)
        return 0
    app.run(host=host, port=port, debug=args.debug)
    return 0


def cmd_admin_set_role(args: argparse.Namespace) -> int:
    """Bootstrap or adjust a role without exposing a web-side privilege API."""

    database = db.get_db_path(args.db)
    db.init_db(database)
    user = db.set_user_role(args.username.strip(), args.role, database)
    if user is None:
        print(f"错误：账户不存在：{args.username}", file=sys.stderr)
        return 2
    events.insert_audit_event(
        actor_user_id=None,
        target_user_id=user["id"],
        action="cli.user.role.set",
        resource_type="user",
        resource_id=str(user["id"]),
        request_id=f"cli-{secrets.token_hex(4)}",
        path=str(database),
        metadata={"role": user["role"]},
    )
    print(f"已更新角色：{user['username']} -> {user['role']}")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.handler(args)
    except KeyboardInterrupt:
        print("\n已退出。")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
