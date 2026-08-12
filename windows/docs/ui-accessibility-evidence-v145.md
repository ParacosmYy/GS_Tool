# UI Accessibility Evidence v145

日期：2026-08-12

## 变更范围

本轮优化 Dashboard 的 `03 / AUTOMATIC COLLECTION` 连接区动作层级。原先“自动检测模型”和“发送 · 自动记账”都使用高亮主按钮语言，连接说明也以长段落直接落在场景上，用户进入表单后不容易快速判断下一步。

新增 `connection-signal.css`，将检测动作收敛为低强调扫描按钮，把发送/自动记账保留为唯一 lime primary action；连接说明增加独立的左侧 signal rail 和局部渐变阅读面；provider 状态点增加低频 idle beacon。模板、字段、脚本、接口、Key 生命周期和数据流均未改变。

## 真实浏览器证据

| 入口 | 视口 | 检测按钮背景 | 提交按钮背景 | 连接卡片宽度 | 文档宽度 | 页面日志 |
| --- | --- | --- | --- | ---: | --- | --- |
| `5000` | 320×720 | `rgba(196, 192, 255, .055)` | `rgb(217, 255, 120)` | `274.7px` | `305 / 305` | `[]` |
| `5011` | 390×844 | `rgba(196, 192, 255, .055)` | `rgb(217, 255, 120)` | `344.7px` | `375 / 375` | `[]` |
| `5011` | 1440×900 | `rgba(196, 192, 255, .055)` | `rgb(217, 255, 120)` | `841.7px` | `1425 / 1425` | `[]` |

改后 390px 截图中，连接说明已形成独立协议轨道；“自动检测模型”是描边次级按钮，“发送 · 自动记账”仍为唯一实心 lime 动作。检测按钮、提交按钮均保持 `48px` 高、`tabIndex=0`，表单状态和可见焦点契约未改变。

## 可访问性与降级

- 只新增视觉层 CSS，不新增焦点节点、tab 顺序、ARIA 属性或交互事件。
- 检测按钮默认仍是可操作 `<button>`；hover/focus-visible 会获得 lime 边界，键盘用户不依赖颜色变化才能发现它。
- `forced-colors: active` 恢复 `CanvasText` / `ButtonText` / `ButtonFace`，关闭普通配色渐变、阴影与 beacon 动画。
- `prefers-reduced-motion: reduce` 关闭 idle beacon；动作按钮原有过渡保持产品级可用，不影响表单操作。
- 修改后文件仍低于 1000 行：`connection-signal.css` 122 行，`ui-polish.css` 989 行，`style.css` 674 行，`responsive-tuning.css` 693 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
