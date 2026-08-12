# UI-3 可访问性与视觉证据 v100

## 变更边界

- **问题**：AI TOKEN 品牌 header 已透明，但 hero 元信息右侧的“已更新：今天”仍继承深色玻璃填充和 blur，使顶部信息层看起来不连续。
- **实现**：`responsive-tuning.css` 在普通配色下将 `#dashboard-status`（含错误态）改为透明背景、无 `backdrop-filter`，保留边界、状态点和轻量阴影；同时为 `OBSERVATORY` 副标题增加局部字形 keyline。
- **交互边界**：不改变 DOM、布局盒、导航、周期 API、认证、表单值、数据流或 Tab 顺序；forced-colors 继续由系统色彩契约接管。
- **文件边界**：变更只落在响应式表现层；`ui-polish.css` 保持 995 行。

## 隔离浏览器观察

- 实例：`http://127.0.0.1:5200/dashboard`，隔离账号仅用于本地视觉观察。
- 真实矩阵：`320×720`、`390×844`、`768×900`、`1024×768`、`1440×900` 均确认 header 与状态胶囊为透明背景、无 blur；页面无横向溢出。
- 深滚：`scrollY=720` 稳定后，sticky header、状态胶囊仍保持透明、无 blur、无阴影面板；品牌副标题 keyline 保留。
- 周期交互：真实点击 Today、Week、Month、All time 后四档 `aria-pressed` 与 active 状态均同步；最终回到 Today。

## 可访问性与稳定性

- 最终 1440px 检查：40/40 当前可见控件具备可读名称；标题结构为 1 个 `h1`、8 个 `h2`。
- 应用页面日志保持清洁；浏览器工具自身 Statsig 网络超时/丢弃事件不属于应用日志。
- 未在本轮宣称真实设备、`prefers-reduced-motion`、forced-colors 或真实 Provider Key 联调通过。
