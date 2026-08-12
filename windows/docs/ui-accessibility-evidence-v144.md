# UI Accessibility Evidence v144

日期：2026-08-12

## 变更范围

本轮修复 Dashboard 分享链接 `/dashboard#overview` 的首屏定位。此前 `#overview` 位于 sticky `76px` 品牌栏之后，却没有 `scroll-margin-top`；浏览器原生 hash 定位会把 hero 顶部滚到 `0px`，造成页面 `scrollY=76`、header 进入 `is-scrolled`、首屏 CTA 开始退场，用户打开分享链接时看起来像已经滚动过页面。

共享 `ui-polish.css` 现在把 `#overview` 纳入既有锚点间距契约，与 `#analysis`、`#connect`、`#activity`、`#history`、`#auto-entry` 和 `#manual-entry` 使用相同的 sticky header clearance。没有修改脚本、DOM、导航、认证、接口、数据或动画逻辑。

## 真实浏览器证据

| 入口 | 视口 | URL | `scrollY` | hero 顶部 | CTA opacity | 文档宽度 | 页面日志 |
| --- | --- | --- | ---: | ---: | ---: | --- | --- |
| `5000` | 390×844 | `#overview` | `0` | `76px` | `1` | `375 / 375` | `[]` |
| `5011` | 390×844 | `#overview` | `0` | `76px` | `1` | `375 / 375` | `[]` |
| `5011` | 1440×900 | `#overview` | `0` | `76px` | `1` | `1425 / 1425` | `[]` |

修复前 5011 的 390px `#overview` 深链为 `scrollY=76`，hero 顶部为 `0px`、CTA opacity 约 `0.002`；修复后两个入口均回到真实首屏，header class 不含 `is-scrolled`，CTA transform 为 `none`。普通深链没有回归：`#analysis`、`#connect`、`#history` 仍落在 sticky header 下方约 `84–101px` 的阅读位置。

## 可访问性与降级

- 只调整 CSS `scroll-margin-top`，不新增焦点节点、tab 顺序、ARIA 属性或可交互区域。
- `#overview` 仍是原有 `<section>`，分享 URL、浏览器历史和原生 anchor 语义保持不变。
- sticky header clearance 继续由既有 `clamp(84px, 7vw, 104px)` 令牌提供，避免重复 magic number。
- reduced-motion、forced-colors、键盘导航和屏幕阅读器结构未被覆盖。
- 修改后文件仍低于 1000 行：`ui-polish.css` 988 行，`style.css` 674 行，`responsive-tuning.css` 693 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
