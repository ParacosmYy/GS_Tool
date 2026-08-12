# UI / Accessibility Evidence v137

日期：2026-08-12
范围：旧入口导航兼容、Activity 工作信号区可达性、sticky 顶部定位、`5000` 与 `5011`

## 变更摘要

本轮处理旧运行模板与当前 Dashboard 内容不同步造成的导航断层：

- 当页面存在 `#activity`，但 `.site-nav` 缺少 Activity 链接时，由导航模块补入一个固定的同文档入口。
- 兼容补链复用现有导航的 active state、hash 定位、滚动动画和 `aria-current="location"` 机制。
- 当前模板已经提供 Activity 时不会重复插入；不新增后端、API、数据、认证或业务状态。
- 移动端继续沿用既有规则隐藏 `.site-nav`，补链不改变文档宽度。

## 根因证据

v137 的真实运行页面检查显示：页面包含 `#activity`（“记录工作信号 / 最近工作事件”），但顶部导航只有 `Analysis / Connect / History` 三项；这使活动记录区只能通过长滚或页面内其它入口到达。工作区当前 `base.html` 已经包含 Activity 链接，说明差异来自保护服务继续运行的旧模板缓存，而不是当前模板缺失。

## 浏览器回归

| 入口 | viewport | 导航结果 | Activity 深链接 | 文档 `client/scroll` |
| --- | ---: | --- | --- | ---: |
| `5000` | `1440×900` | 4 项：Analysis / Connect / Activity / History | — | `1425 / 1425` |
| `5000` | `1024×900` | 4 项可见，导航宽 `270px`，无挤压 | — | `1009 / 1009` |
| `5000` | `390×844` | 4 项存在但按既有响应式隐藏 | — | `375 / 375` |
| `5000` | `1440×900` | — | `scrollY=2431`，目标 top `101px`，位于 76px sticky header 下方 | `1425 / 1425` |
| `5011` | `1024×900` | 4 项，Activity 仅 1 个，兼容标记仅 1 个 | — | `1009 / 1009` |

附加检查：

- Activity 深链接后 Activity 链接获得 `aria-current="location"` 与 `is-active`。
- `#activity` 深链接目标实际高度 `896px`，computed opacity `1`、transform 为 `none`。
- 5000/5011 顶部品牌行 computed `background: transparent`、`background-image: none`、`backdrop-filter: none`、`box-shadow: none`。
- 运行页面侧日志数组为 `[]`；浏览器工具外部遥测提示不来自本地应用代码。
- 未读取或输出 cookie、storage、token、Key 等敏感信息；保护服务未重启、未停止、未修改。

## 质量门禁

- Python `compileall`：待提交前执行。
- `git diff --check`：待提交前执行。
- 项目 audit：待提交前执行；预期保持 `14 pass / 1 pending / 0 fail`。
- `navigation.js` 与证据/工程文档均低于 1000 行。

## 角色签核

- UI-1：确认 Activity 工作信号区从顶部导航可达，且补链不会改变桌面构图或移动端节奏。
- UI-2：确认兼容逻辑只补 presentation/navigation route，不触碰业务数据、API、认证和表单状态。
- 架构师：确认导航、active state 与 hash motion 仍由 `navigation.js` 单点负责，兼容 guard 可独立回滚。
