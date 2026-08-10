# ADR-021: 管理员读模型与审计编排进入 Application 层

## Status

Accepted

## Date

2026-08-10

## Context

管理员 API 控制器原先直接组合 `admin_data` 查询、导出类型校验和
`events.insert_audit_event`。Web 管理员页面没有单独记录页面访问。这样会让审计动作
容易在新增入口时遗漏，也让控制器承担业务编排而不是稳定 HTTP 映射。

## Decision

新增 `admin_service.py`：

- 统一管理员页面、overview、成员列表、成员明细和 CSV 导出用例；
- 在成功的管理员读取/导出动作上写入审计事件；不存在的成员不记录“成功查看”；
- 固定导出类型白名单和错误语义；
- `admin_data.py` 只负责脱敏读模型与固定列数据投影；
- Web/API 控制器只负责认证装饰、请求参数和 HTTP 响应。

## Consequences

- 新增管理员客户端可以复用相同的审计和数据边界。
- 页面加载会产生页面访问审计，随后 overview/users API 仍各自产生细粒度审计，便于追踪。
- 审计写入仍与 SQLite 事务绑定；未来接入异步日志或外部审计仓库需要新增可靠投递策略。

## Verification

- 管理员 API 控制器不再直接调用 admin_data 或审计写入。
- `/admin` 页面访问经过 Application 审计用例。
- Python 编译、错误路由、health/login smoke check 和文件规模扫描通过。
