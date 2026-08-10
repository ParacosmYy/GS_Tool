# ADR-012：版本化客户端 token 记录写入

- **状态：** Accepted
- **日期：** 2026-08-10
- **作者：** AI Token Tracker Engineering Team
- **范围：** `windows/token_tracker/api_v1.py`、Android token 记录入口

## 背景

网页早期使用会话接口 `POST /api/records` 写入用量，但 Android 只持有
`/api/v1` bearer 会话。若移动端复用网页路径，版本边界、认证方式和后续
协议演进会混在一起，无法形成稳定的跨端契约。

## 决策

1. 增加 `GET/POST /api/v1/records`：由 bearer 账号分页读取或写入自己的手动
   token 记录；旧的 `POST /api/records` 继续兼容网页，不改变现有页面行为。
2. 服务端继续使用统一的 `add_usage` 校验模型、输入/输出 token、时间和备注，
   不在 Android 重复实现数据库规则。
3. `source` 由服务端固定为 `android`，客户端不能通过请求体伪造来源或用户归属；
   所有数据仍由 `g.user` 对应账号隔离。
4. 接口返回 `record` 与 `request_id`，写入成功后 Android 重新读取 `/me/summary`，
   由服务端作为汇总单一事实源。
5. Android 写入必须携带用户级 `Idempotency-Key`。服务端以 SQLite 唯一索引保护
   同一账号的重放，重复提交返回原记录而不增加调用次数；幂等键不进入公开投影。

## 结果

- Windows 网页和 Android 使用各自清晰的认证边界，同时写入同一 SQLite 数据库。
- Android 可以完成“填写模型与 token → 写入 Windows → 刷新汇总”的第一条写入闭环。
- Android 在 access token 刷新或网络重试时不会因为同一操作重复计数。
- 不新增网络依赖；APK 编译和设备联调仍等待用户批准 Android 工具链。
