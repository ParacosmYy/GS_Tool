# ADR-014：为移动端 token 写入提供幂等保护

**状态：** Accepted  
**日期：** 2026-08-10  
**作者：** AI Token Tracker Engineering Team  
**范围：** `/api/v1/records`、Android Repository、SQLite `usage_records`

## 背景

移动网络可能在服务端提交成功后丢失响应。客户端若再次提交同一操作，普通
`INSERT` 会把一次调用错误计算成两次。Android 的 bearer 自动刷新也可能在
边界情况下重放业务操作，因此 token 写入必须具备用户级幂等语义。

## 决策

- Android 为一次写入生成 UUID，并通过 `Idempotency-Key` 请求头发送。
- SQLite 在 `usage_records` 增加可空 `idempotency_key`，并建立
  `(user_id, idempotency_key)` 的部分唯一索引，保留历史记录兼容性。
- 服务端先查找同用户同 key 的记录；并发竞争由唯一索引兜底，命中时返回原记录。
- 新写入返回 `201`，重放返回 `200` 和 `replayed: true`。
- 幂等键只用于基础设施去重，不进入网页、Android 或管理员记录投影。

## 影响与边界

该机制只保证同一用户、同一 key 的一次写入语义；客户端为不同业务操作生成
不同 key。它不替代认证、传输加密、服务端校验或离线队列。旧网页会话接口
仍可继续写入无 key 的记录，并保持原有行为。
