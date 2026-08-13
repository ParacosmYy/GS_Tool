# ADR-0151：专注过渡复用共享 120Hz 帧驱动

日期：2026-08-12

状态：已采用（ARCH-100 / UI-1.173）

## 背景

共享 `MotionController` 已使用 8ms PreciseTimer 作为约 120Hz 的 presentation scheduler，但专注
设置↔总览的 `QParallelAnimationGroup` 子动画由 Qt 默认 animation driver 更新。在 Windows/offscreen
组合根采样中，driver 的 `valueChanged` 约 35Hz；高度和 reveal opacity 因而在少数中间帧集中变化，
用户看到组件挤在一起后再弹开。

## 决策

新增 `MotionDrivenAnimationGroup` 作为 presentation-only finite transition owner：保留
`QPropertyAnimation` 的 property/easing/interpolation contract，不调用 `start()`；每次共享
`MotionController.frame_changed` 到来时，以 `time.monotonic()` 的 elapsed time 设置 track 的
current time，再显式写入目标属性。focus owner 继续负责布局目标、effect 和所有 lifecycle cleanup。

该 owner 不创建 timer、线程、事件总线、业务状态、scroll owner 或设备 I/O。`request_activity()`
只延长已有共享时钟的 activity window。其他 page/theme/dialog fade 暂不迁移，避免在一片变更中扩大
行为范围。

## 验证与取舍

离屏真实组合根覆盖进入、退出、快速反转、stop、hide、minimize、close、reduced-motion、pause、
resize、三主题、四 workspace 和横向 scroll；最新活动帧间隔均值约 `9.04ms`，220ms transition 最终
settle，属性 track 数量和临时 effect 均能清理。该数值是当前 scheduler 的短时样本，不是显示器
精确 FPS 承诺；GUI/EXE startup、真实显示器/HIDPI、硬件/HIL、连接/OTA/debug backend 和签名验收
未运行或未授权。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。
Luna 架构师 `019ff36f-09e1-7a03-a5ac-2f4aa8393791` 与独立 reviewer
`019ff376-5ce1-7ae2-a4b6-aa1faf66cf7c` 均超时关闭，未形成外部结论；父代理完成 correctness、
architecture、security、performance、readability 五轴 review 与行为保持简化评估。不作固件标准或
认证声明。
