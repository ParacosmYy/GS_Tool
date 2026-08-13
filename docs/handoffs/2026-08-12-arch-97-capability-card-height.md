# ARCH-97 / UI-1.170 交接：扩展能力卡内容安全高度

日期：2026-08-12

## 变更

修复 `extensionCapabilityCard` 的主题层最小高度优先级回归。能力卡 builder 原本声明 `122px`，
但通用按钮 QSS 使其实际约 `83px`；现在专属 extension selector 明确保留 `min-height: 122px`。
三主题下实际最小高度为 `144px`，扩展页继续使用既有外层 scroll，不引入第二滚动 owner。

卡片选择、焦点、Enter/空格、详情 projection、无障碍文案、只读能力 DTO、OTA/AES/RTT/J-Link
contract-only/attach-only 和 120Hz/8ms MotionController 均未改变。

## 复核与验证

架构师 `019ff342-7abf-7e22-bc8e-40816e7aeef3` 与独立 reviewer
`019ff346-ded8-70b2-9843-fb9845b50952` 均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。
父代理完成 correctness、architecture、security、performance、readability 五轴 review 与行为
保持简化评估。

`ARCH97_CARD_PASS` 覆盖三主题、980/1240、7 张卡、实际高度和横向滚动；`uv run ruff check src scripts`、
`uv run python -m compileall -q src`、`scripts/check.ps1` 通过。offscreen 的 PySide6 font directory
warning 仍为环境记录；GUI/EXE 启动、真实显示器 FPS、硬件验收未运行。

## 打包状态

源码、静态、离屏验证和打包完成；`local-arch-97` onefile 已覆盖 canonical、根目录
`SerialForge.exe` 与 `SerialForge-latest.exe`。三者均为 `48,015,901` bytes，SHA-256 为
`5599524451084AF460D267FDAF87049C83F94261CD89EAE940769F2983484BE6`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
