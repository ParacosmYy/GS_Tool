# ARCH-101 / UI-1.174 交接：一次性过渡共享帧时钟

日期：2026-08-12

## 变更

page fade、theme root fade+sweep、transport panel fade、两个自定义 dialog fade 与 focus transition
均改用各自的 `MotionDrivenAnimationGroup` finite owner，共用窗口级唯一 `MotionController`。
内部 `QPropertyAnimation` 只提供 easing/interpolation，不能独立 `start()`；theme sweep 与 root fade
在同一 group 中运行，消除了双时钟。driver 对失效 target 做 `isValid()` guard，并 stop/emit finished
交给各 owner 清理 effect/overlay。

## 复核与验证

ARCH-101 架构师 `019ff382-2d72-7d32-a067-09b13c9a32d5` 建议“每功能一个 finite owner、全窗口一个
共享时钟”；独立 reviewer `019ff389-959a-77b0-8f26-1e444ee50ac6` 超时关闭，未形成外部结论，未伪造
PASS。父代理完成 correctness/architecture/security/performance/readability 五轴 review 与行为保持
简化评估；Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

Ruff、compileall、`scripts/check.ps1` 通过（179 files ≤1000、3 themes/22 semantic tokens）。真实
composition root offscreen 连续活动窗口均值：page `8.13–8.95ms`、theme `9.40–9.89ms`、transport
`8.21–9.05ms`、dialog `8.21–8.72ms`、focus `8.67ms`；3 themes、4 workspaces、6 transports、
dialog hide、theme overlay cleanup、focus/window hide 通过。未运行真实显示器/HIDPI/FPS、高负载 GUI、
EXE startup、硬件/HIL、连接/OTA/RTT 实连和签名验收。

## 打包状态

`local-arch-101` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`。
三者均为 `48,019,302` bytes，SHA-256 为
`CAABF434DE8A04B49249775B36C897F1AA73B71CAAAD3CE5A57F070DB1C8B558`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
