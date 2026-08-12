# UI Accessibility Evidence v150

日期：2026-08-12

## 变更范围

本轮修复 Dashboard 长页面 `data-reveal` 的可读性契约。此前共享规则会让尚未
进入视口的真实数据区块使用 `opacity: .78`，快速浏览时容易被误读为加载中，
也会让页面下半段的文字持续发灰。

现在新增 `token_tracker/static/reveal-readability.css`，由共享
`ui-polish.css` 引入。Dashboard 区块始终保持 `opacity: 1`，只使用 `8px`
的轻微垂直位移完成入场；进入视口后回到原位。这样保留了层次感，同时不再用
透明度降低真实数据的阅读对比度。`prefers-reduced-motion` 与
`forced-colors` 均关闭位移并保留清晰文本。

本轮不改变模板、DOM、导航、认证、接口、数据库、Provider、Key 或业务数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5026`，使用隔离
演示数据登录 Dashboard。现有 5000/5011 保护服务未重启、未停止、未修改。

| 入口 / 视口 | 场景 | reveal 可读性 | 文档溢出 | 页面日志 |
| --- | --- | --- | --- | --- |
| 5026 / 1440×900 | 首屏加载后 | 所有 8 个 Dashboard 区块 `opacity: 1`；未显示区仅 `translateY(8px)` | 无正向横向溢出 | `[]` |
| 5026 / 390×844 | 移动首屏加载后 | 所有 8 个 Dashboard 区块 `opacity: 1`；未显示区仅 `translateY(8px)` | 无正向横向溢出 | `[]` |
| 5026 / 390×844 | `scrollY=1250` / `ANALYSIS` | 已显示分析区与未显示连接区均 `opacity: 1`；`aria-current=Analysis` | 无正向横向溢出 | `[]` |

移动端透明品牌栏继续保持 `background=transparent`、`box-shadow=none`、
`backdrop-filter=none`。页面在 390px 视口下的文档宽度为 375px，未产生超出
视口的横向滚动通道。

## 可访问性与降级

- 真实交互控件保持原有 DOM、焦点顺序和 `tabIndex=0`；本轮只改变表现层。
- `prefers-reduced-motion: reduce` 下 reveal 不位移、不透明度过渡，内容直接可读。
- `forced-colors: active` 下 reveal 不位移、不透明度过渡，不引入固定颜色依赖。
- `ui-polish.css` 为 992 行，新增模块保持高内聚；所有文本源码文件低于 1000 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
