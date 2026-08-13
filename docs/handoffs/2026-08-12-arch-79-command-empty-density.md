# ARCH-79 / UI-1.152 命令空态密度与调度边界

日期：2026-08-12  
父代理：Codex；当前 checkout 唯一写入者  
范围：Python/PySide6 presentation-only；未修改 embedded C/C++、BSP/HAL/RTOS、协议固件或设备 I/O。

## 问题

真实组合根截图显示：命令管理页在没有批量命令时，`CommandBatchEmptyState` 从约 123px 的
size hint 被拉伸到约 451px，内容和 CTA 被稀释在大面积空白中。该问题属于纵向组合，不是
命令状态或空态组件本身的业务问题。

## 实现与 owner 边界

在 `src/serialforge/presentation/controllers/terminal.py` 的 `build_send_panel()` 中，空态
widget 后增加一个 `root.addStretch(1)`。剩余 viewport 高度现在由 trailing spacer 吸收，空态
按自身 size hint/minimum height 呈现；没有修改 `CommandBatchEmptyState` 的 state、signal、
CTA、accessible metadata、MOTION_MODE、batch bindings、滚动容器或业务回调。

架构师结论：该 trailing stretch 是当前最小且合适的 owner 修复；若未来需要垂直居中，应单独
设计对称 stretch，不在本轮扩大范围。调度器不改：`TARGET_HZ=120`、`PreciseTimer`、8ms
继续作为 scheduler target；Windows/offscreen 约 15.6ms 回调是当前平台观测边界，不能包装成
精确 120fps。改 deadline/0ms/adaptive 会增加 CPU、timer storm 和生命周期复杂度，暂不采用。

## 验证证据

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
ARCH79_COMMAND_GEOMETRY size=1240x820 empty=(14,106,1176,108) sizeHint=123
ARCH79_COMMAND_GEOMETRY size=980x720 empty=(14,106,916,108) sizeHint=123
ARCH79_COMMAND_LAYOUT_PASS
ARCH79_THEME_LAYOUT_PASS themes=3 sizes=2 tabs=4 visible_horizontal_overflow=0 command_empty_height=108
ARCH79_ACTIVITY_PASS total=28 visible=4 single_motion_timer=1
ARCH79_LIFECYCLE_PASS pause=1 hide_show=1 close=1
ARCH79_MOTION_BOUNDARY_PASS target_hz=120 interval_ms=8 timer_type=PreciseTimer frames=46 observed_mean_ms=15.748 observed_min_ms=13.843 observed_max_ms=18.676
ARCH79_EXE_STARTUP_PASS
ARCH79_EXE_SHUTDOWN_PASS
```

离屏 Qt 仍有既有 font-directory warning；它来自当前 `.venv` 的字体目录。实际显示器 FPS、硬件
连接/传输、闪写、部署和 HIL 均未运行；根目录 EXE 已在 Windows 上启动 3 秒并由验证脚本安全结束，
未操作硬件。

## 包交付

`scripts/package.ps1 -Mode onefile -SourceRevision local-arch-79` 已完成，canonical、根目录和
root-latest 字节一致：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 47,984,076 bytes
sha256: 085B1BEF964C3E2861183950E28689DAC798E853CA8A2C38E26A279144C324D9
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; signature=NotSigned; release_eligible=false; hardware_acceptance=not_run
```

根目录文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。

## Embedded applicability

本轮没有 embedded C/C++ 或 firmware source edit；没有适用的 MCU、SDK、RTOS、toolchain 或
public first-party vendor requirement，也没有声称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。
`$embedded-enterprise-workflow` 与 `$embedded-code-review-simplifier` 对本轮标记为 N/A；未进行
硬件写入、刷写、部署或 target 操作。
