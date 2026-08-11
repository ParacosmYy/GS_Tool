# UI / Accessibility Evidence · v84

日期：2026-08-12
变更：为 Dashboard 的 `TOTAL SIGNAL` 核心计数器增加低干扰的内部扫描弧。
隔离实例：`http://127.0.0.1:5181`
数据：仅使用隔离 SQLite 与合成观察账号；未读取、输出或持久化任何真实 Key、Cookie 或密码。

## 变更边界

- `scene-motion.css` 为现有 `.hero-core` 增加 `::before` 装饰弧与 `core-sweep` transform 动画。
- 扫描弧位于 live number 之后、文字之前的背景层，不新增 DOM、不改变数据/API/认证/布局或焦点顺序。
- `forced-colors: active` 隐藏装饰伪元素；`prefers-reduced-motion: reduce` 禁止扫描弧与外环动画。
- 未改变 v83 的完全透明 AI TOKEN 顶栏契约。

## 实时浏览器证据

| 检查项 | 结果 | 证据 |
| --- | --- | --- |
| 首屏视觉 | pass | 默认桌面视口 `1683 × 892` 截图中，TOTAL SIGNAL 仍是焦点，内部 lime 弧细且不遮挡数字、标签或插画。 |
| 动画挂载 | pass | `hero-core::before` computed `animation = 7.2s linear infinite core-sweep`；首个采样矩阵与约 `950ms` 后矩阵不同，确认 transform 正在运行。 |
| 可读性层级 | pass | live number、`TOTAL SIGNAL` 与 calls 文案保持原 DOM 和 z-order，扫描弧只占内圈边界，不覆盖文本。 |
| 横向溢出 | pass | `documentElement.clientWidth = 1668`，`scrollWidth = 1668`。 |
| 可访问性语义 | pass | 保留 banner、main、命名导航、`TOKEN SIGNAL` h1、`当前范围总 token` hero label 与 `统计周期` group；伪元素不进入可访问性树。 |
| 页面日志 | pass | `tabV84.dev.logs({})` 返回空日志数组。 |

## 限制与后续观察

- 本轮浏览器会话未提供 viewport override 能力，因此没有把默认桌面实时结果扩展成 320/390/768/1024/1440px 的新通过结论；历史矩阵继续参考 v79/v81/v83 证据。
- 实时复核还发现周期切换的滑动 pill 在异步摘要刷新开始时可能短暂落后一帧；这是独立的交互同步问题，不属于 v84 扫描弧变更，记录为下一枚小切片处理。
- 真实设备、reduced-motion/forced-colors 运行时偏好、Provider 联调和正式部署仍保持未关闭。

## 验收结论

v84 的单一 UI 目标已在隔离实例中完成：TOTAL SIGNAL 从静态空心圆变为具有克制扫描反馈的信号核心，同时保持内容可读、DOM 稳定与无障碍降级边界。
