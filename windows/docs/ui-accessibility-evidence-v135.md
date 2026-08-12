# UI / Accessibility Evidence v135

日期：2026-08-12  
范围：Dashboard 空态图表的窄屏横向溢出契约、旧入口 `5000`、完整入口 `5011`

## 变更摘要

本轮只处理空态/不可用态图表的 presentation boundary：

- 关闭空态卡片沿用的 off-canvas hover sheen，避免伪元素在卡片内部制造隐藏横向滚动通道。
- 仅对 `empty` / `unavailable` 状态裁剪图表扫描层；有真实数据的图表不改变 viewport contract。
- 将 Chart.js 尚未绑定数据时的隐藏 300px canvas 收缩到当前空态容器，避免 320px 设备出现内部不可见宽度。
- 在 forced-colors 下同步隐藏空态 sheen；没有新增脚本、DOM、依赖、接口、认证或数据改动。

## 根因证据

v135 初始 390px 检查发现：`.chart-wide` 的可视宽度为 `343px`，但 `scrollWidth` 为 `575px`。页面根节点没有正向横向溢出，是因为卡片自身已经裁剪；问题来自空态卡片仍保留的 `::before` 装饰扫描层，其 off-canvas transform 会扩大卡片内部滚动尺寸。

在 320px 检查中，空态 `canvas` 仍保留 `300px` intrinsic width，而容器只有 `236px`。虽然 wrapper 已裁剪，该内部尺寸仍然不符合窄屏空态的尺寸契约。

## 浏览器回归

| 入口 | viewport | 空态卡片 `client/scroll` | chart wrapper `client/scroll` | canvas `client/scroll` | 文档 `client/scroll` |
| --- | ---: | ---: | ---: | ---: | ---: |
| `5000` | `320×720` | `273 / 273` | `236 / 236` | `236 / 236` | `305 / 305` |
| `5000` | `390×844` | `343 / 343` | `306 / 306` | `306 / 306` | `375 / 375` |
| `5000` | `1683×892` | `785 / 785` | `734 / 734` | — | `1668 / 1668` |
| `5011` | `390×844` | `343 / 343` | `306 / 306` | `306 / 306` | `375 / 375` |

附加检查：

- `5000` 390px 的 `.chart-card::before` computed `display` 为 `none`，空态文案仍完整可见。
- `5011` 使用已存在的本地会话重定向到 Dashboard，顶部品牌行、状态胶囊和首屏插画透景截图正常。
- 5000/5011 页面检查未采集到应用侧错误日志；没有读取或输出 cookie、storage、token、key 等敏感信息。
- 保护服务端口未重启、未停止、未修改。

## 质量门禁

- Python `compileall`：通过。
- `git diff --check`：通过。
- 项目 audit：`14 pass / 1 pending / 0 fail`；pending 仍为真实设备/Android/EXE 签名、HTTPS/ACL、真实 Provider、备份恢复、限流等发布门禁。
- `legacy-observatory.css` 继续低于 1000 行；本轮没有触及后端、数据库、API 或业务数据。

## 角色签核

- UI-1：确认空态图表在 320/390px 与桌面端均保持连续、可读且无隐藏横向通道。
- UI-2：确认规则限定在 `empty` / `unavailable` presentation state，populated chart、Chart.js 数据契约与交互未被改变。
- 架构师：确认改动位于旧入口兼容 presentation boundary，未引入跨层依赖；forced-colors 与文件行数门禁保留。
