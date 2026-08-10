# ADR-004: 外部客户端自动采集的 Gateway 边界

## Status

Deferred

## Date

2026-08-10

## Author

AI Token Tracker Engineering Team

## Context

网页可以在用户输入 Base URL/Key 后，代理自己发出的 OpenAI-compatible 请求，并从真实响应的 `usage` 自动入账。但浏览器无法凭空观察 Kimi Code、OpenAI SDK 或其他独立进程的网络调用；仅在网页里填入 URL/Key，不会自动获得另一个客户端的调用记录。

## Decision

当前版本把“网页代理调用自动采集”作为完成边界，不把它描述成系统级流量监控。需要支持外部客户端时，单独实现 OpenAI-compatible Gateway，并在实现前固定以下契约：

- 客户端使用每用户、可撤销、只保存哈希的 ingest token 认证；禁止匿名转发；
- 每个请求必须能确定用户归属和 provider 路由；
- provider Key 要么由客户端按请求传递，要么使用明确的服务器端加密存储，不能直接明文落库；
- gateway 需要复用当前 HTTPS/allowlist/超时/禁重定向/响应上限/限流策略；
- 非流式和流式响应都要定义 usage 归档语义，缺少 usage 时不猜测；
- 新 API、数据库字段、密钥轮换和部署方式必须先新增契约与迁移说明。

## Consequences

- 当前个人体验入口简单、安全边界小，页面调用不需要服务器保存 provider Key。
- Kimi Code 等外部客户端要“全自动”进入账本，后续必须通过专用 gateway 或客户端 adapter；这是工程能力边界，不是 UI 缺陷。
- 在没有 per-user gateway 认证和密钥隔离设计前，不贸然把当前页面代理端点伪装成公共 OpenAI endpoint。
