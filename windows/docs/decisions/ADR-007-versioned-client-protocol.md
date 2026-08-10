# ADR-007：统一 `/api/v1` 与事件命令信封

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted  
**日期：** 2026-08-10

## 背景

网页、Android 和未来的桌面采集器需要使用同一个中心 Windows 服务。嵌入式系统常使用版本化命令来解决设备端升级和兼容问题；本项目也需要可演进的通讯边界，但当前数据量、部署方式和客户端类型并不需要裸 TCP 或二进制命令总线。

## 决策

1. 正式 HTTP API 使用路径版本：`/api/v1`。
2. 登录、汇总、事件和日志使用 REST 资源；不把所有业务包装成一个巨大 `command` 路由。
3. 事件批量同步允许使用统一信封：

   ```json
   {
     "protocol_version": 1,
     "command": "work_event.create",
     "request_id": "client-generated-uuid",
     "idempotency_key": "client-generated-unique-key",
     "payload": {}
   }
   ```

4. `protocol_version` 是整数，当前只接受 `1`；`command` 是有限白名单，不允许客户端传入 SQL、路由名或任意函数名。
5. 服务端以 `request_id` 关联日志，以 `idempotency_key` 防止 Android 离线重试造成重复事件。幂等记录按用户隔离。
6. v0 只用于短期开发实验或迁移适配，不写入新客户端默认配置；任何破坏性变更创建 v2，并保留 v1 迁移窗口。

## 取舍

- 采用 HTTP/JSON 能复用现有 Flask、浏览器和 Android 网络栈，便于审计、抓包和分享部署。
- 版本化路径保证破坏性变更可并行发布；事件信封只解决离线同步和可观测性，不增加普通查询的包装层。
- 若未来需要高频实时 telemetry，再单独评估 WebSocket/消息队列；当前不提前引入复杂基础设施。

## 约束

- 协议版本不是认证；认证仍由浏览器安全会话或 Android 的短期 bearer token 完成。
- 版本信封不允许包含 API Key、密码、原始 prompt 或未经脱敏的 provider 响应。
- 所有跨端字段必须先更新 `windows/docs/api-contract.md`，再更新网页与 Android。

