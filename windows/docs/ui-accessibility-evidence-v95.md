# UI-3 可访问性与视觉证据 v95

## 变更边界

- **问题**：快速跳过长页面区块时，`IntersectionObserver` 可能没有收到交叠回调；返回页面后，区块仍停留在 `.78 / 6px` 的预落位状态。
- **实现**：`motion.js` 维护轻量的 scroll/resize `requestAnimationFrame` 同步器；每次滚动检查已经越过 `.92` 视口阈值的区块，立即补齐 `is-visible`、清除延迟并解除观察。
- **交互边界**：只改变 reveal 的视觉生命周期，不改变 DOM、数据请求、认证、表单值、Tab 顺序或业务接口；隐藏的动态面板继续在打开时保持可读。
- **文件边界**：修复集中在 motion 模块；没有继续增加接近 1000 行的 `ui-polish.css`。

## 隔离浏览器观察

- 实例：`http://127.0.0.1:5191/dashboard`，隔离账号仅用于本地视觉观察。
- 深度快跳：`scrollY ≈ 2995`，页面总高度 `3887px`；8 个 `[data-reveal]` 区块均完成 `is-visible`。
- 返回路径：回到 `scrollY = 1200` 后，8 个 `[data-reveal]` 区块全部为 `opacity = 1`、`transform = none`；未复现预落位灰度残留。
- 顶栏与布局：页面宽度 `scrollWidth = 1668px`，视口宽度 `1683px`，未观察到横向溢出。

## 可访问性与稳定性

- 标题结构：1 个 `h1`、8 个 `h2`；当前可见交互控件 39 个，均具备可见文本、关联 label 或 ARIA 名称。
- 页面日志：清洁；浏览器工具自身的 Statsig 网络超时/丢弃事件未进入页面日志，不属于应用错误。
- 未在本轮宣称真实设备、viewport override、`prefers-reduced-motion`、forced-colors 或真实 Provider Key 联调通过。
