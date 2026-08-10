# ADR-042：中心服务区分存活与 SQLite 就绪

## 状态

已接受（2026-08-10）。

## 运维问题

1. HTTP 进程是否仍然存活并能接收请求？
2. 中心 SQLite 核心 schema 是否完整且可读？
3. 反向代理或进程管理器能否判断失败，而不接触数据库路径、表名、账户或秘密？

## 决策

- `/api/v1/health` 保持轻量 liveness，只返回进程协议状态，不读取业务表。
- 新增匿名 `/api/v1/ready` readiness，检查核心表集合是否可读；成功返回 `status=ready`，
  失败返回 `503 SERVICE_NOT_READY` 和统一错误 envelope。
- readiness 只暴露稳定状态码、错误码和 request id；实际请求仍进入既有白名单访问日志，
  由 request id 关联耗时和状态，不记录路径参数、数据库异常或表名。
- 当前本地优先部署不引入额外 Metrics/Tracing SDK；健康探针和现有脱敏 RED 访问日志足以
  支撑当前单机服务，未来迁移 Go/Vue 或 PostgreSQL 时再按部署平台增加指标适配器。

## 取舍

- ready 检查不能证明 provider、证书、磁盘容量或外部网络可用；这些仍由部署预检和上线检查负责。
- 核心表集合在 schema 迁移时必须同步更新，否则服务会主动保持未就绪，避免部分 schema 接收业务流量。

## 验证

- 初始化临时空 schema 后 `/api/v1/health` 与 `/api/v1/ready` 均返回成功状态。
- 未初始化 SQLite 路径的 readiness 返回 `False`，不输出 SQLite 异常细节。
- Python 编译、统一错误边界、行数门禁和脱敏访问日志规则继续通过；不读取真实数据库内容。
