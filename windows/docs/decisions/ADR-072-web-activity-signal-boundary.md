# ADR-072：Web 个人工作信号沿用 v1 Application 边界

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted；2026-08-10  
**范围：** Windows Web Dashboard 的个人工作方向、结果、效率、正确/错误码和结构化备注

## 背景

项目已经具备 `work_events` 数据模型、`events.py` 校验用例和
`POST/GET /api/v1/events/work` 跨端契约，但个人 Web Dashboard 之前只有管理员读模型入口。
用户需要在网页中记录 AI 辅助工作的方向、效率和结果，同时保持 Android 与 Web 的账号、限流、
幂等和隐私规则一致。

## 决策

1. Dashboard 新增一个独立的 Activity 竖切片，使用 `static/modules/activity.js` 管理表单、状态和最近列表。
2. 浏览器通过既有 same-origin API client 调用 `/api/v1/events/work`；CSRF、session、账户限流、
   `work_event.create` 命令信封、字段白名单、脱敏和用户隔离继续由 API/Application 层负责。
3. 页面只提交方向、结果、0-100 效率评分、项目、任务类型、正确码、错误码和必要备注；不接收原始
   prompt、Provider Key、完整模型响应或任意 HTML。
4. 最近列表使用 `textContent` 写入表格单元格，只展示当前账号的分页结果；后续管理员查看仍走现有 RBAC
   读模型和审计出口，不在 Web 模块复制 SQL。
5. 视觉层将输入卡片和历史表格作为一个响应式双栏区域；`900px` 以下堆叠，历史表格保留横向滚动，
   保证窄屏不压缩字段、不产生页面级横向溢出。

## 后果

- Web、Android 和未来 Gateway 共享同一工作事件契约，避免客户端各自定义“效率/正确码”语义。
- 个人用户可以从网页完成结构化工作记录，管理员可以继续通过既有活动读模型汇总团队数据。
- 当前记录仍是显式用户动作，不代表自动读取其他客户端的 prompt 或日志；自动采集仍需要 Gateway、
  Usage Ingest 或客户端适配器提供受控事件。
- UI-3 v19 需要在隔离合法会话中验证成功写入、列表刷新、错误播报、窄屏表格和控制台无异常；真实设备、
  高对比度/reduced-motion 和正式部署证据仍由最终门禁负责。

## 回滚

删除 Dashboard Activity 区块、导航锚点和 `activity.js` 即可回滚 Web 写入入口；不删除 API、数据库表、
Android 接入或管理员读模型，避免破坏已有跨端契约。
