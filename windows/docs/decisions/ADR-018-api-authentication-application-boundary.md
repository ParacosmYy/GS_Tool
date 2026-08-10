# ADR-018: 将 v1 凭据验证收敛到认证应用边界

## Status

Superseded by ADR-024

## Date

2026-08-10

## Context

`api_v1.py` 原先同时负责 JSON 解析、查找用户和调用 Werkzeug 密码哈希校验。这样会
让 HTTP 控制器知道 `password_hash` 持久化字段，后续增加 Web、CLI 或其他客户端时容易
复制认证语义，也增加敏感字段穿过控制器的机会。

## Decision

最初在 `mobile_auth.py` 增加 `authenticate_user(username, password, path)` 应用函数；
该职责已由 ADR-024 进一步拆分到 `auth_service.py`：

- 统一用户名长度和密码类型边界；
- 在认证模块内读取用户和校验哈希；
- 失败只返回 `None`，不区分用户不存在和密码错误；
- 控制器只负责限流、调用和映射 `INVALID_CREDENTIALS`；
- 成功后仍由原有 `issue_token_pair` 生成短期 access token 和轮换 refresh token。

## Consequences

- `/api/v1` 控制器不再依赖 `db` 或密码哈希库，认证策略集中且可替换。
- Web 表单登录仍是兼容入口，后续认证切片应复用同一应用能力并统一注册策略。
- 认证失败响应和令牌字段保持 API 契约兼容。

## Verification

- 静态检索确认 `api_v1.py` 不直接引用数据库或密码哈希。
- Python compileall、health/login page smoke check 和文件规模扫描通过。
