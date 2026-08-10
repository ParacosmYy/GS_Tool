# ADR-020: 统一 API 错误边界与请求关联 ID

## Status

Accepted

## Date

2026-08-10

## Context

业务路由已经使用统一 `error_response`，但未知 `/api/*` 路径和不支持的方法仍可能落到
Flask 默认 HTML 错误页；这会让网页、Android 和运维采集器难以稳定解析。请求 ID 也由
应用随机生成但没有统一校验入口，跨反向代理排查时缺少明确的关联规则。

## Decision

- 所有 `/api/*` 的 404、405、413、500 错误都返回现有 JSON envelope；HTML 页面继续保留
  面向浏览器的错误页。
- `request_ids.resolve` 接受最多 64 个 ASCII 安全字符作为关联 ID；非法、过长或缺失值
  由服务端生成 128-bit 随机 ID。
- 关联 ID 只用于排障和响应关联，不作为认证、授权、幂等或用户身份依据。
- 每个响应通过 `X-Request-ID` 暴露实际采用的 ID；JSON 错误同时在 `request_id` 字段返回。

## Consequences

- 客户端可以稳定按状态码和机器码处理错误，不再解析 HTML。
- 代理/网关可以透传合法的 trace ID，同时服务端拒绝控制字符和过长输入。
- 仍未引入完整分布式 tracing；未来接入 OpenTelemetry 时复用该边界并增加采样/导出策略。

## Verification

- `/api/missing` 和错误方法返回 JSON `NOT_FOUND` / `METHOD_NOT_ALLOWED` envelope。
- 合法和非法 `X-Request-ID` 都能得到受控响应头；Python 编译与文件规模检查通过。
