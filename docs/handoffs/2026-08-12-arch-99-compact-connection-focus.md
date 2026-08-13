# ARCH-99 / UI-1.172 交接：紧凑窗口连接页自适应专注

日期：2026-08-12

## 变更

980×720 总览模式下，连接配置页曾被实时观测、终端和发送区压缩到约 115px。本轮在
`workspace_runtime.py` 集中管理紧凑高度策略（≤760px）、连接页 focus override 和 resize/tab 同步：
未有用户明确覆盖时，连接页复用现有 220ms focus/reveal 过渡；用户点击“返回总览”后，后续 resize
和 Tab 往返不会抢回。`open_connection_setup` 在切 Tab 前设置 CTA intent，避免相反动画连续启动。

## 复核与验证

Luna 架构师 `019ff35f-fbde-7ce2-ac72-f7ef90d5c962`、Terra 升级架构师
`019ff361-cb49-7fa1-81b2-6f20c6b0337b` 与独立 reviewer `019ff366-a4db-7161-a137-04543c61423f`
均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS；父代理完成 correctness、architecture、
security、performance、readability 五轴 review 与行为保持简化评估。`uv run ruff check src scripts`、
`uv run python -m compileall -q src`、`scripts/check.ps1` 均通过。

真实组合根覆盖：980×720 新窗口、1240×820 宽屏、宽屏缩到紧凑、显式返回总览、Tab 往返、连接 CTA、
三主题、六种传输；紧凑连接页完整 viewport 约 471px，横向 scroll 最大值为 0。1 秒共享动效采样仍
保持 120Hz target/8ms PreciseTimer（实测约 119Hz）。当前 PySide6 font directory warning 仅作环境记录；
GUI/EXE startup、真实显示器 FPS、硬件/HIL、OTA/debug backend 和签名未运行/未授权。

## 打包状态

`local-arch-99` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`。
三者均为 `48,016,245` bytes，SHA-256 为
`3A44AC36190D0A275815D2CB0674B81C86C141F03B1E350EB105B33D9D4AC173`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过。签名为
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
