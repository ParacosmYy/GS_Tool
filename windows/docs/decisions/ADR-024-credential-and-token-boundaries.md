# ADR-024: 分离凭据用例与 Bearer Token 生命周期

## Status

Accepted; supersedes the module placement described by ADR-018.

## Date

2026-08-10

## Context

Web 注册/登录和 Android v1 登录需要同一套用户名/密码规则，但 `web.py`、`api_v1.py`
和 `mobile_auth.py` 原本分别知道表字段、密码哈希或凭据校验。凭据校验与 bearer token
签发/轮换/撤销属于不同安全生命周期，混合会让改动和审计边界变得含糊。

## Decision

- `auth_service.py` 负责用户名/密码边界、scrypt 哈希、注册冲突和凭据验证；
- `mobile_auth.py` 只负责 access/refresh digest、过期、轮换和撤销；
- Web 和 `/api/v1/auth/login` 复用 `auth_service.authenticate_user`；
- 控制器永远不读取 `password_hash`，认证失败使用通用结果；
- 原有 token 字段、TTL 和轮换行为不变。

## Consequences

- 密码规则修改只需要更新一个应用模块，网页和 Android 不会漂移。
- token 生命周期仍可独立替换为更强的 session/token provider。
- `mobile_auth.py` 只依赖数据库 token 操作；`auth_service.py` 不生成 bearer secret。

## Verification

- 静态检索确认 Web/API 控制器不再导入密码哈希库。
- 正常登录/注册边界、无效凭据和健康页通过项目虚拟环境 smoke check。
- Python 编译、JavaScript 语法和文件规模扫描通过。
