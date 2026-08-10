# ADR-077：备份清单与保留策略只读门禁

**作者：** AI Token Tracker Engineering Team  
**维护者：** DEV-5 / ARCH-2  
**状态：** Accepted  
**日期：** 2026-08-10  
**作用：** 固化本地备份盘点、保留策略检查和完整性验证的安全边界。

## 背景

项目已有 `backup`、`verify-backup` 和 `restore-backup`，但管理员仍需要手工浏览目录判断备份是否
存在、是否过旧或占用过多磁盘空间。R-12 需要一个可重复的运维入口，同时不能因为“自动保留策略”
而引入误删生产备份的风险。

## 决策

1. 新增 `backup-inventory` CLI。它只扫描明确目录的直接子项，接受名称为
   `token_tracker-*.sqlite3` 的常规文件；不递归、不创建目录、不读取配置数据库业务表、不改名、
   不上传、不删除文件。
2. 扫描上限固定为 `MAX_INVENTORY_FILES = 512`。软链接、目录、临时文件和其他名称不进入清单，
   超过上限直接 fail closed，避免目录异常导致无界资源消耗。
3. `--min-count`、`--max-age-days` 和 `--max-size-mib` 均为显式策略输入；策略未满足时输出
   `attention` 并返回退出码 2。年龄按本地文件修改时间与本地当前时间的精确秒数判断，展示值只保留
   两位小数。
4. `--verify` 复用既有 `verify_backup` 的只读 integrity、foreign-key 和必要 schema 检查。文本
   输出服务于人工操作，`--json` 输出稳定的 `policy`、`summary`、`violations` 和 `files` 投影，
   不包含 token、密码、provider Key、prompt 或 SQLite 业务行。
5. 保留周期、离线副本、异地副本和责任人由正式部署负责人确认；本项目不提供默认自动删除机制。
6. `release-doctor.ps1` 仅在显式传入 `-CheckBackups` 时调用该清单命令；可附带
   `-BackupDirectory`、`-MinBackupCount`、`-MaxBackupAgeDays`、`-MaxBackupSizeMiB` 和
   `-VerifyBackups`。未开启时不改变既有 Local/LAN/Production 预检行为，开启后清单的非零退出码
   映射为发布检查失败。

## 后果

- 备份状态可以接入 PowerShell、CI 或发布前检查，并且缺失/过旧/超容量/损坏均有稳定机器码。
- 发布预检可以通过显式开关复用同一套清单策略，不会把“新安装尚未创建备份”误判为默认启动故障。
- 新部署尚未创建备份时，清单会明确报告 `BACKUP_DIRECTORY_MISSING` 或 `NO_BACKUPS`，不会悄悄创建空目录。
- 磁盘清理仍是人工确认的独立运维动作；如果未来新增删除命令，必须另立 ADR、二次确认和回滚证据。

## 验证边界

- `backup-inventory --help`、Python 编译、缺失目录 JSON 输出和“不创建目录”行为已在隔离路径检查。
- `release-doctor.ps1` 的备份检查保持显式开启；默认路径不执行清单，策略参数和 `--verify` 只透传到既有
  只读 CLI。
- 后续应在不触碰真实 `data/` 的前提下补充满足策略、策略失败和 `--verify` 的部署演练；真实数据恢复、
  保留周期和责任人记录仍属于 R-12 正式环境门禁。

## 回滚

移除 `release-doctor.ps1` 的 `-CheckBackups` 分支即可回滚发布编排；删除 `backup-inventory` 的 CLI
注册和 `inventory_backups` 公开入口也不会影响既有创建、验证和恢复命令，不需要数据库迁移。
