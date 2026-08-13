# ARCH-98 / UI-1.171 交接：共享动作按钮高度契约

日期：2026-08-12

## 变更

修复主题字体与局部固定高度冲突：`ActionRailButton` 在共享动作组件中统一提供 `36px` 最小高度，
命令空态、组件空态和终端空态删除 `setFixedHeight(30)`。相关按钮不再出现
`minimumHeight=36 / maximumHeight=30`，现由一个高内聚 presentation leaf 负责尺寸底线。

## 复核与验证

架构师 `019ff352-98d1-79f2-a457-1dc781315fac` 和独立 reviewer
`019ff355-5852-7742-93e6-43193230a952` 均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS；
父代理完成 correctness、architecture、security、performance、readability 五轴 review 与行为保持
简化评估。`uv run ruff check`、`uv run python -m compileall -q src`、`scripts/check.ps1` 均通过。

真实组合根验证覆盖三主题、四 workspace、980×720：相关动作按钮 `min=36 / max=16777215`，
四页横向 scroll 最大值均为 `0`；共享动效 1 秒离屏采样为 `119Hz`，配置仍为 `TARGET_HZ=120`
和 `8ms` PreciseTimer。PySide6 font directory warning 仅作环境记录；GUI/EXE 启动、真实显示器 FPS、
硬件/HIL、OTA/debug backend 和签名未运行/未授权。

## 打包状态

`local-arch-98` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`。
三者均为 `48,015,845` bytes，SHA-256 为
`A4E63A9E17EC00DECB51ABB4B93EC3A2FABD2D8FB580843BFAD1FFB9C92006AB`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过。签名为
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
