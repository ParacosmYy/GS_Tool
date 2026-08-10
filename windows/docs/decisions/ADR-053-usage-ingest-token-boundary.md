# ADR-053：外部用量采集 Token 边界

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted  
**日期：** 2026-08-10

## 背景

网页代理可以从自己发出的 OpenAI-compatible 请求响应中读取真实 `usage`，但网页无法观察
Kimi Code、OpenAI SDK 或其他独立进程的网络调用。仅要求同学在网页填写 URL 和 Key，不会让
中心服务自动获得另一个进程的 token 统计；也不能为了“自动”而记录原始 prompt、抓取任意网络
流量或把 provider Key 集中明文保存。

## 决策

增加独立的 per-user Usage Ingest Token 与固定写入接口：

1. CLI 为已有账户创建有期限的 `ait_...` 令牌，支持列出元数据和按账户撤销。原文只在创建命令
   的 stdout 显示一次；SQLite `usage_ingest_tokens` 只保存 SHA-256 digest、用户归属、标签、
   创建/到期/撤销时间。
2. 外部 wrapper、SDK adapter 或中心 Gateway 使用 `X-AI-Tracker-Ingest-Token` 调用
   `POST /api/v1/ingest/usage`。接口只接受模型、输入 token、输出 token、本地时间和有界备注；
   不接受 `source`、密码、provider Key、原始 prompt、完整 provider 响应或任意用户 ID。
3. 服务端从令牌摘要解析用户，并固定写入 `usage_records.source = ingest`。服务端忽略调用者会话
   的用户身份，避免浏览器 Cookie 与外部令牌交叉授权。
4. 外部请求使用用户级 `Idempotency-Key`；重复键返回第一次记录而不重复计数。接口按用户限流，
   无效 token 返回统一 `INGEST_TOKEN_INVALID`，令牌验证不回显存在性差异。
5. 该接口使用 HTTPS 部署要求，但不承担 provider 代理职责。provider Key 仍由合作的 wrapper/
   Gateway 按自身安全策略处理；中心服务不因拥有 ingest token 而获得 provider 调用权。

## 取舍

- 优点：为 Kimi Code 等外部客户端提供明确、可撤销、可审计的自动入账入口；不会把浏览器 CSRF
  会话和机器采集凭据混在一起；幂等键可以安全处理网络重试。
- 限制：现有 stock 客户端不会自动调用该接口。要实现“调用即入账”，还需要后续独立的 SDK
  wrapper 或 OpenAI-compatible Gateway；没有 usage 的 provider 仍不能被系统猜测。
- 兼容：现有 `/api/v1/records`、网页代理和 Android bearer 契约不改变；新增接口字段均为独立
  路由和独立来源值。

## 安全不变量

- ingest token 不写入日志、审计元数据、CSV、响应体或普通客户端存储。
- 所有 token 查询必须限定 digest、未撤销和未过期条件；撤销只允许令牌所属账户执行 CLI 操作。
- token 只能触发 usage 记录写入，不能读取汇总、成员明细、日志或 provider 配置。
- `note` 只用于短标签/上下文，禁止 wrapper 将原始 prompt 或完整响应放入备注。
- 采集客户端必须通过 HTTPS 传输；正式部署仍由 Caddy/Waitress 和 deployment preflight 强制。

## 后续工作

实现 Kimi Code wrapper 或中心 Gateway 时，必须新增 provider 路由/适配 ADR，定义非流式与流式
`usage` 语义、provider Key 的内存/加密存储策略、失败重试和部署边界；不得把 wrapper 逻辑塞入
`web.py` 或绕过本 ADR 的 token 生命周期。
