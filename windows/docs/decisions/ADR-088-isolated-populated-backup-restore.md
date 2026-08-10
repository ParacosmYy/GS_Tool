# ADR-088：已填充隔离数据库的备份恢复证据

**作者：** AI Token Tracker Engineering Team
**维护者：** ARCH-2 / DEV-5
**状态：** Accepted
**日期：** 2026-08-10
**范围：** 记录真实填充过的隔离 SQLite 数据库备份、校验和 staging 恢复，不替代生产数据演练。

## 背景

备份服务已经具备 SQLite online backup、完整性检查、外键校验和默认拒绝覆盖的实现，之前的证据主要覆盖
空 schema 或源码边界。需要用真实写入过的记录走一遍完整链路，确认数据内容能够从运行时数据库进入备份并恢复到明确 staging 目标。

## 决策

使用 ADR-087 生成的隔离 EXE 数据库作为输入。所有备份和恢复文件都放在被忽略的
`windows/.cache/backup-restore-v12-*` 下，不读取或覆盖 `windows/data/`、用户正式 `%LOCALAPPDATA%` 或受保护服务数据库。
生产部署仍必须由负责人提供保留周期、离线副本、恢复责任人和真实数据恢复记录。

## 运行证据

在 2026-08-10 当前 checkout 执行以下等价命令：

```powershell
python -m token_tracker --db <isolated-db> backup --output-dir <isolated-backup-dir>
python -m token_tracker verify-backup --path <backup.sqlite3>
python -m token_tracker --db <isolated-db> backup-inventory --output-dir <isolated-backup-dir> --verify --json
python -m token_tracker restore-backup --path <backup.sqlite3> --target <staging\token_tracker.sqlite3>
python -m token_tracker verify-backup --path <staging\token_tracker.sqlite3>
```

- 备份文件大小：`110592` bytes，创建并校验成功。
- `verify-backup`：`integrity=ok`，外键错误 `0`。
- `backup-inventory --verify`：数量 `1`、完整性失败 `0`、状态 `pass`。
- 恢复目标位于隔离 `staging\token_tracker.sqlite3`，默认未覆盖已有文件，恢复后再次验证通过。
- 输入数据库含有 ADR-087 记录的 `kimi-code` `321 + 654 = 975` 用量，未将账户或令牌写入本 ADR。

## 未完成门禁

真实中心数据库恢复、离线副本、保留周期、限流压测、日志样本和负责人签署仍属于 R-12 正式部署门禁。

## 回滚

本 ADR 只新增隔离证据，不改变运行时代码、schema 或用户数据。若演练目录需要清理，必须由部署负责人确认其位于
`windows/.cache/` 且没有进程使用；不得通过 Git 或脚本删除真实数据库与备份。
