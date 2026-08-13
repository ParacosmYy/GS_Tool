# UI-1.74 自定义对话框入口淡入

日期：2026-08-10  
范围：批量命令编辑与自定义连接 preset 对话框的一次性主题入口过渡。  
父代理：Codex；父代理是本轮唯一写入者。  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 Git/Codex worktree。

## 用户结果

- `CommandBatchEditorDialog` 与 `ConnectionPresetEditorDialog` 显示时使用 150ms、`0.90 -> 1.0`、`OutCubic` opacity 淡入。
- 正常动效显示一次过渡；低动效环境与显式暂停静态回退；hide/close 会停止动画、恢复 opacity 并解除临时 graphics effect。
- 已有 graphics effect 不会被覆盖；draft、校验、按钮 action、popup、焦点、Tab/accessibility、尺寸以及 native
  `QMessageBox`/`QFileDialog` 行为保持不变。

## 架构边界

`src/serialforge/presentation/dialog_transition.py` 是自定义 dialog 入口过渡的唯一 owner。它只消费父窗口的
`decorative_motion_enabled()`，把 animation/effect 作为 dialog 的临时 presentation 状态；不读取 ViewModel、不接触
draft/validation/transport/protocol/security 状态，不创建常驻 timer、不使用第二套 MotionController，也不改变业务 action。
两个 dialog 只通过 show/hide virtual hook 调用 helper；native dialog 不接入该路径。

## 实际修改

- `src/serialforge/presentation/dialog_transition.py`
- `src/serialforge/presentation/command_batch_editor.py`
- `src/serialforge/presentation/connection_preset_editor.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0061-dialog-entrance-transition.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 角色调用与独立质量复核

```text
产品角色       019feba1-478d-7873-baf5-b0a740a4f3e0  called before source edit; wait timed out; closed
架构角色       019feba1-47d6-7e50-9ea3-a64186215d79  called before source edit; wait timed out; closed
UI 设计角色    019feba1-481e-7300-af1a-93d002ecc927  called before source edit; wait timed out; closed
开发角色       019feba1-4870-7022-8a58-94dafb91de85  called before source edit; wait timed out; closed
验证角色       019feba1-48ba-78f2-b72a-0965e2005731  called before source edit; wait timed out; closed
打包/流程角色  019feba1-490b-7022-8a58-94dafb91de85  called before source edit; wait timed out; closed
独立质量复核   019feba2-e2bb-7223-a7f4-5a4c3755d454  called after implementation; wait timed out; closed
```

六角色与独立复核均在运行时窗口内超时，未把超时当成通过。父代理完成五轴审查：

- correctness：show/hide 与 effect/animation 生命周期配对，重复 show 不覆盖已有 effect。
- readability/simplicity：独立模块保持单一职责，两个 dialog 只增加明确 virtual hook，不复制动画实现。
- architecture：模块只位于 presentation，复用现有 motion policy，不向 ViewModel、application 或 domain 反向依赖。
- security：没有输入、存储、网络、密钥或依赖变化。
- performance：每次显示最多运行一段 150ms 动画，不增加常驻 timer、循环或业务热路径状态。

简化评估结论：复用既有 `decorative_motion_enabled()` 和一次性 `QPropertyAnimation` 是最小完整实现；不引入全局 dialog
基类、第二套 motion clock、常驻 timer、业务 transition state 或 native dialog 替换。

## 验证

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI174_DIALOG_TRANSITION_PASS theme=star_trail dialog=command animation=active
UI174_DIALOG_TRANSITION_PASS theme=star_trail dialog=preset animation=active
UI174_DIALOG_TRANSITION_PASS theme=moonlit_ocean dialog=command animation=active
UI174_DIALOG_TRANSITION_PASS theme=moonlit_ocean dialog=preset animation=active
UI174_DIALOG_TRANSITION_PASS theme=sakura_night dialog=command animation=active
UI174_DIALOG_TRANSITION_PASS theme=sakura_night dialog=preset animation=active
UI174_REDUCED_MOTION_PASS dialog=command animation=static
UI174_REDUCED_MOTION_PASS dialog=preset animation=static
UI174_PAUSED_MOTION_PASS dialog=preset animation=static
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根的短时 Qt offscreen 实例，仅构造并显示/隐藏两个 dialog，
未显示主窗口、未启动 EXE/后台服务/持续 GUI、未接入硬件或网络。offscreen 环境提示缺失 PySide6 虚拟字体目录及
`propagateSizeHints()` 不支持；未据此宣称 Windows 字体/HIDPI/真实窗口通过。未运行完整 GUI/HIDPI/读屏/视觉帧差分、EXE 启动、
真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包状态

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.74` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.74`
- size：`47,891,949` bytes
- SHA-256：`5514973AF8FB8275A8510DA7159523E494370902114FBF091CF4FC8FF0F57916`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
