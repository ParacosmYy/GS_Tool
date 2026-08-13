# ADR-0143：响应式连接带与 120Hz presentation pacing

- 状态：Accepted for ARCH-92 / UI-1.165
- 日期：2026-08-12
- 范围：`presentation/controllers/composition.py`、`connection_builder.py`、
  `presentation/widgets.py`、`controllers/lifecycle_motion.py` 及四个 motion surface

## 背景

真实组合根的连接页在短内容场景会把连接控制带垂直拉长，形成用户看到的白色/空白段；同时共享
动效虽然声明 120Hz target，但整数 8ms timer slot 等价于约 125 ticks/s，且所有 surface 都会
在每个 animated frame 调用 `update()`，增加无动态 glyph 的 repaint fan-out。

## 决策

1. 滚动页内容统一 `AlignTop`；连接带与 UART panel 采用水平 `Expanding`、垂直 `Fixed`，让自然
   高度决定控件几何，剩余空间归 viewport。
2. 继续只保留一个 `MotionController`/`PreciseTimer`。保留 8ms scheduler slot，以
   `TARGET_HZ * interval_ms / 1000 = 0.96` 的 nominal budget 产生平均约 120Hz frame signal；phase
   仍按 elapsed time 前进，Qt 合帧、桌面调度和显示器刷新不被伪装成精确 FPS。
3. `lifecycle_motion.py` 提供可选 `motion_active()` surface predicate。仅在 animated fan-out 中
   对明确静态的 surface 跳过 `set_frame()`；stop/static 广播仍覆盖所有 surface。状态判断留在
   surface owner，避免共享协调器复制业务状态。

## 取舍与不变量

- 选择 0.96 nominal budget 是针对整数 Qt timer slot 的小型 cadence 校准，不创建第二个 timer、
  线程或渲染循环；一个偶发 16ms signal 间隔是预算累积的预期代价，需以真实显示器体验继续观察。
- `MotionController.TARGET_HZ=120`、8ms `PreciseTimer`、reduced-motion、暂停、隐藏/最小化/关闭
  stop/rearm、activity-only gate、phase offsets、业务 binding 和 accessibility 不变。
- 没有 `motion_active()` 的 surface 保留兼容 fallback `True`，避免静态优化改变既有表面语义。

## 验证

- `uv run ruff check src scripts`：pass。
- `uv run python -m compileall -q src`：pass。
- `scripts/check.ps1`：source limit 177 files ≤1000、theme token audit pass。
- 真实组合根 offscreen：980/1240 可见 scroll `horizontal_max=0`，连接带 `138px`，1200ms
  动效样本 `145 frames / 120.83Hz`；隐藏停止 timer、显示后 timer 可恢复；closed/open 与
  `animated=False` 状态投影通过。
- 未运行 EXE 启动、真实显示器/HIDPI/FPS、硬件/HIL、连接/OTA/RTT 和签名验收。
- 本轮 Python/PySide6 presentation-only；embedded C/C++ public vendor source applicability 为
  N/A，不作 MISRA、ISO 26262、ASIL 或认证声明。
