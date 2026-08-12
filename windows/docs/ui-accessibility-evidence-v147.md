# UI Accessibility Evidence v147

日期：2026-08-12

## 变更范围

本轮优化移动端 Dashboard 的滚动上下文。手机断点会隐藏 `.site-nav`，用户进入 Analysis、Connect、Activity 或 History 深处后，原先顶部只剩 AI TOKEN 品牌和退出按钮，无法快速判断当前区域。

新增非交互的 `.mobile-route-context` 状态文字，由现有 `navigation.js` 的 `setActive` 统一更新；桌面端保持隐藏，透明品牌栏、原有导航链接、业务数据流和 API 均未改变。该状态仅承担可见上下文，不新增点击目标或第二套导航逻辑。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `127.0.0.1:5026`，验证完成后已关闭；保护服务 5000/5011 未重启、未停止、未修改。

| 视口 / 入口 | 当前 hash | 路由指示 | 指示宽度 | 顶栏宽度 | 目标落点 | 文档溢出 | 页面日志 |
| --- | --- | --- | ---: | ---: | ---: | --- | --- |
| 320×720 | `#activity` | `ACTIVITY` | `80px` | `275px` | `84px` | 无 | `[]` |
| 390×844 | `#history` | `HISTORY` | `141.7px` | `345px` | `326px` | 无 | `[]` |
| 1440×900 | `#activity` | 元素 `display:none` | — | `1320px` | `101px` | 无 | `[]` |

390px 深链回归确认 `Analysis / Connect / Activity / History` 会跟随现有 `aria-current="location"` active 状态更新；320px 下品牌宽度 `121px`、退出操作宽度 `66px`、中间状态栏 `80px`，没有挤压或横向滚动。桌面端原有导航链接几何保持，`Activity` 链接高度 `20px`、`aria-current` 正常。

## 可访问性与降级

- 路由状态使用普通文本节点，不承担操作，不加入 Tab 顺序；可见文本可被辅助技术读取，未使用 `aria-hidden`。
- 现有导航仍是唯一交互源；状态更新复用 `setActive`，hash 初始状态与 IntersectionObserver 状态保持同一来源。
- `prefers-reduced-motion: reduce` 关闭状态文字过渡；`forced-colors: active` 使用 `CanvasText`，移除文字阴影和 glow。
- 1440px 桌面端 `.mobile-route-context` 为 `display:none`，不影响既有透明 AI TOKEN 顶栏和导航节奏。
- 修改后文件仍低于 1000 行：`route-context.css` 79 行，`navigation.js` 333 行，`ui-polish.css` 991 行，`base.html` 82 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
