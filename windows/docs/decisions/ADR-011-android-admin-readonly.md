# ADR-011：Android 管理员只读观测边界

- **状态：** Accepted
- **日期：** 2026-08-10
- **作者：** AI Token Tracker Engineering Team
- **范围：** `android/` 管理员界面、`/api/v1/admin/*`

## 背景

Windows 中心服务已经提供管理员总览、成员聚合、成员活动和审计导出。Android 与网页使用同一账号时，管理员需要在移动端快速查看团队状态；普通同学仍只能看到自己的数据。

## 决策

1. Android 登录后根据服务返回的 `user.role` 选择个人仪表盘或管理员只读观测台；这只是 UI 路由提示，所有 `/api/v1/admin/*` 请求仍由 Windows 服务端 RBAC 强制授权。
2. Android 首期只读取 `/admin/overview`、`/admin/users` 和 `/admin/users/<id>/records`，成员列表使用 `limit/offset`，成员明细使用服务端安全上限，不在客户端实现跨用户 SQL 或本地聚合。
3. 管理员界面展示团队 token 汇总、模型趋势、成员聚合和选中成员的脱敏 usage/events/logs；不展示密码哈希、令牌、provider API Key、原始 prompt 或完整 provider 响应。
4. 访问成员明细和团队总览继续由服务端写入 `audit_events`；Android 不额外生成不可验证的审计记录。
5. Android 暂不实现管理员 CSV 下载，避免在移动端引入文件权限、分享和数据残留问题；网页/Windows 继续作为安全导出入口。

## 结果

- 同一个管理员账号可在 Windows 和 Android 查看一致的团队只读数据。
- 普通账号的 ViewModel 不调用管理员接口，且服务端即使收到越权请求也返回 `ADMIN_REQUIRED`。
- 成员明细采用按需加载，避免登录时对每个同学发起 N+1 请求。
- APK 编译和设备联调仍需要用户批准并安装 Android 工具链；源码阶段只做契约、静态和 Windows 运行验证。
