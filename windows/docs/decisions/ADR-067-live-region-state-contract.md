# ADR-067：动态状态与错误播报契约

**作者：** AI Token Tracker Engineering Team  
**状态：** accepted  
**日期：** 2026-08-10  
**范围：** Windows Web UI / Dashboard / Admin

## 背景

Dashboard、自动采集和管理员明细都包含异步状态。此前模板虽然声明了
`role="status"`，但运行时错误仍沿用 polite live region；颜色可以提示鼠标用户，
却不能稳定通知屏幕阅读器。各页面若分别修改 ARIA 属性，也容易产生语义漂移。

## 决策

1. 新增 `static/modules/live-region.js`，集中维护动态播报语义。
2. 普通加载、成功和空状态使用 `role="status"`、`aria-live="polite"`、
   `aria-atomic="true"`。
3. 错误状态使用 `role="alert"`、`aria-live="assertive"`、
   `aria-atomic="true"`，并保留可见错误样式；颜色不是唯一状态信号。
4. Provider 状态保留装饰性状态点，但由同一模块切换播报语义；业务模块不直接
   复制 ARIA 属性组合。
5. 该契约只影响浏览器语义，不改变 API、数据库、请求重试或 Provider Key 生命周期。

## 验证与边界

- 隔离 SQLite、`127.0.0.1:5019` 浏览器回归：Dashboard 无 Key 检测错误为
  `alert/assertive`，切换到 Week 后恢复为 `status/polite`。
- Admin 明细加载为 `status/polite`，焦点仍进入关闭按钮；浏览器 error/warning 为空。
- `node --check`、Python `compileall` 和 `token_tracker audit --json` 通过。
- 该证据不替代真实设备、Provider 真实 Key、导出下载和完整四档 UI-3 验收。
