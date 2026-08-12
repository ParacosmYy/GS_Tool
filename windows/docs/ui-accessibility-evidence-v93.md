# UI-3 可访问性与视觉证据 v93

## 变更边界

- **问题**：透明 AI TOKEN sticky 顶栏下方的深蓝卡片仍然穿过品牌行，用户会把下方卡片误读为顶栏不透明。
- **实现**：`sticky-occlusion.js` 为现有内容表面测量与顶栏的交叠区；`responsive-tuning.css` 仅裁掉卡片内部对应交叠带，保留背景场景透过品牌行。
- **交互边界**：不改变 DOM、API、认证、表单值、Tab 顺序或数据流；卡片 `:focus-within` 时移除视觉裁剪，避免键盘用户在操作字段时看到不完整表面。
- **文件边界**：继续复用现有 sticky 表现模块；未向接近 1000 行的 `ui-polish.css` 追加规则。

## 隔离浏览器观察

- 实例：`http://127.0.0.1:5189/dashboard`，隔离账号仅用于本地视觉观察。
- 深滚位置：`scrollY = 2200`，header 几何为 `top = 0 / bottom = 76`。
- 顶栏计算样式：`background-color = transparent`、`backdrop-filter = none`；截图确认 AI TOKEN 行可见背景插画，不再被下方卡片填充覆盖。
- 自动采集卡片：`top = -727.9 / bottom = 156.1`，只裁剪其内部 `727.9–803.9px` 的 76px 交叠带。
- 自动采集操作行：`top = 46.4 / bottom = 94.4`，`is-under-sticky-header`、`opacity = 0`、`pointer-events = none`；提交按钮和 Key 隐私说明不会穿过品牌行。
- 字段焦点：聚焦 `#auto-entry input[name="note"]` 后，active element 仍为该输入框，label `opacity = 1`、`pointer-events = auto`，卡片 mask 清除。
- 返回恢复：离开交叠带后卡片回到普通类名和 `mask = none`，布局表面完整恢复。

## 可访问性与稳定性

- 可见焦点控件：40；缺少可访问名称：0；缺少表单标签：0。
- 标题结构：1 个 `h1`、8 个 `h2`。
- 页面宽度：`scrollWidth = clientWidth = 1668`，未观察到横向溢出。
- 页面日志：清洁；浏览器工具自身的 Statsig 网络超时/丢弃事件未进入页面日志，不属于应用错误。
- 未在本轮宣称真实设备、viewport override、`prefers-reduced-motion`、forced-colors 或真实 Provider Key 联调通过。
