# UI-1.81 历史回放终态 Activity Confirmation 交接

日期：2026-08-10  
工作区：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；本轮唯一写入者  

## 交付结果

PLAYING 继续使用 520ms shared activity。历史回放 EOF、STOPPED、ERROR 到达可见窗口时，新增一次 480ms terminal confirmation；既有回放轨道在
history/error 状态绘制主题化静态终态 marker，用户能更容易确认回放已结束、被停止或失败。

PAUSED/EMPTY、bootstrap hydration、隐藏/最小化、暂停和 reduced-motion 保持静态；回放 DTO、按钮、source badge、历史数据、文案、焦点、Tab 和
accessibility 语义不变。无百分比、无局部 timer、无第二套时钟。

## 修改文件

- `src/serialforge/presentation/controllers/replay.py`
- `src/serialforge/presentation/replay_activity_surface.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0068-replay-terminal-activity.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 评审与简化

```text
产品角色       019febdc-66ec-7d62-a2d2-b7fd81cedd22  called before source edit; wait timed out; closed
架构角色       019febdc-6738-7ab3-853c-fa021d0382b6  called before source edit; wait timed out; closed
UI 设计角色    019febdc-6789-7bb2-8c9c-a8c0b39546c3  called before source edit; wait timed out; closed
开发角色       019febdc-67e2-71a1-b6d7-4719210c61b6  called before source edit; wait timed out; closed
验证角色       019febdc-6832-78f0-b4da-16a752d411f6  called before source edit; wait timed out; closed
打包/流程角色  019febdc-687e-78a1-b37b-dcc9aa7fadc7  called before source edit; wait timed out; closed
独立质量复核   019febdf-5451-77a1-9b3d-d4199c6e6537  called after implementation; wait timed out; closed
```

角色与独立复核没有返回完整报告，未被当作通过。父代理五轴复核确认 controller 只管理 activity request，surface 只读取 projection 绘制静态 marker；
没有新增 DTO、状态源、timer、事件总线、输入面、依赖或常驻资源。简化结论：一个可见 guard helper 加终态 marker 即为最小完整 diff。

嵌入式 C/C++ 适用性：N/A。

## 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src          PASS
UI181_HIDDEN_REPLAY_STATE_PASS state=playing
UI181_HIDDEN_REPLAY_STATE_PASS state=paused
UI181_HIDDEN_REPLAY_STATE_PASS state=eof
UI181_HIDDEN_REPLAY_STATE_PASS state=stopped
UI181_HIDDEN_REPLAY_STATE_PASS state=error
UI181_VISIBLE_ACTIVITY_PASS playing=520 terminal=480
UI181_LIFECYCLE_GUARD_PASS hidden=minimized=closing
UI181_TERMINAL_MARKER_RENDER_PASS theme=star_trail
UI181_TERMINAL_MARKER_RENDER_PASS theme=moonlit_ocean
UI181_TERMINAL_MARKER_RENDER_PASS theme=sakura_night
UI181_SURFACE_VECTOR_PASS terminal_states=history,error themes=3
UI181_REPLAY_TERMINAL_VECTOR_PASS hidden_static=pass visible_probe=pass render=pass
```

验证使用真实 composition root 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；缺失 PySide6 虚拟字体目录的 Qt warning 已记录，未据此宣称真实
Windows 字体/HIDPI 通过。未运行完整 GUI、读屏、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。

## 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.81` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.81`
- size：`47,893,847` bytes
- SHA-256：`DA71E46BE6B626E644F6E907FC75CCE44C8E9C4E174BCA0FC8E6BA9B4B7893D8`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
