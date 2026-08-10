# ADR-023: Provider 输入字段预算

## Status

Accepted

## Date

2026-08-10

## Context

provider 代理已经限制总请求体、上游响应、超时和 URL allowlist，但 API Key、模型名、
Base URL 和消息数组没有独立预算。攻击者或误配置客户端可以把单个字段推到总上限，
增加解析、日志排障和上游转发成本。

## Decision

在 provider adapter 边界统一执行：

- API Key 最多 4096 字符；
- Base URL 最多 2048 字符；
- 模型名最多 200 字符；
- 消息数组最多 100 项；
- 继续保留 256 KB HTTP 请求体和 2 MB 上游响应上限。

超限使用现有 `ProviderInputError`/`PROVIDER_INPUT_INVALID` 语义，不把 Key 或原始内容
写入错误详情。常见 OpenAI-compatible/Kimi 请求远低于这些上限，不改变成功响应字段。

## Consequences

- provider 资源预算在字段边界可审查、可观测，未来专用 adapter 可以复用同一安全预算。
- 更严格的输入校验可能拒绝异常大的历史消息；调用方应在客户端分页或截断上下文。

## Verification

- 正常 provider 边界函数继续接受有效请求。
- 超长 Key、Base URL、模型和消息数组在发送上游前被拒绝。
- Python 编译、HTTP smoke check、JavaScript 语法和文件规模扫描通过。
