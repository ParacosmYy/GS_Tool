# UI-1.76 快速配置选择 Activity Rail

日期：2026-08-10  
范围：快速配置选择/清空动作的共享 activity rail 反馈。  
父代理：Codex；父代理是本轮唯一写入者。  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 Git/Codex worktree。

## 用户结果

- 选择内置或自定义快速配置后，连接带中的 `ConnectionPresetContextSurface` rail 会短暂扫过一次，确认选择事件已经被界面接收。
- 清空快速配置也走同一视觉反馈；反馈不代表连接成功，不自动连接，不保存配置。
- 420ms activity pulse 复用唯一 `MotionController`，低动效/暂停/隐藏/最小化/关闭保持静态；combo value、apply callback、tooltip/
  AccessibleDescription、焦点、Tab 和 popup 语义保持不变。

## 架构边界

`controllers/connection_builder.py:_dispatch_connection_preset()` 是选择事件 owner，只在原有 projection/apply 完成后调用
`window._motion_controller.request_activity(420)`。`ConnectionPresetContextSurface` 只负责 bounded painting 和既有 `set_frame/stop`，
不读取 ViewModel、不解释 SessionState、不创建 timer、不持有业务状态。catalog hydration 的 blocked signal 不触发反馈。

## 实际修改

- `src/serialforge/presentation/controllers/connection_builder.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0063-preset-activity-rail.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 角色调用与独立质量复核

```text
产品角色       019febb3-6c48-7561-afd1-3a55aa12cf80  called before source edit; wait timed out; closed
架构角色       019febb3-6c98-7423-9ae1-61eb509607db  called before source edit; wait timed out; closed
UI 设计角色    019febb3-6ce3-7b80-8208-8c3e2790390c  called before source edit; wait timed out; closed
开发角色       019febb3-6d30-7853-94dc-a561a9c720b8  called before source edit; wait timed out; closed
验证角色       019febb3-6d86-7f31-9264-658692d09200  called before source edit; wait timed out; closed
打包/流程角色  019febb3-6dcf-79b3-ba09-d298ae2b5c1a  called before source edit; wait timed out; closed
独立质量复核   019febb5-25aa-7640-9acd-9d66d3f0157c  called after implementation; wait timed out; closed
```

六角色与独立复核均未返回完整报告，未把超时当成通过。父代理完成 correctness/readability/simplicity/architecture/security/performance
五轴审查；简化结论是复用既有 `request_activity()` 与 context rail，不引入新 timer、业务字段、selection event bus 或独立动画模块。

## 验证

```text
pwsh.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS
.venv\Scripts\python.exe -m compileall -q src  PASS
UI176_PRESET_ACTIVITY_PASS theme=star_trail frames=2 clear=pass
UI176_PRESET_ACTIVITY_PASS theme=moonlit_ocean frames=3 clear=pass
UI176_PRESET_ACTIVITY_PASS theme=sakura_night frames=3 clear=pass
UI176_REDUCED_MOTION_PASS activity=static
UI176_PAUSED_MOTION_PASS activity=static
```

vector 使用真实组合根的短时 Qt offscreen 实例；环境提示缺失 PySide6 虚拟字体目录，未据此宣称 Windows 字体/HIDPI/真实窗口通过。
完整 GUI/HIDPI/读屏/视觉帧差分、EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包状态

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.76` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.76`
- size：`47,892,668` bytes
- SHA-256：`F726E12FD00E0718FC3CBA66883CC783018244184A6F54CA493C81FAE9508593`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
