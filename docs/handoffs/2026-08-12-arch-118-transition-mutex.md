# ARCH-118 / UI-1.191 Shell transition 互斥交接

日期：2026-08-12  
范围：`transition_coordinator.py` 与四类一次性 shell transition 的互斥/生命周期边界。

## 结果

主题、workspace、focus、transport 四类一次性过渡现在共享窗口级互斥协调器：新过渡启动前会停止其他
过渡；无效目标、低动效、隐藏/最小化、resize/hide/close 和重复 stop 都走幂等清理。原有动画 owner 仍负责
effect 与完成回调，不新增时钟；用户可见的动效不再同时叠加。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`TRANSITION_MUTEX_MATRIX_PASS=pass`（75 checks，0 failures）；`MOTION_SCHEDULER_SAMPLE=pass`
（142 callbacks / 1.2s，约 118.33Hz）；`package=pass`；`root-exe=pass`；`root-latest=pass`；
`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。

已知 offscreen PySide6 font-directory warning 只影响截图字形；调度样本不等同显示器 120fps。未新增或运行
unit test、mock、fixture、harness 或 test-only 资产。

## 架构、审查与简化记录

架构师 `019ff4e7-5a7d-7341-8dce-e14395627a4e` 要求新增 coordinator；`019ff4e9-d12d-7f50-af4f-35558bc91b89`
确认入口调用链；`019ff4f0-2690-7fe1-92fe-e247d76b774e` 批准早退全量清理。独立 reviewer
`019ff4f2-615d-7843-8b37-82353dd6b266` 两次等待超时后关闭，未形成外部 findings；父代理完成六轴 review 与
行为保持 simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 交付产物

使用 `local-arch-118` 构建 onefile，并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical、
root、root-latest 均为 `48,033,518` bytes，SHA-256 为
`83A3EFCB92D13D45C234C5D17D213B8AF22953DA2C8FA267F674EE250683B00F`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
