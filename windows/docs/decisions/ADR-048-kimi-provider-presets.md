# ADR-048：Kimi provider 预设与 OpenAI-compatible 适配边界

## 状态

已接受（2026-08-10）。

## 背景

用户希望在网页中输入自己的 Key 和 URL 后自动发现模型并记录 usage。Kimi 开放平台与 Kimi Code
都提供 OpenAI-compatible 调用形状，但它们使用不同的 Base URL、Key 体系和产品权限。把两者
混成一个“万能 Kimi Key”会让失败语义和安全说明不准确。

## 决策

- Web 自动采集增加只负责填充 URL 的快速预设：Kimi Code 使用
  `https://api.kimi.com/coding/v1`，Kimi 开放平台使用 `https://api.moonshot.cn/v1`，并保留
  OpenAI 与自定义 OpenAI-compatible 选项。
- 预设不绕过服务端 `TOKEN_TRACKER_ALLOWED_BASE_URLS` 白名单，不携带 Key，不写入数据库；用户仍可
  编辑 URL，最终请求必须经过既有 URL、HTTPS、超时、响应大小和 Key 内存边界。
- 本阶段不新增 Kimi 专用网络分支：官方 OpenAI-compatible 协议可复用现有 adapter。只有未来出现
  专用 header、Anthropic-only capability 或不同 usage 结构时，才新增独立 adapter，并保持 provider
  registry 接口不变。
- Kimi Code 的真实账户权限、模型可用性和产品条款由 Kimi 控制台决定；系统只记录上游成功响应中
  明确返回的 usage，不猜测配额或计费量。

## 官方依据

- [Kimi Code API Access](https://www.kimi.com/code/docs/en/)：OpenAI-compatible Base URL、模型 ID
  和与 Kimi 开放平台的区别。
- [Kimi Code Error Reference](https://www.kimi.com/code/docs/en/kimi-code/error-reference.html)：
  Kimi Code 与 Open Platform 的 Key/Base URL 不可互换。
- [Kimi Chat API](https://platform.kimi.com/docs/api/chat)：非流式 `usage.prompt_tokens` /
  `completion_tokens` 响应结构。

## 验证边界

- 已完成：官方 URL/协议核对、UI 预设源码、白名单示例、文档和行数门禁同步。
- 待完成：使用用户本人合法 Key 的真实 `/models` 与非流式 chat smoke；本地环境不读取或生成
  用户 Key，也不伪造上游响应。
