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

from . import backup, csv_export, db, deployment_checks, events, gateway, gateway_queue, ingest_auth, release_audit
from .services import UsageValidationError, add_usage, query_range


def non_negative_int(value: str) -> int:
    """Parse a CLI integer that may be zero but never negative."""

    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("must be a non-negative integer") from exc
    if parsed < 0:
        raise argparse.ArgumentTypeError("must be a non-negative integer")
    return parsed


def positive_int(value: str) -> int:
    """Parse a CLI integer that must be greater than zero."""

    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("must be a positive integer") from exc
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be a positive integer")
    return parsed


def port_number(value: str) -> int:
    """Parse a TCP port within the IANA 1-65535 range."""

    parsed = positive_int(value)
    if parsed > 65535:
        raise argparse.ArgumentTypeError("port must be between 1 and 65535")
    return parsed


def build_parser() -> argparse.ArgumentParser:
    """Build the stable command tree without executing a command."""

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
    preflight_parser.add_argument(
        "--host",
        default=None,
        help="预检监听地址，默认 TOKEN_TRACKER_HOST 或 127.0.0.1；必须与 serve 的有效地址一致",
    )
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

    gateway_parser = subparsers.add_parser("gateway", help="启动本地 OpenAI-compatible 自动采集 Gateway")
    gateway_parser.add_argument("--upstream-url", required=True, help="固定的 provider Base URL，例如 Kimi Code /v1")
    gateway_parser.add_argument("--ingest-url", required=True, help="中心服务的 /api/v1/ingest/usage 地址")
    gateway_parser.add_argument("--host", default="127.0.0.1", help="监听地址，默认只绑定 127.0.0.1")
    gateway_parser.add_argument("--port", type=port_number, default=8787, help="监听端口，默认 8787")
    gateway_parser.add_argument(
        "--provider-key-env",
        default="TOKEN_TRACKER_GATEWAY_PROVIDER_KEY",
        help="provider Key 所在环境变量名，不会把 Key 放入参数或日志",
    )
    gateway_parser.add_argument(
        "--ingest-token-env",
        default="TOKEN_TRACKER_GATEWAY_INGEST_TOKEN",
        help="中心 Usage Ingest Token 所在环境变量名",
    )
    gateway_parser.add_argument(
        "--gateway-token-env",
        default="TOKEN_TRACKER_GATEWAY_ACCESS_TOKEN",
        help="非 loopback 监听时的独立 Gateway Bearer Token 环境变量名",
    )
    gateway_parser.add_argument("--allow-http", action="store_true", help="仅本地调试时允许 HTTP 上游/中心地址")
    gateway_parser.add_argument("--timeout", type=positive_int, default=120, help="上游请求超时秒数，默认 120")
    gateway_parser.add_argument("--report-timeout", type=positive_int, default=10, help="中心上报超时秒数，默认 10")
    gateway_parser.add_argument(
        "--queue-path",
        default=str(gateway_queue.default_queue_path()),
        help="DPAPI 加密重试队列路径，默认 data/gateway-usage-queue.sqlite3",
    )
    gateway_parser.add_argument(
        "--memory-only",
        action="store_true",
        help="显式关闭跨重启队列，仅适用于临时调试",
    )
    gateway_parser.add_argument("--debug", action="store_true", help="仅本机调试使用 Flask 开发服务器")
    gateway_parser.set_defaults(handler=cmd_gateway)

    admin_parser = subparsers.add_parser("admin", help="管理本地账户角色（首次部署使用）")
    admin_subparsers = admin_parser.add_subparsers(dest="admin_command", required=True)
    role_parser = admin_subparsers.add_parser("set-role", help="将指定账户设为 user 或 admin")
    role_parser.add_argument("--username", required=True, help="已注册账户名")
    role_parser.add_argument("--role", choices=("user", "admin"), default="admin", help="目标角色，默认 admin")
    role_parser.set_defaults(handler=cmd_admin_set_role)

    ingest_parser = subparsers.add_parser("ingest-token", help="管理外部客户端用量采集 token")
    ingest_subparsers = ingest_parser.add_subparsers(dest="ingest_command", required=True)
    ingest_create = ingest_subparsers.add_parser("create", help="创建一次性显示的采集 token")
    ingest_create.add_argument("--username", required=True, help="已注册账户名")
    ingest_create.add_argument("--label", default="external-client", help="客户端标签")
    ingest_create.add_argument("--expires-days", type=int, default=90, help="有效天数，1-3650，默认 90")
    ingest_create.set_defaults(handler=cmd_ingest_token)
    ingest_list = ingest_subparsers.add_parser("list", help="列出账户的采集 token 元数据")
    ingest_list.add_argument("--username", required=True, help="已注册账户名")
    ingest_list.set_defaults(handler=cmd_ingest_token)
    ingest_revoke = ingest_subparsers.add_parser("revoke", help="撤销一个采集 token")
    ingest_revoke.add_argument("--username", required=True, help="已注册账户名")
    ingest_revoke.add_argument("--id", type=int, required=True, dest="token_id", help="采集 token ID")
    ingest_revoke.set_defaults(handler=cmd_ingest_token)

    return parser


