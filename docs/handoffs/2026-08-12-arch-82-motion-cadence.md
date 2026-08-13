# ARCH-82 / UI-1.155 动效快照与恢复调度

日期：2026-08-12  
父代理：Codex   
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
写入策略：父代理是唯一写入者；未创建或操作 Git/Codex worktree；未创建、修改或运行单元测试资产。

## 目标与边界

本轮针对“动画挤在一起、帧数低、窗口恢复后 timer cadence 退化”做 presentation-only 优化：

- `lifecycle.py` 将动态扫描改成完整 motion surface catalog + 可见 snapshot；
- `workspace_runtime.py` 在 workspace route 变化时使 snapshot 失效；
- `MotionController` 保持唯一 shared clock、`TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target；
- host hide/minimize/show/restore 用 `rearm_required` 和两跳 queued lifecycle fence 重建 controller-owned timer；
- 不改 domain/application/infrastructure、UART/网络/BLE 连接语义、设备 I/O、OTA/AES contract-only、RTT/J-Link attach-only 边界。

## 实现证据

`src/serialforge/presentation/controllers/lifecycle.py`：

- `_motion_surface_catalog` 只初始化一次，共 56 项；
- `_motion_surface_snapshot` 只在 Show/Hide/ParentChange、resize、route、主题和 window-state 边界失效后重建；
- `on_motion_frame()` 消费 `(catalog_index, widget)` 快照，完整 catalog index 保证 phase offset 稳定；
- 隐藏 activity-only surface 只在快照边界 stop，不在每个 ambient frame 重复 stop；
- show/restore 使用 `_queue_motion_rearm()` 的固定两跳 `QTimer.singleShot(0, ...)`，回调有 closing/visible/minimized fence。

`src/serialforge/presentation/widgets.py`：

- `MotionController.rearm_after_show()` 停止、断开、`deleteLater()` 旧 timer，再建立同配置的唯一 active `PreciseTimer`；
- `_rearm_required` 仅由 host visibility suspend 设置，成功 rearm 后清零；
- phase、activity deadline、motion-enabled、paused、suspended、transition、ambient 和 `frame_changed` fan-out 保持；
- pause/reduced-motion 的既有 policy gate 仍阻止 timer 启动，恢复回调不强制绕过。

## 验证证据

已授权且非破坏性执行：

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
  source line limit: 167 files <= 1000
  theme audit: 3 themes / 22 tokens / 19 selectors / legacy_qss_literals=0
```

真实组合根 offscreen 矩阵：

```text
catalog=56; themes=3; tabs=4
ARCH82_IDLE_980:     frames=100 mean=8.001ms p50=8.043ms p95=8.981ms max=10.608ms
ARCH82_ACTIVITY_980: frames=81  mean=7.998ms p50=8.020ms p95=8.963ms max=9.742ms
ARCH82_ROUTE_THEME_PASS: cases=24; snapshot_counts=[12,25,12,6,...,14,27,14,8]; horizontal_overflow=0
ARCH82_FOCUS_PASS: focus_snapshot=8; overview_snapshot=18
ARCH82_PAUSE_PASS: timer_active=False; frames_after_pause=0
ARCH82_REDUCED_PASS: timer_active=False; frames_reduced=0
ARCH82_HIDE_PASS: timer_active=False; frames_hidden=0; rearm_required=True
ARCH82_MINIMIZE_PASS: state=True; timer_active=False; frames_minimized=0; rearm_required=True
ARCH82_CLOSE_PASS: timer_active=False; closed=True
```

可见性计数器验证：稳定 idle/activity frame fan-out 期间没有增加 `isVisibleTo()` 查询；查询只在 route、
theme、resize、visibility/state 边界重建 snapshot。恢复路径在当前 offscreen/Qt 环境约
`13.5ms mean / 15.2ms p50 / 15.9ms p95`，直接 raw `PreciseTimer` 对照也表现相同，属于当前平台/合成
事件粒度，不能解释为真实显示器刷新率。未引入 sleep、`processEvents()`、deadline 或自适应重试。

截图（系统临时目录，未进入仓库）：

```text
C:\Users\Gs\AppData\Local\Temp\serialforge_arch82_980.png
C:\Users\Gs\AppData\Local\Temp\serialforge_arch82_1240.png
```

视觉检查确认两种尺寸无白色横条、横向溢出或字段重叠；offscreen 环境的 Qt font-directory warning 会令
中文临时显示为方框，不代表 Windows 系统字体运行时适配失效。

## 架构审查、简化与 assurance

架构师：Luna/max。先批准 catalog + visible snapshot，随后根据恢复实测批准最小
`rearm_required` 状态机与固定两跳 queued defer；明确禁止新 controller、第二个常驻时钟、sleep、
`processEvents()` 和无界重试。

独立审查尝试：ARCH-82 之后启动了三条只读子代理审查（Luna/max、Terra/max、窄范围 Luna/max），均在
服务等待窗口内未返回结论，已安全关闭；没有把超时写成外部 PASS，也没有让子代理修改 checkout。父代理按
`code-review-and-quality` 五轴和 `embedded-code-review-simplifier` 的生命周期/资源/简化清单做了 fresh-pass：
没有发现 Required 级的重复 active timer、close fence 穿透、pause/reduced-motion 绕过或状态丢失；该结论是
父代理复核，不替代未返回的外部 reviewer。

简化评估：将每帧可见性判断收敛为可失效 snapshot，减少 hot path 查询；保留完整 catalog index 以避免相位
漂移；用 `rearm_required` 替代每次 show 无条件重建；两跳 defer 是有界生命周期 barrier，不是常驻时钟。没有
删除生命周期 fence、错误处理、diagnostic signal 或可访问性语义。

公共厂商来源适用性：N/A。本轮为 Python/PySide6 presentation-only 代码，没有 MCU、C/C++、BSP/HAL/CMSIS、
RTOS、ISR/DMA、driver、bootloader、Flash/NVM、power 或 motor-control 变更；没有适用的 public first-party
vendor requirement，也没有声称 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。

未运行/未授权：真实显示器 120Hz/HIDPI 与读屏实机、真实串口/网络/BLE/RTT、J-Link、OTA、刷写、部署、HIL、
硬件验收和单元测试均未运行；本轮没有操作目标硬件。

## 打包与根目录覆盖

执行：

```text
scripts/package.ps1 -Mode onefile -SourceRevision local-arch-82
```

产物与校验：

```text
canonical: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root:      SerialForge.exe
latest:    SerialForge-latest.exe
size:      47,990,523 bytes (all three)
sha256:    FC404D7D7D796F9B697811ED1434CC85F0A63668E5E8E06BA20D444C27524B7C (all three)
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance revision: local-arch-82
Python/PyInstaller: 3.12.13 / 6.22.0
signature: NotSigned
release_eligible: false
hardware_acceptance: not_run
```

根目录 EXE 启停烟测：`ARCH82_EXE_STARTUP_PASS pid=6012`、`ARCH82_EXE_SHUTDOWN_PASS pid=6012`；已核对进程
路径为 `D:\Workplace\Agent_Workplace\SerialForge\SerialForge.exe`。旧 root EXE PID 48580 也已在覆盖前按
精确路径安全停止。

