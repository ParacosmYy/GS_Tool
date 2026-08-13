# ARCH-78 / UI-1.151 动效层级与 activity fan-out

日期：2026-08-12  
父代理：Codex；当前 checkout 唯一写入者  
范围：Python/PySide6 presentation-only；未修改 embedded C/C++、BSP/HAL/RTOS、协议固件或设备 I/O。

## 目标与边界

用户反馈动画挤在一起、组件视觉拥挤。本轮保持单一 `MotionController` 和约 120Hz 的 8ms
`PreciseTimer` scheduler target，把共享 surface 分为两级：ambient 环境装饰持续运行，
action rail 与空态 composite 只在短时 activity/transition 期间运行。8ms 不宣称显示器精确
120fps；实际调度仍由 Qt/Windows/compositor 决定。

## 实现

- `MotionController.activity_active()` 只读输出非 ambient 活动窗口；不创建第二个 timer、不读取业务状态。
- `ActionRailButton`、`BusyActionButton`、`TerminalEmptyState`、`CommandBatchEmptyState` 和
  `ComponentEmptyStateSurface` 通过 `MOTION_MODE="activity"` 声明 activity-only。
- `controllers/lifecycle.py` 是唯一 fan-out/cleanup owner：activity true→false 下降沿调用一次
  stop，隐藏 activity surface 在活动期间只 stop 一次，重新可见才恢复 frame；hide/minimize/
  pause/reduced-motion/close 继续全量 stop。
- `AnalysisStatusLabel` 未标记 activity-only，避免把持续语义状态误判为短时反馈。

## 架构师审阅与简化评估

架构师前置审阅批准上述 owner 边界：时钟负责 activity 判定，lifecycle 负责 fan-out/cleanup，
widget 只渲染，按钮不自行 request activity。post-implementation 只读 review 结论为
`PASS / Required=0 / Optional=2`，确认 activity gate、隐藏一次性 stop、single motion timer 与
pause/hide/minimize/close fence。实现没有引入全局 surface registry、widget timer、业务状态或
跨层 callback。

父代理五轴 review 记录：正确性覆盖 activity gate、下降沿、隐藏恢复和生命周期；可维护性通过
marker + 三个 lifecycle helper 收敛重复可见性/stop 分支；架构边界保持 presentation-only；
安全面没有新增输入、网络、密钥、文件或设备操作；性能面从默认 ambient fan-out 中移除 28 个
activity-only surface。行为保持型简化已完成：复用现有 `set_frame/stop` contract 和既有
`request_activity()` 调用点，没有新增抽象。进一步把持续状态 surface 改为 activity-only 被
刻意推迟，因为其 `state/source` 是持久语义而非短时动作。

两轮外部 Luna/max 独立审查线程均因服务侧超时未返回，未伪造独立通过；该限制已记录，父代理
复核作为 fallback，不把它写成外部 reviewer approval。

## 非破坏性验证

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
MOTION_ACTIVITY_SURFACES total=28 visible=4
MOTION_CLOCK_TIMERS 1
MOTION_ACTIVITY_ON visible_animated=4
MOTION_ACTIVITY_OFF all_activity_static=1
MOTION_HIDDEN_ACTIVITY_PASS hidden=24
MOTION_ROUTE_ACTIVITY_PASS visible=7 animated=6
ARCH78_THEME_LAYOUT_PASS themes=3 tabs=4 visible_horizontal_overflow=0
ARCH78_LAYOUT_PASS sizes=2 horizontal_overflow=0
MOTION_PAUSE_RESUME_PASS
MOTION_HIDE_SHOW_PASS
MOTION_CLOSE_PASS
ARCH78_MOTION_CONFIG target_hz=120 interval_ms=8 timers=1
ARCH78_MOTION_CONFIG_PASS
```

离屏验证保留既有 Qt font-directory warning；它来自当前 `.venv` 的字体目录，不代表 Windows
系统字体缺失。实际显示器 FPS、真实 GUI 启动、硬件连接/传输、闪写、部署和 HIL 均未运行，
原因是本轮仅授权本地非破坏性 presentation 验证。

## 包交付

`scripts/package.ps1 -Mode onefile -SourceRevision local-arch-78` 已完成，canonical、根目录和
root-latest 字节一致：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 47,986,125 bytes
sha256: BA907F2587004E2FE797B0AC6397B286AF034929DD05C888964C27BC3B4EC8B1
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; signature=NotSigned; release_eligible=false; hardware_acceptance=not_run
```

根目录文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。

## Embedded applicability

本轮没有 embedded C/C++ 或 firmware source edit；因此没有适用的 MCU、SDK、RTOS、toolchain
或 public first-party vendor requirement，也没有声称 MISRA、ISO 26262、ASIL、ASPICE 或认证
合规。`$embedded-enterprise-workflow` 与 `$embedded-code-review-simplifier` 的 review/source
gate 对本轮标记为 N/A；未进行硬件写入、刷写、部署或 target 操作。
