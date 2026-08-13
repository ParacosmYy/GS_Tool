# ARCH-80 / UI-1.153 顶部 header 响应式密度

日期：2026-08-12  
父代理：Codex；当前 checkout 唯一写入者  
范围：Python/PySide6 presentation-only；未修改 embedded C/C++、BSP/HAL/RTOS、协议固件或设备 I/O。

## 用户问题

最小窗口的顶部控制区虽然没有发生横向溢出，但 status、motion、theme 三组控件旁仍有两枚纯
装饰动效 rail；用户反馈组件与动画视觉密度偏高。UART 八列网格在此前 ARCH-77/UI-1.134 已
通过 bounded selector 和真实组合根验证，本轮不重排其业务字段。

## 架构决策与实现

架构师审阅记录存在两份不同建议：Luna 线程先在等待窗口超时，随后返回“不要新增 helper，
由 `workspace.py` 单 owner 处理”；Terra 升级线程返回“可做，建议独立 `presentation/header_density.py`
helper”。父代理选择较小的 workspace-local 方案，并保留该差异，不把它写成一致通过。

`src/serialforge/presentation/controllers/workspace.py` 新增 `_AdaptiveHeader`，由 header 自身
`resizeEvent()` 调用 `_sync_header_density()`。当 header 的 Qt logical width `<1120` 时：

- 仅隐藏 `SignalFieldWidget(148×34)` 与 `ThemePaletteSwatch(62×22)`；
- controls row spacing 从 8px 收到 6px；
- `statusCluster`、`motionControls`、`themeControls`、主题下拉、低动效/暂停动效、连接状态、
  键盘/Tab/accessibility 和 `HeaderChromeBindings` 全部保留；
- 宽屏恢复两枚装饰件，当前 ThemeSpec 和 shared MotionController 不变。

没有新增 timer、MotionController、业务状态、偏好字段、事件总线、网络、设备 I/O 或 OTA/debug
依赖；不可见装饰件由既有 lifecycle 可见性过滤自然跳过 repaint。

## 验证证据

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
ARCH80_HEADER_DENSITY 980/1119/1120 compact; 1240 regular
ARCH80_THEME_CONTROL_PASS themes=3
ARCH80_LAYOUT_PASS sizes=2 tabs=4 visible_horizontal_max=0
ARCH80_SETTLED_SCREENSHOTS_PASS count=3
ARCH80_MOTION_BOUNDARY target_hz=120 interval_ms=8 timer_type=PreciseTimer frames=29 mean_ms=15.607
ARCH80_PAUSE_PASS timer_active=0
ARCH80_HIDE_SHOW_PASS resume_active=1
ARCH80_ACTIVITY_SURFACES total=28 visible=4
ARCH80_CLOSE_PASS timer_active=0
ARCH80_EXE_STARTUP_PASS pid=6044
ARCH80_EXE_SHUTDOWN_PASS pid=6044
```

离屏 Qt 仍有既有 font-directory warning；实际显示器 FPS、HIDPI、读屏、真实硬件连接/传输、
闪写、部署和 HIL 均未运行；本轮没有操作目标硬件。

## 包交付

```text
source revision: local-arch-80
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 47,987,629 bytes
sha256: CB829EBBF27D9C48E1993040E28E07A57A9092EE3EBDC2A1BF32082A9D30F598
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
signature: NotSigned
release_eligible: false
hardware_acceptance: not_run
canonical/root/root-latest: byte-identical
```

根目录文件：[SerialForge.exe](../../SerialForge.exe)、[SerialForge-latest.exe](../../SerialForge-latest.exe)。

## Embedded applicability

本轮没有 embedded C/C++ 或 firmware source edit；没有适用的 MCU、SDK、RTOS、toolchain 或
public first-party vendor requirement，也没有声称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。
未进行硬件写入、刷写、部署或 target 操作。

## Assurance receipt

- Public source applicability：N/A。没有 firmware/MCU/SDK/RTOS/driver/OTA implementation change，
  因而没有可适用的 manufacturer requirement、vendor URL、revision 或 target scope 可记录。
- Independent review：Luna architect 线程先超时后返回 workspace-local 结论；Terra escalation
  线程返回独立的 helper 备选建议。父代理按 correctness、ownership、lifecycle、accessibility、
  performance、security 五轴复核；两份架构结论存在差异，已如实记录，没有伪造一致通过。
- Simplification assessment：没有嵌入式代码可简化；UI 仅新增 header-local geometry owner，
  未新增 timer、状态源、线程、I/O、动态资源或跨层 abstraction，未做行为改变型压缩。
- Authorized non-destructive validation：Ruff、compileall、source-limit/theme-audit、真实组合根
  三主题/四 Tab/两尺寸边界、可见滚动页、settled screenshots、motion pause/hide/show/close、
  provenance verify、root EXE startup/shutdown；未运行硬件、HIDPI 实机、读屏、连接传输、刷写、
  部署或 HIL 验收。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
