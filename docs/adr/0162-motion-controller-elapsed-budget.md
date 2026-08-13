# ADR-0162：共享动效时钟使用实测 elapsed frame budget

## 状态

已接受（ARCH-111a，2026-08-12）

## 背景

`MotionController` 使用唯一的 `PreciseTimer`，目标为 120Hz，调度槽位为整数 8ms。旧实现每次
tick 固定累积 `0.96` 帧预算（`120 × 8 / 1000`），因此整数槽位的长期平均值低于真实
`1000 / 120 = 8.333ms` 周期；Qt/Windows 调度抖动也无法反映到预算中。

## 决策

继续由 `MotionController` 单独拥有一个 8ms `PreciseTimer`，但每次 tick 使用已有的、受
`_MAX_PHASE_DELTA_SECONDS` 限制的实测 monotonic elapsed 累积 `elapsed × TARGET_HZ`。每次
tick 仍最多发出一个 `frame_changed(..., True)`，不追赶积压预算，不新增 timer、线程、组件
动画、业务状态或 repaint loop。

`_stop_timer()`、pause、reduced-motion、隐藏/最小化、rearm 和 close 继续清零时间锚点与剩余
预算；phase 仍由相同的 elapsed 和速度常量推进。这样修正调度目标的长期预算，又避免事件循环
短暂繁忙后产生突发帧。

## 备选方案

- 为每个组件新增 timer：拒绝，会破坏单一时钟和高内聚 presentation owner。
- 将 timer 改成 1ms：既有实验未改善真实 `app.exec()` cadence，增加唤醒和预算丢帧风险。
- 按剩余预算循环补发多帧：拒绝，会在恢复后制造 repaint burst，且不代表显示器刷新率。
- 只继续固定 `0.96`：实现简单，但无法校正整数 8ms 槽位与真实 8.333ms 目标的系统误差。

## 验证边界

真实 Qt offscreen 主循环样本：`87` 帧，平均间隔 `8.360ms`，p95 `16.000ms`，有效调度约
`119.61Hz`；timer interval 仍为 `8ms`。暂停与隐藏时 timer inactive，恢复后 active，关闭后
inactive。三主题、980×720/1240×820、四 workspace 的横向 scrollbar maximum 均为 `0`，
focus/overview 可见性保持。该证据不等同真实显示器 120Hz、HIDPI 或 EXE 正式性能。

本轮为 Python/PySide6 presentation-only，没有嵌入式 C/C++ 改动；public-vendor-source
applicability 为 N/A，不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。
