# UI-3 可访问性与视觉证据 v94

## 变更边界

- **问题**：深滚时卡片只剩一小段位于透明 AI TOKEN sticky 顶栏下方，形成第二条深色横带；短暂过渡还可能让提交动作露出。
- **实现**：`sticky-occlusion.js` 计算表面位于顶栏下方的可见片段；当片段不超过 120px 时加上 `is-sticky-fragment`，`responsive-tuning.css` 在非焦点状态下整体退出该片段。
- **交互边界**：不改变 DOM、布局盒、API、认证、表单值或 Tab 顺序；`:focus-within` 恢复完整卡片和动作行，reduced-motion/forced-colors 保持可见复位。
- **文件边界**：继续复用现有 sticky 表现模块；未继续增加接近 1000 行的 `ui-polish.css`。

## 隔离浏览器观察

- 实例：`http://127.0.0.1:5190/dashboard`，隔离账号仅用于本地视觉观察。
- 严格深滚：`scrollY = 2200`，header 为 `top = 0 / bottom = 76`，等待完整过渡后采样。
- 顶栏计算样式：`background-color = transparent`、`backdrop-filter = none`；截图确认 AI TOKEN 行下方连续显示背景插画。
- 自动采集卡片：`top = -727.9 / bottom = 156.1`，类名包含 `is-sticky-fragment`，`opacity = 0`、`pointer-events = none`。
- 自动采集操作行与提交按钮：分别约 `top = 46.4 / bottom = 94.4`，均为 `opacity = 0`、`pointer-events = none`；完整过渡后未露出动作残片。
- Tab 路径：真实键盘顺序可到达 `#auto-entry input[name="note"]`；聚焦时卡片 `opacity = 1`、`pointer-events = auto`、`focus-within = true`，输入框保持可见焦点路径。
- 主体内容保护：中间滚动时完整表单卡片保持 `opacity = 1`；离开交叠带后类名和 mask 清除。

## 可访问性与稳定性

- 可见焦点控件：40；缺少表单标签：0；标题结构：1 个 `h1`、8 个 `h2`。
- 页面宽度：`scrollWidth = clientWidth = 1668`，未观察到横向溢出。
- 页面日志：清洁；浏览器工具自身的 Statsig 网络超时/丢弃事件未进入页面日志，不属于应用错误。
- 未在本轮宣称真实设备、viewport override、`prefers-reduced-motion`、forced-colors 或真实 Provider Key 联调通过。
