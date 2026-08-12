# UI / Accessibility Evidence v136

日期：2026-08-12
范围：Dashboard 长页面 reveal 可读性、旧入口 `5000`、现有完整入口 `5011`

## 变更摘要

本轮只修复长页面进入动效的视觉层级：

- 将共享 `data-reveal` 的预显隐从 `opacity: .34 / translateY(12px)` 收敛为 `opacity: .78 / translateY(6px)`。
- 将过渡时间从 `0.82s` 收敛为 `0.64s`，减少用户快速滚动时的“黑幕覆盖”感。
- 规则仍由 `ui-polish.css` 的单一 reveal 契约持有，旧 `5000` 兼容入口与 `5011` 现有链路共同复用。
- `prefers-reduced-motion: reduce` 仍强制 `opacity: 1; transform: none`；没有新增 DOM、脚本、依赖、API、认证或数据改动。

## 根因证据

v136 初始真实页面测量发现，5000 页面完成 `motion-ready` 初始化后的首屏等待阶段，`#analysis`、`#connect`、`#activity` 和 `#history` 会进入约 `0.44` 的中间透明度，随后仍以低透明度等待 IntersectionObserver。长页面深滚时，这种状态会被感知为新的黑色遮罩，而不是轻量进入动画。

此前 `responsive-tuning.css` 已定义 `.78 / 6px / .64s` 的目标契约，但旧入口的 `ui-polish.css` 末尾规则以 `.34 / 12px / .82s` 覆盖了它。本轮将参数直接收敛到原有共享规则，不保留重复覆盖块。

## 浏览器回归

| 入口 | viewport | reveal 结果 | 文档 `client/scroll` |
| --- | ---: | --- | ---: |
| `5000` | `1683×892` | 未进入视口区块稳定在 `.78 / 6px`；进入 Analysis/Connect 后过渡到 `1 / 0px` | `1668 / 1668` |
| `5000` | `390×844` | 首屏构图、透明品牌栏和 hero signal 保持；长页面区块使用同一 reveal 参数 | `375 / 375` |
| `5000` | `320×720` | 空态图表画布 `236 / 236`，章节内容可读，无横向溢出 | `305 / 305` |
| `5000` | `768×900` | 平板布局与 reveal 状态稳定，无断层 | `753 / 753` |
| `5011` | `1440×900` | 重定向后的 Dashboard 使用 `.64s` reveal 契约，认证入口可用 | `1425 / 1425` |

附加检查：

- 5000 桌面深滚 `scrollY=1200` 时，Analysis/Connect 已获得 `is-visible`，其余未进入阅读窗口的区块保持 `.78` 轻层次，不再降为 `.34` 黑幕态。
- 390/320 空态图表的 card、wrapper、canvas 尺寸契约保持 v135 结果，无新增内部滚动通道。
- 页面侧只读采集的应用日志数组为 `[]`；浏览器工具产生的 Statsig 外部遥测超时/丢弃提示不来自本地应用代码，也未被写入项目日志。
- 未读取或输出 cookie、storage、token、Key 等敏感信息；保护服务未重启、未停止、未修改。

## 质量门禁

- Python `compileall`：待提交前执行。
- `git diff --check`：待提交前执行。
- 项目 audit：待提交前执行；预期保持 `14 pass / 1 pending / 0 fail`。
- `ui-polish.css`、`style.css`、`responsive-tuning.css` 和本轮文档均低于 1000 行。

## 角色签核

- UI-1：确认长页面预显隐仍有层次，但不会把正常章节误读成黑色覆盖层。
- UI-2：确认只调整既有 reveal presentation contract，populated chart、表单、周期切换、导航和数据流未改变。
- 架构师：确认没有新增重复模块或跨层耦合；reduced-motion、forced-colors 与文件行数门禁保持有效。
