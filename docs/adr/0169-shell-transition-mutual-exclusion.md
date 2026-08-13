# ADR-0169：Shell 一次性过渡互斥

## 状态

已接受（ARCH-118 / UI-1.191，2026-08-12）

## 背景

真实 Qt offscreen 快速交互证明：主题切换后立即切换 workspace tab 时，theme 与 workspace 两个
`MotionDrivenAnimationGroup` 可同时存在；focus 与 transport 也有独立入口。多个 opacity/geometry effect
共享同一个 120Hz 帧源，会造成动效叠加、视觉拥挤和生命周期清理难以审计。

## 决策

新增无状态 `transition_coordinator.py`：

1. 用 `ShellTransitionKind` 明确 theme/workspace/focus/transport 四类槽位；
2. 每个有效启动入口先 `prepare_shell_transition(window, kind)`，停止竞争 owner，再由原 owner 创建和登记
   自己的动画；
3. 无效目标、隐藏/最小化、低动效、resize/hide/minimize/close 使用 `stop_shell_transitions(window)` 全量清理；
4. 通过延迟导入避免循环依赖，协调器不接管 effect、布局、业务、滚动或完成回调；
5. 继续复用唯一 `MotionController`，不引入新 timer 或显示帧承诺。

## 被否决的替代方案

- 只在 theme 与 workspace 两个入口互相 stop：transport/focus 仍可绕过，无法形成窗口级不变量；
- 在每个 owner 复制四类 stop 列表：容易随新入口漂移，且生命周期边界重复；
- 新建第二个动画时钟或用 geometry animation 补偿：会增加调度竞争，不能解决互斥根因。

## 验证与审查

架构师 `019ff4e7-5a7d-7341-8dce-e14395627a4e` 给出 REVISE 结论并批准 coordinator owner、六文件边界；
架构师 `019ff4e9-d12d-7f50-af4f-35558bc91b89` 确认入口调用链可复用现有 stop owner；架构师
`019ff4f0-2690-7fe1-92fe-e247d76b774e` 批准早退全量清理修正。独立 reviewer
`019ff4f2-615d-7843-8b37-82353dd6b266` 两次等待超时后关闭，未形成外部结论；父代理完成 correctness、
architecture、lifecycle/accessibility、performance、security、readability 六轴审查与 behavior-preserving
simplification assessment。无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

`TRANSITION_MUTEX_MATRIX` 75 checks、0 failures；compileall、Ruff、项目静态检查通过。真实 GUI/HIDPI、EXE
startup、硬件、签名和 OTA/RTT 实连未运行。`local-arch-118` onefile 已覆盖根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`；三者均为 `48,033,518` bytes，SHA-256 为
`83A3EFCB92D13D45C234C5D17D213B8AF22953DA2C8FA267F674EE250683B00F`，archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
