# ADR-054：本地 OpenAI-compatible Usage Gateway

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted  
**日期：** 2026-08-10

## 背景

`POST /api/v1/ingest/usage` 已经提供了安全的中心入账入口，但 Kimi Code、OpenAI-compatible
CLI 或 IDE 只会把请求发送到它配置的 provider Base URL，不会主动调用 Tracker。要实现
“外部客户端调用后自动记账”，需要一个兼容客户端协议的本地适配器。

Kimi Code 官方文档说明其 API 提供 OpenAI-compatible Base URL
`https://api.kimi.com/coding/v1`，第三方工具可以覆盖 Base URL；Kimi 的流式协议以 SSE
传输，并在完成 chunk 提供 usage 时才能得到准确输入/输出 token。实现依据：

- [Kimi Code API access](https://www.kimi.com/code/docs/en/)
- [Kimi Code provider configuration](https://www.kimi.com/code/docs/en/kimi-code-cli/configuration/providers.html)
- [Kimi streaming output](https://platform.kimi.com/docs/guide/utilize-the-streaming-output-feature-of-kimi-api)

## 决策

增加独立的本地 `gateway.py` 进程，不把 Gateway 路由塞入中心 `web.py`：

1. Gateway 暴露 OpenAI-compatible `GET /v1/models`、`POST /v1/chat/completions`，并保留无版本
   别名方便不同客户端；支持非流式 JSON 和流式 SSE。默认只绑定 `127.0.0.1`/`::1`。
2. Gateway 启动时从指定环境变量读取 provider Key 和中心 Usage Ingest Token，放入进程内存；
   不写 SQLite、`.env`、普通日志、请求响应或命令行参数。非 loopback 绑定必须额外配置独立
   `Authorization: Bearer <gateway-access-token>`，不能把 provider Key 当 Gateway 身份凭据。
3. 客户端请求体仍由 provider adapter 的字段 allowlist、消息数量、请求体大小、HTTPS 上游
   allowlist、超时和响应上限约束；Gateway 仅把 provider Key 注入上游请求，不信任客户端传来的
   `base_url` 或 `api_key`。
4. Gateway 从非流式响应或 SSE 完成 chunk 提取真实 usage。输入或输出缺失时不猜测、不上报；
   上报使用中心 `X-AI-Tracker-Ingest-Token` 和请求级 `Idempotency-Key`。中心上报失败不篡改
   已成功的 provider 响应，响应通过 `X-AI-Tracker-Usage` 暴露 `recorded`、`missing` 或
   `report-failed` 状态。
5. Gateway 仅支持 OpenAI Chat Completions 协议；Anthropic Messages、OpenAI Responses 和
   供应商私有工具协议必须新增 adapter，不得在 Gateway 路由里猜测转换。

## 取舍

- 优点：同学只需把 Kimi Code/OpenAI-compatible 客户端的 Base URL 改为本机 Gateway，真实
  provider Key 不离开本机；中心网站只接收用量事实。
- 限制：每台使用外部客户端的电脑都要运行一个 Gateway；中心服务不能自动观察未经过 Gateway
  的既有进程流量。首版上报失败不持久化队列，后续可靠性切片必须增加加密/脱敏的本地重试边界。
- 兼容：网页自己的代理仍使用原有 `/api/proxy/chat/completions`；Android 继续使用 bearer
  provider API；Gateway 是新增本地进程，不改变现有 API v1 成功字段。

## 安全不变量

- provider Key 仅存在于 Gateway 进程内存，永不作为中心请求字段、URL、日志或命令参数传播。
- Gateway 默认 loopback；显式 LAN/公网绑定没有访问令牌时启动失败。
- 上游 URL 和中心 ingest URL 启动时校验 scheme、凭据、查询参数和资源路径；禁止重定向绕过。
- SSE 解析只保留 bounded buffer 和 usage 摘要，响应大小超过上限即终止；不把 prompt 写入备注。
- `stream=true` 时尽可能请求 `stream_options.include_usage=true`；没有最终 usage 就明确标记
  `missing`，不按字符或本地 tokenizer 估算。

## 后续工作

1. 增加本地加密重试队列，使用同一幂等键恢复中心短时不可用的上报。
2. 根据真实 Kimi Code CLI、Claude Code、OpenCode 的请求形状补齐兼容性矩阵；不伪造未验证的
   tool/Responses/Anthropic 能力。
3. 使用用户本人合法 Key 完成一次非流式和一次流式真实上游联调，并保存脱敏证据；没有授权 Key
   时只能验证配置、协议解析和失败边界。
