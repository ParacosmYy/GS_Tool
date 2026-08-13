# ADR 0079：共享动效帧生命周期门禁

日期：2026-08-10

状态：已接受

## 背景

`MotionController` 已经在隐藏、最小化、暂停、低动效和关闭时停止自己的计时器，但 Qt 事件队列中可能仍有一帧已经排队的 `frame_changed` 回调。旧的 lifecycle 分发器只检查关闭状态，过期帧可能重新点亮装饰 surface 或 RX 活跃态。

## 决策

- `controllers/lifecycle.py:on_motion_frame()` 作为共享帧的唯一生命周期门禁；关闭、不可见、最小化或 `workspace_motion_enabled()` 为 false 时拒绝分发。
- 被拒绝的帧必须冻结所有 presentation motion consumer，并将 data-activity 投影清回静态态；不触碰 SessionViewModel、传输、记录、解析或发送状态。
- 动效策略继续复用 `motion_policy.py`/`workspace_motion_enabled()`；不新增第二个策略源、timer、事件总线或控件自有时钟。
- `MotionController` 仍只负责低频时钟和帧信号，lifecycle 仍负责窗口可见性、关闭和 widget consumer 清理，保持 owner 边界。

## 结果

隐藏/最小化/低动效/关闭路径不会因排队 Qt 信号留下视觉脉冲；正常可见且允许动效的窗口仍按原路径 fan-out。验证采用静态检查、compileall、ruff 和不显示主窗口的 offscreen 内存向量；真实 GUI、HIDPI、硬件和正式发行仍需授权环境单独验收。

