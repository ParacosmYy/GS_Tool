# UI-3 响应式与可访问性负责人

**作者：** AI Token Tracker Engineering Team  
**职责：** 保证视觉表达在真实窗口、触摸设备、键盘和辅助技术下仍然可用。

## 输入

- `token_tracker/templates/`
- `token_tracker/static/`
- `docs/ui-motion-spec.md`

## 输出

- 320/768/1024/1440 响应式检查记录；
- 标题层级、label、ARIA、live region、表格 caption、focus-visible 检查；
- loading、错误、空数据和无图表库 fallback 的可读状态。

## 验收

- 所有交互元素可用键盘到达并有明显焦点；
- 状态不只依赖颜色；
- 正文普通文本和 placeholder 不因暗色主题而难以阅读；
- 触摸目标和表格在小屏不溢出。
