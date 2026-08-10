# 中心化多用户平台方案

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** 设计基线，按垂直切片实现

## 目标

Windows 端作为中心服务主机，所有同学通过网页或 Android 客户端登录同一个服务。服务主机上的数据库保存用户、token 用量、AI 使用方向、效率事件、正确/错误结果和可导出的审计日志。

```text
网页端 ───────┐
Android APK ──┼── HTTPS /api/v1 ── Windows 中心服务 ── SQLite（小规模）
CLI ──────────┘                         │
                                       ├─ usage_records
                                       ├─ work_events
                                       ├─ app_logs
                                       └─ audit_events
```

## 账号与权限

- `user`：只能读取和写入自己的 usage、工作事件和客户端日志。
- `admin`：可以查看按用户汇总、授权范围内的明细和导出；不能看到密码、API Key 或 refresh token。
- `admin` 的查看、导出、角色变更必须写入 `audit_events`，并包含操作者、目标用户、动作、时间和结果。
- 管理员权限不能由客户端字段决定，必须由服务端会话/访问令牌对应的数据库角色决定。

## 数据模型边界

`usage_records` 继续只保存 token 事实：模型、输入/输出、时间、来源和备注。新增信息使用独立事件表，避免把不同口径混在一个表中：

| 表 | 记录内容 | 默认可见范围 |
|---|---|---|
| `users` | 账号、密码哈希、角色、创建时间 | 本人；管理员只看非敏感账号字段 |
| `usage_records` | 输入/输出/总 token、模型、时间、来源 | 本人；管理员按权限查看 |
| `work_events` | 任务方向、开始/结束、产出、效率指标 | 本人；管理员按团队政策查看 |
| `app_logs` | 客户端/服务端日志、级别、错误码、请求关联 ID | 本人相关；管理员按权限查看 |
| `audit_events` | 管理员和安全相关操作 | 管理员；不可由普通用户删除 |

“正确码”不定义为猜测的业务结论，而是结构化结果：`outcome=success|failure|partial`、`error_code`、`http_status`、`request_id` 和可选安全摘要。原始 prompt、API Key、密码、refresh token 不进入普通日志和导出文件。

## 部署与分享

本机体验仍监听 `127.0.0.1`。给同学使用时，不能直接把 Flask 开发服务器暴露到公网；应采用以下任一安全入口：

1. Windows 主机 + HTTPS 反向代理 + Waitress，适合可信局域网或固定域名。
2. Windows 主机 + Tailscale/ZeroTier 私网，适合小范围同学协作。
3. 公网部署到 VPS/托管服务，使用持久化磁盘或 PostgreSQL，Windows 电脑只作为开发/备份端。

SQLite 适合当前小规模团队和单机中心；如果并发写入、历史日志或备份要求增长，保留 API 契约后迁移 PostgreSQL，不让客户端感知数据库变化。

## 隐私与治理

集中保存同学数据前，需要让参与者明确知道记录范围、管理员可见范围、保存期限和删除/导出规则。管理员面板默认先显示汇总，明细和原始日志使用二次确认并留下审计事件。

