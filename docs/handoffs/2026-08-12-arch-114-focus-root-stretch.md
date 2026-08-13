# ARCH-114 / UI-1.187 主题工作区根布局伸缩交接

日期：2026-08-12  
范围：`workspace_focus_transition.py` 的 focus/overview 根布局 owner、终端 slot 快照恢复与共享动效边界。

## 结果

focus 模式现在明确把根布局可用高度交给 themed workspace shell：root stretch 为 `shell=1, terminal=0`；
terminal slot、实时观测和发送区隐藏，不再留下白色占位或把配置组件挤到一起。shell 内 tabs 是唯一
可伸缩内容 owner，route strip 保持 31px，长页面继续使用原生 QScrollArea 滚动。

overview 通过 `_FocusLayoutSnapshot` 精确恢复进入前的 root/shell/tabs/route stretch、size policy、
min/max、terminal parent/child hidden、鼠标透明和终端可见性。几何切换即时完成，opacity 过渡仍只复用
唯一共享 MotionController，120Hz target 不变。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`FOCUS_MATRIX_PASS=pass`（三主题×980/1180/1240×四 workspace，36 cases，72 checks，0 failures）；
`LIFECYCLE_MATRIX_PASS=pass`（118 checks，0 failures）；`MOTION_120HZ_SAMPLE=pass`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。

静态检查输出：`scripts/check.ps1`、`python -m compileall -q src`、`ruff check src` 均通过；Qt offscreen
环境有 PySide6 font-directory warning，但不影响本轮 geometry/visibility 断言。该 warning 不代表真实
Windows 字体部署已验收。

## 架构、审查与简化记录

架构师 `019ff489-8b27-77a2-b059-bc160805c0fb` 的正式结论为 conditional pass，要求保存 child hidden、
统一恢复顺序、shell root stretch owner、native scroll 与唯一 MotionController；该要求已落地。架构师
`019ff497-1040-7721-8aa4-865f176e9e1d` 复核 `_set_terminal_spacer` 的 bool 返回契约，建议已落地。
独立 reviewer `019ff499-0005-76b3-9e7c-e042c2777518` 本轮在等待窗口内超时关闭，未形成外部 findings，
没有把超时记录成通过。父代理完成五轴 review 与 behavior-preserving simplification assessment。

本轮没有嵌入式 C/C++、MCU、RTOS、ISR/DMA、驱动、OTA 固件或安全升级实现；public-vendor-source
applicability 为 N/A，不作 MISRA/ISO 26262/认证合规声明。

## 交付产物

`local-arch-114` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。
三者均为 `48,026,927` bytes，SHA-256 为
`966C16A74D04454CE8780F424E19D99773E3E39B1CBE42A0A5490812CB605F2C`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，manifest `release_eligible=false`，`hardware_acceptance=not_run`。
