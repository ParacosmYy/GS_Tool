# ADR-017: 集中管理可替换限流状态

## Status

Accepted

## Date

2026-08-10

## Context

网页控制器和 `/api/v1` 控制器都需要保护登录、模型发现和 provider proxy，旧实现分别
维护锁、时间窗口和 bucket。两份实现的算法虽然相同，但会让策略修复、生产替换和运行
观测出现漂移；多进程部署时两份状态也都不能提供全局保护。

## Decision

将滑动窗口的策略和存储适配器收敛到 `token_tracker/rate_limit.py`，控制器只调用一个稳定的
`allow(bucket, identity, limit, window_seconds)` 接口。local 保持内存实现；shared、production
和 lan 使用 SQLite 共享表，确保多个 Waitress worker 看到同一限流窗口。
更高并发部署仍可在不改变控制器接口的前提下替换为 PostgreSQL/Redis 或入口层策略。

## Alternatives considered

### 保留控制器内重复实现

改动最小，但会持续产生策略漂移和重复锁；拒绝。

### 现在直接引入 Redis

会增加外部服务、部署和数据一致性成本，当前小规模 Windows 中心服务先使用 SQLite 共享表；高并发迁移时再替换适配器，暂不引入。

### 在每个请求里实例化 limiter

会丢失窗口状态，实际无法限流；拒绝。

## Consequences

- 登录、provider 和未来 API 限流共享同一实现和替换边界。
- shared/production/lan 模式的当前限流窗口跨 Waitress worker 共享，但 SQLite 写锁仍不适合无限扩大规模。
- 以后接入 Redis、网关或 PostgreSQL 时只替换基础设施模块并补充部署 ADR。

## Verification

- 两个控制器不再声明独立的锁和 bucket；应用组合根按 runtime mode 配置内存或 SQLite 适配器。
- Python 编译、schema smoke、health/login smoke check 和文件规模扫描通过。
- 隔离 SQLite 并发冒烟使用 8 个线程请求同一窗口，严格得到 3 个允许、5 个拒绝；数据库只保存哈希 key，未保存原始身份。
- 该冒烟只证明当前小规模 SQLite 适配器的事务边界，不替代真实生产负载测试；更高并发仍需 PostgreSQL/Redis 或入口层方案。
