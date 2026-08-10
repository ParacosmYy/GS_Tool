# ADR-099：Kimi 官方 Endpoint 上下文与 Key 隔离

**作者：** AI Token Tracker Engineering Team  
**维护者：** BE / ARCH-2  
**状态：** Accepted  
**日期：** 2026-08-10  
**关联：** ADR-048、ADR-054

## 背景

复核 Kimi 官方文档时发现两个产品上下文的 Base URL 不能简单合并：Kimi 开放平台 API
概览/快速开始继续示例 `https://api.moonshot.cn/v1`；Kimi Code CLI 的 provider 配置页
另列 `https://api.moonshot.ai/v1` 作为 Moonshot API provider 默认地址；Kimi Code 自身的
OpenAI-compatible coding API 仍是 `https://api.kimi.com/coding/v1`。不同地址对应的账户体系、
认证方式和产品权限不能凭域名相似而互换。

## 决策

1. Web 预设继续保持明确分离：Kimi Code 使用 `https://api.kimi.com/coding/v1`，Kimi 开放平台
   使用 `https://api.moonshot.cn/v1`；不把 `.ai` 替换成默认的开放平台预设。
2. `.env.example` 和生产 allowlist 示例额外允许 `https://api.moonshot.ai/v1`，仅作为用户根据
   Kimi Code CLI provider 文档和自身账户选择的可选地址；UI 不把它伪装成新的产品预设。
3. 任何 Key 只与其官方账户/Base URL 配对；系统仍通过 allowlist、HTTPS、超时、重定向和 usage
   校验边界处理请求，不根据域名猜测账户归属或计费口径。

## 官方依据

- [Kimi Code providers and models](https://www.kimi.com/code/docs/en/kimi-code-cli/configuration/providers)：
  记录 Kimi Code CLI 的 provider 类型与 Moonshot API provider 配置。
- [Kimi Code overview](https://www.kimi.com/code/docs/en/)：记录 Kimi Code 的
  `https://api.kimi.com/coding/v1` OpenAI-compatible 接口。
- [Kimi API overview](https://platform.kimi.com/docs/api/overview) 与
  [Kimi API quickstart](https://platform.kimi.com/docs/api/quickstart)：记录 Kimi 开放平台的
  `https://api.moonshot.cn/v1` SDK/HTTP Base URL。

## 验证与边界

- 已更新项目 allowlist 示例和文档说明；未读取、生成或发送任何用户 Key。
- 真实 `/models`、非流式/流式 usage 和账户权限仍需用户本人合法 Key 联调；本 ADR 不将官方
  文档地址核对描述为真实 Provider 成功。

## 回滚

删除 ADR-099、移除 `.ai` allowlist 示例并恢复原文档即可回滚；不改变已保存的用户 `.env`、
数据库、认证会话或运行中的服务。
