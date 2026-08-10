# ADR-025: 增加只读备份验证命令

## Status

Accepted

## Date

2026-08-10

## Context

`backup` 命令使用 SQLite online backup API 创建副本并在发布前执行 integrity check，
但运维人员没有一个独立命令验证已有副本。直接覆盖生产数据库做恢复演练具有数据损坏
风险，也不符合本项目的非破坏性默认策略。

## Decision

- 新增 `python -m token_tracker verify-backup --path <backup.sqlite3>`；
- 命令以 SQLite read-only URI 打开文件，执行 `integrity_check`、`foreign_key_check` 和
  必要表检查（包括 `usage_ingest_tokens` 当前生产表）；
- 只输出路径和验证摘要，不输出账户、日志、token 或密钥内容；
- 命令不修改备份、不修改当前数据库、不删除旧文件；
- 真正的恢复演练仍需管理员提供明确 staging 目标，并单独记录结果。

## Consequences

- 管理员可在发布前和定期保留策略中验证备份可读性。
- 只读验证不能证明目标机器权限、磁盘空间或完整业务恢复时间；这些属于部署演练门禁。

## Verification

- 当前数据库或已存在副本可通过只读验证；缺文件、损坏或缺表返回非零退出码。
- CLI help、Python compile 和文件规模检查通过。
