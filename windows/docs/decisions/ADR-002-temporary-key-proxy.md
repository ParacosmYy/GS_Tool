# ADR-002: 通过一次性代理请求自动记录 usage

## Status

Accepted

## Date

2026-08-10

## Context

不同 AI 服务的管理后台统计口径和接口不同，应用无法可靠地从一个统一接口拉取所有人的 token。很多 OpenAI-compatible 完成接口会在响应 `usage` 中返回本次输入/输出 token，因此可以在调用链路上记录实际结果。

## Decision

页面把用户填写的 Base URL、Key、模型和消息发给同源后端；后端完成白名单、HTTPS、超时、重定向和限流检查后代理一次非流式请求。只有上游明确返回输入/输出 usage 时才写入记录。Key 永不写入数据库、CSV、日志或浏览器持久化存储。

## Alternatives Considered

### 只做手动录入

- 优点：实现和安全面最小。
- 拒绝原因：无法满足用户“调用后自动获得真实用量”的主要体验。

### 把 Key 保存在用户配置表

- 优点：下次调用少填一次。
- 拒绝原因：共享部署中服务器会持有高价值凭据，泄露影响更大；当前产品优先保护 Key。

### 支持流式请求并从每个 chunk 拼 usage

- 优点：交互体验更接近聊天产品。
- 拒绝原因：不同 provider 的流式 usage 行为不一致，首版先保证统计准确；后续通过独立 adapter 增加。

## Consequences

- 用户每次自动代理调用都需要临时填 Key，但风险边界清晰。
- 没有 usage 的 provider 必须手动补录，系统不会猜测 token。
- 公网共享必须配置 `TOKEN_TRACKER_ALLOWED_BASE_URLS`，不能把任意 URL 代理变成 SSRF 入口。
