# ADR-003: 在当前页面会话内复用 Provider Key

## Status

Accepted

## Date

2026-08-10

## Author

AI Token Tracker Engineering Team

## Context

自动采集主流程需要连续发送多次请求。每次都重新粘贴 Key 会破坏“一次连接、连续追踪”的操作体验，但把 Key 写入 SQLite、Cookie、localStorage 或服务端配置会扩大共享部署的泄露半径。

## Decision

检测模型或提交调用后，前端允许在当前页面 JavaScript 内存中复用 Key。页面刷新、关闭或显式点击“清除 Key”时清除；Key 不进入 localStorage、session cookie、SQLite、CSV、日志或响应。后端仍把每个请求当作一次独立的临时凭据，不建立用户 Key 配置表。

## Alternatives Considered

### 保存到 localStorage

- 优点：刷新后无需再次输入。
- 拒绝原因：浏览器持久存储的读取半径更大，且不适合共享电脑或公共演示。

### 加密保存到服务器

- 优点：多设备复用。
- 拒绝原因：需要密钥托管、轮换、撤销、审计和租户隔离，超出当前本地优先产品的最小安全边界。

## Consequences

- 页面会话中可连续自动记账，改善核心交互。
- 刷新页面后用户需要重新输入 Key，这是有意的安全权衡。
- 若未来实现外部客户端 gateway，必须遵守 [ADR-004](ADR-004-external-gateway-boundary.md) 单独设计 per-user ingest token 和 provider key 隔离，不得复用当前页面内存方案。
