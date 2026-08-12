# UI 与可访问性证据 v134

## 变更主题

本轮聚焦 Dashboard 首屏元信息带。真实移动截图显示，`AI TOKEN / OBSERVATORY` 与 `已更新：今天` 直接叠在动态插画上，状态文本缺少局部阅读边界；直接加深整条 header 会破坏 v133 已完成的透明顶栏契约。

- 新增独立 `hero-signal.css`，只拥有 Dashboard hero 元信息的局部信号样式。
- 产品标签增加小型 lime beacon 和字形 keyline；更新状态增加低 alpha 深蓝胶囊、细边界、状态点和错误态颜色。
- 外围 `.hero-topline` 与 `.site-header` 继续透明；移动端限制胶囊宽度和文本为单行，避免状态文案把页面撑出横向滚动。
- `prefers-reduced-motion`、`forced-colors` 和既有 `role=status / aria-live=polite` 语义保持明确。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口 `5000/5011` 未重启、停止或修改进程。

### 桌面首屏

- `1683×892`：标签与状态胶囊同一行，状态胶囊高度 `30px`、宽度约 `109px`；外层元信息带仍为透明，无背景图和 blur。
- 截图确认 `已更新：今天` 从背景屏幕中分离出来，但没有形成贯穿页面的黑色横条。
- 文档宽度保持 `scrollWidth=clientWidth=1668px`。

### 390px 与 320px

- `390×844`：标签宽 `236.53px`、状态胶囊宽 `98.14px`，两者同一行，状态文案完整显示；页面 `scrollWidth=clientWidth=375px`。
- `320×720`：标签宽 `179.01px`、状态胶囊宽 `85.66px`，字号分别降至 `9px`，`已更新：今天` 仍完整显示；页面 `scrollWidth=clientWidth=305px`。
- 截图确认首屏标题、总量轨道、操作按钮和周期切换器没有被元信息层挤出视口。

### 5011 完整链与深滚

- `5011` 加载 `style.css`、`ui-polish.css`、`scene-motion.css`，由 `ui-polish.css` 的 import 继承 `hero-signal.css`。
- 深滚至 `scrollY=900` 后 header 为既有 `is-scrolled`，仍是 `background-color=transparent`、`background-image=none`、`backdrop-filter=none`、`box-shadow=none`；状态胶囊保留局部阅读材质。
- 浏览器页面日志为 `[]`；DOM 快照仍包含 `main`、hero 产品标签、`role=status` 和 `aria-live=polite`。

## 工程验证

- `hero-signal.css=163`、`ui-polish.css=920`，均低于项目 `1000` 行硬门禁。
- 无新增脚本、监听器、依赖、DOM 或数据契约；状态胶囊仅由 CSS 作用于既有节点。
- 保护进程保持不变：`5000 → PID 43832`、`5011 → PID 8100`。

## 未覆盖门禁

真实设备 UI、reduced-motion 实机、高对比度实机、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练继续按发布矩阵保持 pending。
