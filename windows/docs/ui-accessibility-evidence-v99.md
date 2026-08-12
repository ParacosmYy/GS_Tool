# UI-3 可访问性与视觉证据 v99

## 变更边界

- **问题**：最上方 `AI TOKEN / OBSERVATORY` 品牌行虽已声明透明，但其下方 hero 元信息的全宽底线与场景暗部叠加后，仍容易被误读为黑色覆盖层。
- **实现**：`responsive-tuning.css` 在普通配色下再次明确 `.site-header` 与 `.site-header.is-scrolled` 为 `transparent / background-image: none / backdrop-filter: none / box-shadow: none`；`.hero-topline` 取消全宽底线，仅保留短的 lime → lavender 信号线。
- **交互边界**：不改变 DOM、布局盒、导航、周期 API、认证、表单值、数据流、Tab 顺序或指标布局；forced-colors 继续由系统色彩契约接管。
- **文件边界**：变更只落在响应式表现层；`ui-polish.css` 保持 995 行。

## 隔离浏览器观察

- 实例：`http://127.0.0.1:5195/dashboard`，隔离账号仅用于本地视觉观察。
- 真实矩阵：`320×720`、`390×844`、`768×900`、`1024×768`、`1440×900` 均确认 header 与 hero-topline 的背景色为 transparent、无 blur、无阴影；页面无横向溢出。
- 390px 移动端：品牌行首屏与 `scrollY≈720` 滚动态均保持透景；hero 短信号线随宽度收缩，不形成连续暗带。
- 周期交互：真实点击 Today、Week、Month、All time 后四档 `aria-pressed` 与 active 状态均同步；最终回到 Today。

## 可访问性与稳定性

- 最终 390px 检查：36/36 当前可见控件具备可读名称；标题结构为 1 个 `h1`、8 个 `h2`。
- 应用页面日志保持清洁；浏览器工具自身 Statsig 网络超时/丢弃事件不属于应用日志。
- 未在本轮宣称真实设备、`prefers-reduced-motion`、forced-colors 或真实 Provider Key 联调通过。
