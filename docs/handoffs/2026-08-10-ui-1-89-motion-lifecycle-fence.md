# UI-1.89 共享动效帧生命周期门禁

日期：2026-08-10  
范围：修复 Qt 事件队列中的过期 `MotionController.frame_changed` 帧在隐藏、最小化、低动效或关闭边界回流到展示层的问题。

## 结果

- `controllers/lifecycle.py:on_motion_frame()` 现在统一检查 `_closing`、`isVisible()`、`isMinimized()` 和 `workspace_motion_enabled()`。
- 阻断态调用既有 `_stop_motion_surfaces()`，并清除 data-activity presentation property；不改变 SessionViewModel、transport、recorder、parser、send queue 或业务事实。
- 正常可见且允许动效的帧继续按原路径 fan-out 到共享 consumers；没有新增 timer、状态源、事件总线或设备依赖。
- 开发约束、架构说明、任务清单、README、ADR 0079 与最新交接入口已同步。

## 角色与独立复核

```text
产品角色       019fec34-01a7-7de0-bef1-242485dba41e  called; wait timed out; closed
架构角色       019fec34-01f8-7ea1-8736-50adaf44a20c  called; wait timed out; closed
UI 设计角色    019fec34-024d-7962-9d7f-5edf92a55424  called; wait timed out; closed
开发角色       019fec34-02a0-71c0-b757-6e8fd3aa8fe1  called; wait timed out; closed
验证角色       019fec34-02f3-72f1-94c2-fa20638796c1  called; wait timed out; closed
打包角色       019fec34-033a-7011-b000-3f6d5b1367f2  called; wait timed out; closed
独立质量复核   019fec34-9627-7fb1-9ded-fd836d94d024  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认四个生命周期门禁和正常 fan-out；readability/simplicity 确认复用现有 workspace motion policy 与 stop fan-out；architecture 确认 lifecycle 继续拥有 Qt 可见性和 consumer 清理、MotionController 继续拥有时钟；security 确认无输入、I/O、网络、密钥或依赖变化；performance 确认只增加低成本状态判断，不增加 timer/线程/事件总线。

简化评估结论：将过期帧拦截放在现有 lifecycle 唯一分发点是最小完整实现，不新增第二个 MotionController 或分散到每个 widget 的可见性判断。
嵌入式 C/C++ 适用性：N/A；本轮只修改 Python/PySide6 presentation lifecycle。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
MOTION_LIFECYCLE_VECTOR_PASS blocked=4 allowed_frames=1
```

向量使用 Qt offscreen 与内存对象，没有显示主窗口；覆盖不可见、最小化、低动效、关闭四种阻断态和可见/允许动效正常态。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实 UART/网络/BLE/RTT、OTA、硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.89` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.89
size: 47,896,526 bytes
SHA-256: 8D18CF6A28BA1EF0C3062ABDB097AA3F5A12D764B930FBA1F208F131FF8BE565
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
