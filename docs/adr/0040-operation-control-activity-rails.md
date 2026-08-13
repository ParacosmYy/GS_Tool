# ADR 0040：批量停止与历史回放暂停 activity rail

日期：2026-08-10

## 状态

已接受，UI-1.53。

## 决策

批量命令执行时，既有“停止”按钮复用 `BusyActionButton`，由 `connection.py` 把已有 `view_model.batch_active` 投影为
`set_busy()`；批量未执行时静态。历史回放“暂停/继续”按钮同样复用 `BusyActionButton`，由 `replay.py` 只把既有
`ReplayState.PLAYING` 映射为 busy；`PAUSED` 的“继续”、`EOF/STOPPED/ERROR/EMPTY` 均静态。

两个按钮仍由原有 Qt signal、enabled gate、文本和 AccessibleName/Description 提供权威交互语义，装饰只通过 lifecycle 的
共享 `MotionController` frame/stop fan-out 更新。视觉 busy 不表示设备完成度、批量百分比、回放总进度或写入确认。

## 边界

- `terminal.py`/`protocol.py` 只负责构造原生兼容按钮；
- `connection.py`/`replay.py` 只投影已有 batch/replay 状态；
- `lifecycle.py` 只加入 frame/stop fan-out；
- 不新增 timer、ViewModel 字段、业务状态源、renderer 或操作副作用。

## 验证要求

覆盖 batch inactive/active、replay EMPTY/PLAYING/PAUSED/EOF/STOPPED/ERROR、native type/click/accessibility、共享 frame、
reduced-motion/stop fallback、三主题和最终 provenance 构建。
