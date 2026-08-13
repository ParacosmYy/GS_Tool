# ADR 0141：共享动效协调 owner

- 状态：Accepted for ARCH-90 / UI-1.163
- 日期：2026-08-12

## 背景

`controllers/lifecycle.py` 已接近 1000 行，同时包含主题/状态/错误/窗口事件与共享动效 catalog、
可见性缓存、帧 fan-out、activity gate。继续把新的动效 surface 接入同一文件会降低边界清晰度，
也容易触发行数门禁。

## 决策

新增 `controllers/lifecycle_motion.py` 作为共享 presentation motion owner，迁移：

- `_MotionSurfaceVisibilityFilter` 与 motion surface catalog；
- visible snapshot、activity surface stop 和 hidden-edge handling；
- `on_motion_frame()` 的 phase offset/fan-out；
- `set_data_activity_motion()` 的既有状态 projection。

`lifecycle.py` 保留 shell 状态、主题、错误和 Qt show/hide/close/change 事件，并导入上述函数以
保持既有兼容调用。没有修改 `MotionController`、window staged refs、业务回调、transport、
ViewModel、recording/replay 或 protocol binding。

## 依赖与生命周期

新 owner 只向内依赖 bindings、presentation property bridge 和 `workspace_runtime.workspace_motion_enabled`。
`workspace_runtime.py` 仍在 tab-change 函数内 lazy-import `lifecycle.invalidate_motion_surface_snapshot`，
因此 import-time 依赖图没有形成环。原有单一 `MotionController` 继续负责 timer，拆分模块不启动新时钟。

## 结果与验证

`lifecycle.py` 从 939 行降至 662 行，`lifecycle_motion.py` 为 300 行。父代理完成 owner、依赖、
行为保持、可读性/简化和动效资源 fresh-pass；架构师与独立 review 调用均在服务窗口内超时并关闭，
未伪造外部 PASS。Ruff、compileall、工程约束、motion re-export/cadence contract、source-limit、
theme audit 和 provenance verify 通过；未启动 GUI/EXE、未采样显示器 FPS、未执行硬件/HIL 或签名验收。

交付 onefile：canonical、root、root-latest 均为 `48,005,887` bytes，SHA-256
`E3EAE6FCC3CEB46C498EBB3094B70551211C80168BD54D327B4FCE178CD1EF9E`，source revision
`local-arch-90`，archive listing SHA-256 `0FC3A33559DC02B0C14313BE9A76FDE3C0FAA881A5C7861C9DFE146A0307775A`，
签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
