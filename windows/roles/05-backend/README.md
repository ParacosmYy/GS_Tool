# 后端工程师交付包

**作者：** AI Token Tracker Engineering Team  
**职责：** 提供稳定、可验证、安全的自动采集 API 和本地数据持久化。

## 输入

- `docs/api-contract.md`
- `docs/architecture.md`
- `roles/06-architect/`

## 输出

- `token_tracker/web.py` 路由、会话和错误响应；
- `token_tracker/services.py` 用例与领域校验；
- `token_tracker/db.py` 参数化 SQL、聚合和 CSV；
- `token_tracker/providers.py` allowlist、模型发现、请求和 usage adapter。

## 验收

- 每个受保护端点授权且按用户隔离；
- 外部响应先验证形状；
- Key 不落盘、不写日志、不回传；
- SSRF、重定向、超时、请求大小和限流边界明确。
