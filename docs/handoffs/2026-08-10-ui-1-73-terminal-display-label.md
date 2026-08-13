# UI-1.73 实时观测显示字段标签主题层级

日期：2026-08-10  
范围：实时观测工具栏“显示”字段的主题语义层级。  
父代理：Codex；父代理是本轮唯一写入者。  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 Git/Codex worktree。

## 用户结果

- 实时观测工具栏中“显示”标签统一进入 `QLabel[role="muted"]` 次级视觉层级，避免系统 palette 把字段文字渲染成高亮白。
- “实时观测”继续使用 `role="section"`；暂停、记录和接收活动继续使用 `role="status"`。
- display mode combo 的文本/Hex 两项、`currentIndexChanged`、`_rerender_preview`、暂停/记录动作、快捷键、焦点、Tab 顺序和布局不变。

## 架构边界

`src/serialforge/presentation/controllers/terminal.py` 新增局部 `_field_label(text)`。helper 只创建 `QLabel` 并设置
`role="muted"`，由既有三套主题 stylesheet 负责颜色。它不读取 ViewModel/DTO，不接收 signal，不拥有 display mode/暂停/记录/接收
状态，不创建 timer，不参与 MotionController 生命周期，也不扩展为跨页面 label factory。

## 实际修改

- `src/serialforge/presentation/controllers/terminal.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0060-terminal-display-field-label-hierarchy.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 角色调用与独立质量复核

```text
产品角色       019feb98-8f9f-7442-8641-03f05dc4a76d  called before source edit; wait timed out; closed
架构角色       019feb98-8fe9-7a02-a234-f6cb2c885044  called before source edit; wait timed out; closed
UI 设计角色    019feb98-9032-7de3-ba1f-9f7c4ffcc82e  called before source edit; wait timed out; closed
开发角色       019feb98-9080-7353-8d72-0ab8d5cff758  called before source edit; wait timed out; closed
验证角色       019feb98-90d0-7581-9cc2-e058b26ad90b  called before source edit; wait timed out; closed
打包/流程角色  019feb98-9123-7382-8234-cb9c67833d42  called before source edit; wait timed out; closed
独立质量复核   019feb9a-75d9-7892-86df-dd257735b192  called after implementation; wait timed out; closed
```

六角色与独立复核均在运行时窗口内超时，未把超时当成通过。父代理完成五轴审查：

- correctness：只增加 muted role 并替换一个普通标签，section/status 与 display mode 行为保持。
- readability/simplicity：局部 helper 只有单一职责，terminal.py 共 366 行，不接近 1000 行门禁。
- architecture：helper 留在 terminal builder，不新增 shared form service、跨 controller 依赖或状态源。
- security：没有输入、存储、网络、密钥或依赖变化。
- performance：只在工具栏构建时创建既有 QLabel，不增加动画、timer、重绘循环或热路径分支。

简化评估结论：复用已有 `QLabel[role="muted"]` selector 的局部 helper 是最小实现；增加 display-mode 专用动画或全局 label 工厂均会扩大耦合，
本轮不采用。

## 验证

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI173_TERMINAL_TOOLBAR_PASS theme=star_trail labels=5 display_modes=2
UI173_TERMINAL_TOOLBAR_PASS theme=moonlit_ocean labels=5 display_modes=2
UI173_TERMINAL_TOOLBAR_PASS theme=sakura_night labels=5 display_modes=2
```

vector 使用真实 application composition root 的短时 Qt offscreen 实例，不是测试资产；环境提示 PySide6 虚拟字体目录缺失，未据此宣称
完整字体/HIDPI 通过。未运行完整 GUI/EXE 启动、读屏/视觉帧差分、真实 UART/TCP/BLE/RTT/J-Link/OTA、硬件、签名和正式发行验收；未创建、
修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包状态

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.73` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- size：`47,888,984` bytes
- SHA-256：`6D9EDF9E9AC4B717450260C83E6EB34D144A390FF59F74B45AA8E16AC4283AE9`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
