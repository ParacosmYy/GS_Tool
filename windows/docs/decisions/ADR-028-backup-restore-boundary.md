# ADR-028：SQLite 备份恢复的显式目标与原子发布边界

**状态：** Accepted  
**日期：** 2026-08-10  
**范围：** `token_tracker/backup.py`、CLI `restore-backup`、Windows 部署 runbook

## 背景

备份创建和只读验证已经能够证明 SQLite 文件完整，但正式部署还需要可执行的恢复路径。恢复操作涉及账户、令牌摘要和日志，不能因为命令参数输入错误而直接覆盖当前数据库，也不能把未验证的备份发布为可运行数据源。

## 决策

1. 新增 `restore-backup --path <backup> --target <database>`，恢复目标必须由操作者显式提供；不支持隐式替换配置中的当前数据库，且服务层拒绝把数据库写入网站 `static` 目录。
2. 默认拒绝覆盖已有目标。只有显式传入 `--overwrite` 才允许在验证成功后原子替换目标。
3. 恢复顺序固定为：源文件只读 integrity/foreign-key/required-schema 校验（包括共享限流状态表） → sibling partial 文件写入 → partial 再次验证 → `os.replace` 原子发布。
4. 任一步失败都清理 partial 文件；如果清理失败，错误信息只报告路径和操作状态，不输出数据库内容、密码哈希、令牌或 provider Key。
5. CLI 只负责参数和安全错误映射，恢复实现留在 Infrastructure `backup.py`；部署文档要求在隔离 staging 目标完成演练后才允许切换生产数据库。

6. SQLite source/target connection 使用显式 `close()` 语义；Python 的 SQLite connection
   context manager 只负责事务，不足以释放 Windows 文件句柄，不能直接依赖 `with
   sqlite3.connect(...)` 完成资源生命周期。

## 取舍与未执行项

- 当前不会自动复制工作区现有数据库，也不会替用户选择 staging 位置；真实恢复演练需要项目负责人明确目标磁盘、保留周期和停机窗口。
- 该命令不执行云端同步、备份删除或跨主机传输，避免把本地敏感数据扩散到未授权位置。

## 当前验证

- 使用隔离临时目录创建空 schema 备份，完成只读验证、显式 staging 目标恢复和恢复后完整性/外键/必要表校验。
- 演练未读取、覆盖或导出工作区现有数据库；正式数据恢复仍需由负责人提供目标、停机窗口和回滚记录。
