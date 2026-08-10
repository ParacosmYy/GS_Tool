# ADR-073：管理员成员详情展示脱敏诊断日志

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted；2026-08-10  
**范围：** Windows Web Admin 成员详情读模型

## 背景

管理员 API 的成员活动读模型已经返回 `app_logs`，但 Admin 页面只渲染 token records 和 work events，
导致管理员只能看到日志总数和 CSV 导出入口，无法在选择成员后定位一条具体的脱敏诊断信息。

## 决策

1. 在既有 `admin-detail` 中增加全宽 Diagnostic logs 表格，稳定的 `tbody#admin-log-rows` 展示时间、等级、事件类型、错误码、脱敏消息和 request id。
2. 前端继续消费 `/api/v1/admin/users/<id>/records` 返回的 `logs` 数组；不新增路由、不复制 SQL、不改变管理员 RBAC 或审计语义。
3. 所有单元格通过 `textContent` 创建；消息列允许自然换行，表格仅在自身容器内横向滚动，避免长日志破坏页面布局。
4. 空日志时渲染明确的空状态；管理员导出仍使用既有 `kind=logs` 固定列 CSV，不把页面展示当作导出授权。

## 后果

- 管理员可以在同一成员详情内完成 token、工作事件和诊断日志的关联观察。
- 该页面只展示服务端已经脱敏和限长的字段，不引入原始 prompt、Provider Key 或未经投影的上游响应。
- 真实生产环境仍需按部署负责人策略验证日志保留周期、访问审计和导出权限。

## 回滚

删除 Admin 详情中的 Diagnostic logs 区块、renderer 分支和对应样式即可回滚；API `logs` 读模型和 CSV
导出保持不变，避免破坏 Android 或其他管理工具。
