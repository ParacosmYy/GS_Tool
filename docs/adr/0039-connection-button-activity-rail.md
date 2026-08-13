# ADR 0039：连接主按钮 activity rail

日期：2026-08-10

## 状态

已接受，UI-1.52。

## 决策

连接控制带的主连接按钮复用 `presentation/action_surface.py:BusyActionButton`。连接 controller 已经拥有
`SessionState.OPENING/CLOSING` 的权威投影，因此只把这两个状态映射为 `set_busy(True)`；`DISCOVERED/OPEN/CLOSED/ERROR`
保持静态。按钮仍由 `connection_runtime.toggle_connection()` 接收原生 click，文案、enabled gate、QSS、AccessibleName/
AccessibleDescription 和连接动作不改变。

按钮纳入 `lifecycle.py` 的既有 `_motion_surfaces` fan-out，继续使用唯一共享 `MotionController`。低动效、暂停、隐藏、
最小化和关闭只冻结装饰轨道，不改变 session 状态或连接请求。

## 边界

- `connection_builder.py` 只负责构造 `BusyActionButton`；
- `connection.py` 只投影既有 `SessionState`，不读取 ViewModel 新字段、不创建 timer；
- `lifecycle.py` 只负责 frame/stop fan-out；
- 不新增 renderer、状态源、连接重试、进度百分比或硬件行为。

## 拒绝的替代方案

- 不在按钮内部创建 `QTimer` 或启动连接状态轮询；
- 不把 opening/closing 伪装成百分比进度或传输完成度；
- 不复制一套 connection-specific painter；
- 不让 builder 直接读取 session 或 ViewModel。

## 验证要求

覆盖 `OPENING/CLOSING` busy、`OPEN/CLOSED/ERROR` static、native button type/click/accessibility、共享 frame、
reduced-motion/stop fallback、三主题与 980px 布局；最终运行静态门禁和 provenance 构建校验。
