# ADR-035：SQLite 备份与 staging 文件的仓库隔离边界

## 状态

已接受（2026-08-10）。

## 背景

应用主数据库位于 `windows/data/token_tracker.sqlite3`，备份命令默认写入
`windows/data/backups/`，恢复演练通常使用 `windows/data/staging/`。这些文件包含账户、
token 用量、工作事件、诊断日志和管理员审计，不能因为路径从 data 根目录移动到子目录就失去
版本库排除保护。

## 决策

根 `.gitignore` 同时覆盖：

- `windows/data/` 下任意层级的 `*.sqlite3` 和 `*.sqlite3-*`；
- `windows/data/backups/`；
- `windows/data/staging/`；
- 已有的主数据库、`.env`、Caddyfile 和边缘日志目录规则继续保留。

忽略规则只阻止后续误提交，不负责清理已经被 Git 跟踪的敏感文件；发布前仍必须审查索引和
历史，若发现真实数据已提交，应按密钥/数据泄露流程处理。

## 验证

- 当前工作区的数据文件位于 `windows/data/token_tracker.sqlite3`，命中主数据库规则。
- 规则文本覆盖递归 SQLite、backups 和 staging 路径；未修改任何数据库内容。