def add_range_arguments(parser: argparse.ArgumentParser) -> None:
    """Attach the shared local-time period and date-range options."""

    parser.add_argument(
        "--period",
        choices=("day", "week", "month", "all"),
        default="day",
        help="统计周期，默认 day",
    )
    parser.add_argument("--from", dest="date_from", help="起始日期，格式 YYYY-MM-DD，包含当天")
    parser.add_argument("--to", dest="date_to", help="结束日期，格式 YYYY-MM-DD，包含当天")


def get_user(args: argparse.Namespace, database: Path) -> dict:
    """Initialize the CLI database and resolve its bounded local user."""

    db.init_db(database)
    return db.get_or_create_cli_user(args.user, database)


def cmd_init(args: argparse.Namespace) -> int:
    """Create the configured SQLite schema and report its path."""

    database = db.init_db(args.db)
    print(f"数据库已初始化：{database}")
    return 0


def cmd_add(args: argparse.Namespace) -> int:
    """Validate and append one CLI usage record."""

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
    """Format a token count with stable thousands separators."""

    return f"{int(value):,}"


def print_table(rows: Iterable[dict], columns: list[tuple[str, str]]) -> None:
    """Render bounded tabular projections without a third-party dependency."""

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
    """Print model-level and total usage for one validated local range."""

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
    """Write one bounded, BOM-compatible usage CSV to an explicit path."""

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
        if args.host:
            settings["TOKEN_TRACKER_HOST"] = args.host
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
    """Start the selected local, LAN, or production web runtime."""

    from .web import create_app
    from .settings import build_settings

    database = db.get_db_path(args.db)
    effective_host = args.host or os.getenv("TOKEN_TRACKER_HOST", "127.0.0.1")
    if args.production or args.lan_preview:
        # Shared modes must not silently fall back to an ephemeral session key;
        # local mode keeps the zero-config personal experience.
        os.environ["TOKEN_TRACKER_RUNTIME_MODE"] = "production" if args.production else "lan"
        try:
            settings = build_settings(database)
            settings["TOKEN_TRACKER_HOST"] = effective_host
            deployment_checks.assert_valid(settings, require_https=args.production)
        except (deployment_checks.DeploymentCheckError, RuntimeError, ValueError) as exc:
            mode_label = "生产" if args.production else "LAN 预览"
            print(f"{mode_label}服务启动被拒绝：{exc}", file=sys.stderr)
            return 2
    app = create_app(database)
    host = effective_host or app.config.get("TOKEN_TRACKER_HOST", "127.0.0.1")
    port = args.port or int(app.config.get("TOKEN_TRACKER_PORT", 5000))
    print(f"Web 仪表盘：http://{host}:{port}")
    if args.production or args.lan_preview:
        from waitress import serve

        serve(app, host=host, port=port)
        return 0
    app.run(host=host, port=port, debug=args.debug)
    return 0


