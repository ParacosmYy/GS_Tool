# ADR-001: 使用 Flask + SQLite 的本地优先架构

## Status

Accepted

## Date

2026-08-10

## Context

项目需要记录个人和小范围共享的 AI token 用量。核心数据是用户、模型、输入/输出 token 和本地时间，数据量预计较小；用户希望双击即可体验，同时可以在局域网或 HTTPS 反向代理后共享给同学。

## Decision

使用 Python Flask 提供 CLI 复用的 Web API，使用 Python 标准库 `sqlite3` 保存本地数据，使用原生 HTML/CSS/JavaScript 和 Chart.js 完成页面。

## Alternatives Considered

### Node.js + better-sqlite3

- 优点：前后端语言统一。
- 拒绝原因：用户偏好 Python；当前数据层不需要 Node 的额外运行时复杂度。

### PostgreSQL + 独立前端

- 优点：更适合高并发和多实例部署。
- 拒绝原因：个人/同学规模会引入数据库服务、跨域会话和部署成本；当前架构已经保留 API 边界，未来可迁移。

## Consequences

- 个人体验无需云服务和数据库账号。
- SQLite 写并发不适合大规模公开服务；正式扩大规模前要迁移到服务型数据库并增加共享限流。
- 同源 API 让 CSRF、会话和前端启动更简单。
