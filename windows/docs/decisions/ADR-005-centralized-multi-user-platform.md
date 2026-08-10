# ADR-005：由 Windows 端承载中心化多用户数据

## Status

Accepted for the first team deployment; PostgreSQL migration remains an evolution path.

## Context

网页和 Android 端需要使用同一个账号，并由管理员查看团队用量、效率事件和审计日志。数据不能分散在每个同学的浏览器或手机上。

## Decision

`windows/` 运行唯一的业务服务和数据库。网页与 Android 只调用版本化 HTTP API；客户端不直接读取 SQLite。当前小规模使用 SQLite，所有记录带 `user_id`，所有管理员读取通过服务端 RBAC 和审计事件控制。

## Alternatives

- 每个客户端本地 SQLite：无法形成统一团队视图，也难以审计。
- 直接共享 SQLite 文件：会造成锁、权限和数据损坏风险。
- 立即改 Go + PostgreSQL：长期可行，但会把中心化、管理员和移动端三个风险同时叠加到一次重写中。

## Consequences

- Windows 主机必须持续在线，分享时必须使用 HTTPS 和可靠入口。
- SQLite 备份和恢复必须成为交付项；团队规模增长时迁移 PostgreSQL。
- 管理员权限、隐私告知、数据保留和审计不能省略。