def cmd_gateway(args: argparse.Namespace) -> int:
    """Start a local provider gateway without persisting either credential."""

    try:
        from dotenv import load_dotenv

        load_dotenv()
        provider_key = gateway.read_environment_secret(
            args.provider_key_env,
            "provider_key",
            4096,
        )
        ingest_token = gateway.read_environment_secret(
            args.ingest_token_env,
            "ingest_token",
            256,
        )
        gateway_token = gateway.read_environment_secret(
            args.gateway_token_env,
            "gateway_token",
            256,
            required=False,
        )
        config = gateway.build_config(
            upstream_url=args.upstream_url,
            provider_key=provider_key,
            ingest_url=args.ingest_url,
            ingest_token=ingest_token,
            host=args.host,
            gateway_token=gateway_token,
            allow_http=args.allow_http,
            timeout=args.timeout,
            report_timeout=args.report_timeout,
            queue_path=None if args.memory_only else args.queue_path,
        )
    except gateway.GatewayConfigError as exc:
        print(f"Gateway 启动被拒绝：{exc}", file=sys.stderr)
        return 2
    try:
        app = gateway.create_gateway_app(config)
    except gateway.GatewayConfigError as exc:
        print(f"Gateway 启动被拒绝：{exc}", file=sys.stderr)
        return 2
    print(f"本地 Gateway：http://{config.host}:{args.port}/v1")
    print("provider Key 和 Usage Ingest Token 仅驻留在当前进程内存。")
    if config.queue_path:
        print(f"失败重试队列：Windows DPAPI 加密存储于 {config.queue_path}")
    else:
        print("警告：当前使用仅进程内存的失败重试队列，重启后未上报记录会丢失。", file=sys.stderr)
    if args.allow_http:
        print("警告：当前允许 HTTP，仅适用于本机调试。", file=sys.stderr)
    if args.debug:
        app.run(host=config.host, port=args.port, debug=True)
    else:
        from waitress import serve

        serve(app, host=config.host, port=args.port)
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


def cmd_ingest_token(args: argparse.Namespace) -> int:
    """Create, inspect, or revoke external ingest credentials locally."""

    database = db.get_db_path(args.db)
    db.init_db(database)
    user = db.find_user(args.username.strip(), database)
    if user is None:
        print(f"错误：账户不存在：{args.username}", file=sys.stderr)
        return 2

    if args.ingest_command == "create":
        try:
            result = ingest_auth.issue_token(
                user_id=user["id"],
                label=args.label,
                expires_days=args.expires_days,
                path=str(database),
            )
        except ValueError as exc:
            print(f"错误：{exc}", file=sys.stderr)
            return 2
        events.insert_audit_event(
            actor_user_id=None,
            target_user_id=user["id"],
            action="cli.ingest_token.create",
            resource_type="ingest_token",
            resource_id=str(result["id"]),
            request_id=f"cli-{secrets.token_hex(4)}",
            path=str(database),
            metadata={"label": result["label"], "expires_at": result["expires_at"]},
        )
        print(f"采集 token #{result['id']} 已创建：账户={user['username']}，标签={result['label']}，到期={result['expires_at']}")
        print(f"仅此一次显示 token：{result['token']}")
        print(f"请求头：{ingest_auth.INGEST_TOKEN_HEADER}: <token>")
        return 0

    if args.ingest_command == "list":
        rows = db.list_ingest_tokens(user["id"], database)
        if not rows:
            print("该账户暂无外部采集 token。")
            return 0
        print_table(
            [
                {
                    **row,
                    "status": "revoked" if row["revoked_at"] else "active",
                    "revoked_at": row["revoked_at"] or "-",
                }
                for row in rows
            ],
            [("id", "ID"), ("label", "标签"), ("created_at", "创建时间"), ("expires_at", "到期时间"), ("status", "状态"), ("revoked_at", "撤销时间")],
        )
        return 0

    if args.ingest_command == "revoke":
        if not db.revoke_ingest_token(args.token_id, user["id"], database):
            print(f"错误：找不到可撤销的采集 token #{args.token_id}", file=sys.stderr)
            return 2
        events.insert_audit_event(
            actor_user_id=None,
            target_user_id=user["id"],
            action="cli.ingest_token.revoke",
            resource_type="ingest_token",
            resource_id=str(args.token_id),
            request_id=f"cli-{secrets.token_hex(4)}",
            path=str(database),
        )
        print(f"已撤销采集 token #{args.token_id}（账户：{user['username']}）。")
        return 0

    print("错误：不支持的 ingest-token 操作", file=sys.stderr)
    return 2


def main(argv: list[str] | None = None) -> int:
    """Parse arguments, dispatch one command, and map Ctrl+C to a CLI exit."""

    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.handler(args)
    except KeyboardInterrupt:
        print("\n已退出。")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
