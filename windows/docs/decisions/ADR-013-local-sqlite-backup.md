# ADR-013：提供本地 SQLite 在线备份

**状态：** Accepted  
**日期：** 2026-08-10  
**作者：** AI Token Tracker Engineering Team  
**范围：** `windows/` 中心服务、CLI、SQLite 数据文件

## 背景

项目支持 Windows 电脑作为同学网页和 Android 客户端的中心数据宿主。正式分享前，管理员需要一个不依赖云服务、不会把半成品数据库暴露出去的备份入口。直接复制 SQLite 文件无法可靠处理正在写入的数据库，也没有完整性证明。

## 决策

增加 `token_tracker backup` 命令：

- 默认读取配置的 SQLite 文件，并在数据库旁创建 `backups/` 目录。
- 以只读连接打开源数据库，调用 SQLite online backup API 生成副本。
- 备份完成后执行 `PRAGMA integrity_check`。
- 先写隐藏的 `.partial` 临时文件，再通过同目录原子替换发布最终文件。
- 支持 `--output-dir` 覆盖备份目录，但不会自动上传、删除旧备份或导出 API Key。

## 影响与边界

备份包含账户、token 记录、工作事件、日志和审计数据，必须按敏感数据保护。管理员负责保留周期、访问权限、异机保存和恢复演练。该命令是本地备份能力，不等同于经过验证的灾备系统；正式公网部署仍需要 HTTPS、限流、脱敏日志和恢复流程。

## 验收证据

- CLI 帮助应列出 `backup` 子命令。
- Python 编译检查应通过。
- 运行服务仍应通过 `/api/health`、`/api/v1/health` 和安全响应头检查。
- 未经用户明确授权，不复制真实数据库到异机或云端。
