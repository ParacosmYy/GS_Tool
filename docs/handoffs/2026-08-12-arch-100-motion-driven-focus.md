# ARCH-100 / UI-1.173 交接：专注过渡共享帧驱动

日期：2026-08-12

## 变更

专注设置↔总览的高度和 reveal opacity 轨道改由
`presentation/motion_transition.py:MotionDrivenAnimationGroup` 驱动。Qt
`QPropertyAnimation` 只保留起止值与 easing/interpolation contract，不再启动 Qt 默认动画时钟；
有限过渡在唯一 `MotionController.frame_changed` 上按 monotonic elapsed time 更新。这样修复了原
`valueChanged` 约 35Hz 导致的低帧率与中间帧挤压，且未引入第二 timer、线程、业务状态、scroll
owner、设备 I/O 或 OTA/debug coupling。

## 复核与验证

Luna 架构师 `019ff36f-09e1-7a03-a5ac-2f4aa8393791` 与独立 reviewer
`019ff376-5ce1-7ae2-a4b6-aa1faf66cf7c` 均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS；
父代理完成 correctness、architecture、security、performance、readability 五轴 review 与行为保持
简化评估。`uv run ruff check src scripts`、`uv run python -m compileall -q src`、`scripts/check.ps1`
均通过。

离屏真实组合根覆盖专注进入/退出、快速反转、显式 stop、hide、minimize、close、reduced-motion、
pause、resize、三主题、四 workspace 和横向 scroll。220ms transition 最终稳定，活动帧间隔均值约
`9.04ms`，共享 `TARGET_HZ=120` / `8ms` scheduler contract 保持。该数字不是显示器精确 FPS；GUI/EXE
startup、真实显示器/HIDPI、硬件/HIL、OTA/debug 实连和签名验收未运行或未授权。嵌入式 C/C++
public-source applicability 为 N/A。

## 打包状态

`local-arch-100` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`。三者均为 `48,018,575` bytes，SHA-256 为
`F9793EA0A9B9A9C727ABE0E888D4B5598A70DAD23B1D6D8AEDB9451F3A06C255`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
