---
name: project-ui-orchestration
description: Project-local orchestration rules for the AI Token Tracker five-module UI, readable Moonshot-inspired motion, enterprise comments, and browser evidence.
---

# AI Token Tracker UI 编排技能

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**作用：** 把通用前端技能落实为本项目可执行的视觉、动效、无障碍和代码结构门禁。

## 视觉方向

- 使用近黑画布、灰阶文字、单一 signal 高亮、超大排版、细线边界和克制留白。
- 参考 Moonshot 官网的空间感与运动逻辑，但不复制其商标、Logo、文案、图片或源码。
- 关键文字正文必须满足 WCAG AA：普通文字目标对比度至少 4.5:1，大号文字至少 3:1；不可只依赖颜色表达状态。
- 不使用紫色模板、满屏渐变、无意义圆角卡片、重阴影或为了“高级”而牺牲扫描效率的装饰。

## 动效协议

每个动效提交前必须回答：

1. 它表达什么信息或状态？
2. 触发方式、持续时间、缓动曲线和性能成本是什么？
3. `prefers-reduced-motion` 下如何变为静态反馈？
4. 触摸屏、键盘和低性能设备是否仍可用？

至少覆盖：页面入场、鼠标跟随空间反馈、中心 signal 轨道、滚动 reveal、数字变化、hover/focus、loading/success/error/empty。鼠标跟随必须使用 `pointer-events: none`，不得抢焦点或改变业务点击。

## 五模块交付

| 模块 | 必查内容 |
|---|---|
| `01-shell` | token、全局布局、字体、pointer aura、页面转场 |
| `02-auth` | 登录/注册错误、可读表单、焦点、入场动效 |
| `03-observatory` | 总量 signal、趋势/占比、loading/empty/error |
| `04-connect` | 检测/调用状态、Key 仅内存、成功失败反馈 |
| `05-history` | 表格/导出、移动端、键盘、ARIA、性能 |

## 企业级实现要求

- HTML/CSS/JS 文件顶部保留 `Author`、`Maintainer`、`Purpose` 和模块元信息。
- 公共函数、事件处理器、API client 和跨模块状态必须有 JSDoc；注释解释设计原因和禁止事项，不翻译下一行代码。
- `app.js` 不承载 API、图表和动效细节；新增能力拆成高内聚模块并通过显式 export/import 或稳定 DOM contract 连接。
- 每个文件最多 1000 行；接近 800 行立即按职责拆分。
- 不在前端保存密码、API Key、bearer token 或未经脱敏的上游响应。

## 验收证据

- 真实浏览器截图：登录、仪表盘、连接、历史、管理员页。
- DOM/ARIA：标题层级、表单 label、focus-visible、live region、空/错状态。
- 控制台无错误/无不必要警告；网络响应符合 `windows/docs/api-contract.md`。
- 断点检查：320px、768px、1024px、1440px；再检查 reduced-motion。

