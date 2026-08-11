# UI / Accessibility Evidence · v85

日期：2026-08-12
变更：修复统计周期切换后滑动 active pill 落后一拍的问题。
隔离实例：`http://127.0.0.1:5181`
数据：仅使用隔离 SQLite 与合成观察账号；未读取、输出或持久化任何真实 Key、Cookie 或密码。

## 变更边界

- `range-switcher.js` 在 pressed state 更新的同一任务内立即测量 active button。
- 保留原有 `requestAnimationFrame` 二次测量，用于字体加载、ResizeObserver 和响应式布局完成后的最终校准。
- 不改变 period API、摘要请求、URL、DOM 结构、ARIA pressed 语义或业务数据。

## 实时浏览器证据

| 检查项 | 结果 | 证据 |
| --- | --- | --- |
| Week | pass | 点击 `Week` 后 active=`Week`，pill `x=61px / width=50.40625px`，与 active button 实际几何完全一致；状态为“已更新：本周”。 |
| Month | pass | 点击 `Month` 后 active=`Month`，pill `x=115.40625px / width=57px`，几何一致；状态为“已更新：本月”。 |
| All time | pass | 点击 `All time` 后 active=`All time`，pill `x=176.40625px / width=76.80208587646484px`，几何一致；状态为“已更新：全部”。 |
| Today | pass | 点击 `Today` 后 active=`Today`，pill `x=0 / width=57px`，几何一致；状态为“已更新：今天”。 |
| 可访问性 | pass | 每个周期按钮继续暴露独立 accessible name 与 `aria-pressed`，仅 active 状态变化；统计周期 group 保持原语义。 |
| 横向溢出 | pass | 实时浏览器 `documentElement.clientWidth = 1668`，`scrollWidth = 1668`。 |
| 页面日志 | pass | v85 交互后的 `tabV84.dev.logs({})` 无应用错误或警告。 |
| 顶栏回归 | pass | 滚动态 computed `site-header` 仍为 `background-color = rgba(0, 0, 0, 0)`、`backdrop-filter = none`；v83 透明契约未回退。 |

## 限制

- 本轮浏览器会话未提供 viewport override 能力，因此没有把默认桌面实时结果扩展成 320/390/768/1024/1440px 的新通过结论；历史矩阵继续参考 v79/v81/v83/v84 证据。
- 真实设备、reduced-motion/forced-colors 运行时偏好、Provider 联调和正式部署仍保持未关闭。

## 验收结论

v85 已消除周期文字与 active pill 几何不同步的问题：切换反馈在同一任务内落位，异步摘要刷新不会让视觉状态停留在上一个周期。
