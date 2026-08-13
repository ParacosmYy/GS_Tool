# ADR-0152：一次性 UI 过渡共享时钟、独立 owner

日期：2026-08-12

状态：已采用（ARCH-101 / UI-1.174）

## 背景

专注过渡迁移到共享 `MotionController` 后，page、theme、transport panel 和 dialog 仍各自使用
Qt 默认属性动画 driver。这样同一窗口存在不同 cadence，theme sweep 还可能与 root fade 形成双时钟，
在低 cadence 中间帧出现视觉抖动。

## 决策

统一“时钟”而不合并“功能 owner”：每个过渡继续由自己的模块和有限 group 拥有，所有 group 复用
窗口级 `MotionController.frame_changed`。`MotionDrivenAnimationGroup` 内部保留
`QPropertyAnimation` 的插值/easing，不调用其 `start()`；只由 group 按 monotonic elapsed time 设定
current time 并写入目标属性。theme root opacity 与 sweep geometry 共享一个 group。

stop/finish 仍由 page/theme/transport/dialog/focus 各自负责，生命周期统一在现有 pause、reduced-motion、
resize、hide、minimize、close 边界清理。driver 发现 target 无效时 stop 并 emit finished，使 owner 有
机会释放 effect/overlay；不引入全局 TransitionManager、业务状态、线程、设备 I/O 或第二时钟。

## 验证与取舍

真实 composition root 的 Qt offscreen cadence：page `8.13–8.95ms`、theme `9.40–9.89ms`、transport
`8.21–9.05ms`、dialog `8.21–8.72ms`、focus `8.67ms`；均低于 `13ms` 工程阈值。覆盖 3 themes、4
workspaces、6 transports、两个 dialog show/hide、theme overlay cleanup、focus 和 window hide。
首次验证脚本的无变化 transport/不可见 panel 前置条件错误已修正后复跑通过；产品代码未为该脚本错误
改变行为。真实 Windows 可见窗口/高刷新显示器/HIDPI、高负载、EXE startup、硬件/HIL、连接/OTA/debug
实连和签名验收未运行或未授权。

ARCH-101 架构师 `019ff382-2d72-7d32-a067-09b13c9a32d5` 给出独立 owner/共享时钟建议；独立 reviewer
`019ff389-959a-77b0-8f26-1e444ee50ac6` 超时关闭，未形成外部结论；父代理完成五轴 review 与行为
保持简化评估。嵌入式 C/C++ public-source applicability 为 N/A，不作认证声明。
