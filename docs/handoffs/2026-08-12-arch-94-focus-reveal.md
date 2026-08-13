# ARCH-94 / UI-1.167 交接归档：工作区 reveal 防拥挤

日期：2026-08-12  
范围：`presentation/workspace_focus_transition.py` 的 presentation-only focus transition。

## 变更

退出专注设置时，既有 Tab/下方 surface `maximumHeight` 动画保持不变；三块下方 surface 额外使用
临时 opacity effect 渐显，避免低高度中间帧绘制被压缩的内部控件。`_clear_focus_transition_effects()`
负责自然完成、停止、反转、静态、隐藏、最小化、resize 和关闭清理，Qt 所有权负责 effect 回收。

没有新增 timer、MotionController、线程、业务状态、scroll owner、设备 I/O、网络、OTA/AES/RTT/J-Link
或 application/domain 依赖；所有修改文件均低于 1000 行。

## 架构与复核

架构师调用 `019ff30a-ce6a-7a53-b7eb-cfd39d0c584d` 在服务窗口内超时并关闭，未形成外部结论；独立
reviewer `019ff311-7fa0-7c71-98cc-89a97522b908` 也超时并关闭。父代理按 correctness、readability、
architecture、security、performance 五轴完成复核，未发现 Required finding；简化评估确认新增逻辑
仍集中在既有 focus transition owner，没有搬移业务复杂度。该轮不涉及嵌入式 C/C++，public-vendor-source
applicability 为 N/A，不作固件或认证声明。

## 验证

- `uv run ruff check src scripts`：通过。
- `uv run python -m compileall -q src`：通过。
- `scripts/check.ps1`：通过，177 files ≤1000，3 themes/22 semantic tokens，legacy QSS literal 0。
- 真实组合根 `QT_QPA_PLATFORM=offscreen`：980×720 进入/退出、mid reveal、settle、快速反转、显式
  stop、hide、close 通过；`mid_effects=3`、settled lower heights `(96, 162, 98)`、effects clean。
- 同一组合根检查唯一 MotionController：`TARGET_HZ=120`、timer interval `8ms`；离屏脚本等待窗口
  callback 计数 `185`，不解释为显示器真实 FPS。
- 未运行：GUI/EXE 启动、真实显示器/HIDPI/读屏、真实 UART/网络/BLE/RTT、OTA/debug 硬件、签名和正式
  release acceptance。

## 交付

本轮 `local-arch-94` onefile 已生成并覆盖 canonical、root、root-latest 三份 artifact；三者均为
`48,009,747` bytes，SHA-256 为
`BE74331C2BFAE11F0DDBD2DC5CC2D89D9581AFD08ACAA125CCE67C1516E283C3`；archive listing SHA-256 为
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过；签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
