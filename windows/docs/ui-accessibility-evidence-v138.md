# UI Accessibility Evidence v138

日期：2026-08-12

## 变更范围

本轮只收敛 Dashboard hero 底部周期切换器在正常高度手机首屏中的可见性。390×844 首屏原先的周期切换器位于 `top=807 / bottom=851`，视口底部为 844px，Today 活动项底部为 846px，导致用户第一次进入页面时只能看到被截断的周期选择器。

最终规则落在 `ui-polish.css` 的共享兼容层：361–620px 宽、761px 以上高的手机将 `.hero-foot` 顶部间距从 18px 收敛为 10px。短屏保留原有空间分配，避免 CTA 被压缩。初次尝试放在 `responsive-tuning.css`，但真实保护实例的旧启动链未加载该文件，已撤回并将规则放回两个启动链共同加载的共享样式，未留下重复规则。

## 真实浏览器证据

| 入口 | 视口 | 周期切换器 | hero footer | 文档宽度 | 结果 |
| --- | --- | --- | --- | --- | --- |
| `5000` | 390×844 | `top=799.14 / bottom=842.61 / height=43.48` | `padding-top=10px` | `375 / 375` | 完整进入首屏 |
| `5011` | 390×844 | `top=799.14 / bottom=842.61 / height=43.48` | `padding-top=10px` | `375 / 375` | 共享入口一致 |
| `5000` | 320×720 | `top=753 / bottom=796` | `padding-top=18px` | `305 / 305` | 短屏契约保持 |
| `5000` | 768×900 | `top=1145.26 / bottom=1188.74` | `padding-top=18px` | `753 / 753` | 平板布局未改变 |
| `5000` | 1440×900 | `top=830.52 / bottom=874.00` | `padding-top=18px` | `1425 / 1425` | 桌面布局未改变 |

四个周期按钮继续返回正确的 `aria-pressed` 状态：Today=`true`，Week/Month/All time=`false`。页面侧应用日志为 `[]`，没有新增横向滚动，也没有触碰后端、数据库、认证、Provider、Key 或端口进程。

## 可访问性与降级

- 保留既有按钮语义、键盘切换和焦点样式；本轮只调整空间，不改变交互脚本。
- `prefers-reduced-motion` 与 `forced-colors` 规则未被覆盖；新规则只改变 `padding-top`。
- 所有修改文件继续低于 1000 行：`ui-polish.css` 927 行，`responsive-tuning.css` 693 行，`style.css` 674 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项仍按项目发布清单保持 pending。
