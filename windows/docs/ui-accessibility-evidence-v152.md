# UI Accessibility Evidence v152

日期：2026-08-12

## 变更范围

本轮聚焦 Dashboard `05-work-signal / 06-activity-trace` 的空态层级。此前工作
信号表单高度约 838px，而右侧空历史卡片只有约 332px，桌面端形成明显的下方
空洞；移动端空态则像一行普通表格。

现在新增 `token_tracker/static/activity-history-signal.css`，由 `base.html`
统一加载，并只作用于 `.activity-history-card.is-empty`：

- 宽屏让历史卡片跟随同一网格轨道，与工作信号表单共享高度和起始基线。
- 空态状态胶囊、标题底轨、观测网格和中心 marker 组成完整的等待首条事件状态面。
- ≤620px 保持紧凑空态窗口，不把手机页面拉成长屏。
- populated event rows 不改动，仍保留原有表格和滚动契约。

本轮不改变活动事件 API、请求 payload、数据库、认证、导航或业务数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5026`，使用隔离
演示数据库登录。现有 5000/5011 保护服务未重启、未停止、未修改。

| 入口 / 视口 | 场景 | 活动历史表现 | 横向边界 | 页面日志 |
| --- | --- | --- | --- | --- |
| 5026 / 1440×900 | `ACTIVITY` 深链 | 空历史卡片与工作信号表单同高 `838.3px`，从同一网格基线开始 | 活动区内部溢出 `0` | `[]` |
| 5026 / 390×844 | `ACTIVITY` 深链 | 空态窗口 `156px`，历史卡片 `370.3px`，不拉伸整页 | 活动区内部溢出 `0` | `[]` |

桌面空态 marker、状态胶囊和观测网格可见；移动端 `ACTIVITY` 路由上下文与导航
`aria-current="Activity"` 正常。活动区可见交互控件保持原有焦点顺序与 `tabIndex=0`。

## 可访问性与降级

- 空态文案仍保留在原有 `td.table-empty--signal` 中，屏幕阅读器可以读取完整提示。
- `prefers-reduced-motion: reduce` 下 beacon 动画关闭，内容和状态文本不变。
- `forced-colors: active` 下状态胶囊、分隔线、marker 和网格交给 `CanvasText` /
  `Canvas` 系统色，不依赖固定品牌色。
- `activity-history-signal.css` 约 150 行，所有修改文本源码低于 1000 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
