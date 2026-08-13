# ADR-0173：高刷新共享动效时钟与响应式概览指标

## 状态

已接受（ARCH-122 / UI-1.195，2026-08-12）

## 决策

- 保持窗口内唯一 `MotionController` 与唯一 `PreciseTimer`，scheduler slot 为 4ms。
- 将 MotionController 内部所有 elapsed/deadline 读取统一为 `time.perf_counter()`；业务层不迁移时间源。
- frame budget 按真实 elapsed 累积，出帧后取模只保留小数余量，每 callback 最多一次 emit，禁止过期预算补发 burst。
- 扩展概览使用独立 `_ResponsiveMetricGrid` 按 6/3/2/1 列重排已有指标；value label 可收缩换行，accessible description 保留完整值。

## 边界

本 ADR 只覆盖 Python/PySide6 presentation。不得新增 timer、线程、业务状态、DTO 字段、scroll owner、设备连接或 OTA/AES/RTT/J-Link backend。
`TARGET_HZ=120` 是 scheduler target，不是显示器实际 120fps 保证；真实 GUI/HIDPI/EXE/硬件验收仍需授权。

## 评审与验证

架构师 `019ff579-3e62-7551-9cf5-4907a3638a9a` APPROVE 使用 perf_counter 统一时间域并保留4ms；独立 reviewer
`019ff57c-bd7a-79b3-838c-6471f5a8d87c` APPROVE，无 Critical/Required findings；简化 reviewer
`019ff57c-bdd6-7f30-965b-cb05faefd0b1` 无必须简化项。embedded C/C++ public-vendor-source applicability=N/A。

`compileall`、Ruff、`scripts/check.ps1` 通过；10.2秒 offscreen 为 1224 frames/119.992Hz/平均8.334ms/p95 12.097ms；50ms 卡顿
单 tick 只 emit 1 帧且 budget=0.259；activity deadline、pause/suspend/close 和 640/800/980 长文本 geometry 通过。以上不等同显示器/EXE 性能。
