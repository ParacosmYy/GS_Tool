# UI Accessibility Evidence v140

日期：2026-08-12

## 变更范围

本轮修复 5000 旧启动链与 5011 完整启动链的场景动效断层。真实计算样式显示，5000 只加载 `style.css + ui-polish.css`，`.story-backdrop` 没有环境动画；5011 通过完整模板加载 `scene-motion.css`，使用 `backdrop-glow/backdrop-scan`。共享层现在为旧链提供低强度 `shared-scene-glow/shared-scene-scan`，让两个入口都保有轻微环境生命感，同时保持透明顶栏和文字阅读区不变。

新增动画只作用于装饰性 `.story-backdrop::after`，不改变 DOM、内容布局、表单、图表、周期切换、认证、Provider 或数据流。`prefers-reduced-motion` 会停止该伪元素动画；`forced-colors` 继续隐藏场景层。

## 真实浏览器证据

| 入口 | 视口 | 场景动效 | 背景焦点 | CTA | 周期切换器 | 文档宽度 |
| --- | --- | --- | --- | --- | --- | --- |
| `5000` | 390×844 | `shared-scene-glow, shared-scene-scan` / `18s, 21s` | `30% 50%` | `top=706.32 / bottom=754.32` | `top=799.14 / bottom=842.61` | `375 / 375` |
| `5000` | 320×720 | 共享低强度动效 | `30% 50%` | `top=652.13 / bottom=700.13` | `top=752.94 / bottom=796.42` | `305 / 305` |
| `5000` | 768×900 | 共享低强度动效 | `40% 50%` | `top=1030.59 / bottom=1078.59` | `top=1145.26 / bottom=1188.74` | `753 / 753` |
| `5000` | 1440×900 | 共享低强度动效 | `40% 50%` | `top=688.06 / bottom=736.06` | `top=830.52 / bottom=874.00` | `1425 / 1425` |
| `5011` | 390×844 | `backdrop-glow, backdrop-scan` / `16s, 13s` | `30% 50%` | 共享首屏布局 | 共享首屏布局 | `375 / 375` |
| `5011` | 1440×900 | 正式场景动效 | `40% 50%` | 共享桌面布局 | 共享桌面布局 | `1425 / 1425` |

四个周期按钮继续返回正确的 `aria-pressed` 状态：Today=`true`，Week/Month/All time=`false`。顶栏计算背景为透明 `rgba(0, 0, 0, 0)`，页面侧应用日志为 `[]`。

## 可访问性与降级

- DOM 快照确认 skip link、banner、main、导航、周期 group、表单控件、表格和 live status 均存在可访问名称或语义。
- 背景层 `aria-hidden=true`，不会进入屏幕阅读器内容；新增伪元素不产生焦点或文档尺寸。
- 标题层级为 H1 `TOKEN SIGNAL`，其后为 H2 区块标题；移动文档仍无正向横向溢出。
- `prefers-reduced-motion` 停止共享场景动画；`forced-colors` 隐藏装饰层并由系统颜色接管。
- 修改后文件仍低于 1000 行：`ui-polish.css` 966 行，`style.css` 674 行，`scene-motion.css` 385 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项仍按项目发布清单保持 pending。
