# UI-1.72 批量命令编辑器字段标签主题层级

日期：2026-08-10  
范围：批量命令编辑对话框普通字段/辅助说明的主题语义层级。  
父代理：Codex；父代理是本轮唯一写入者。  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 Git/Codex worktree。

## 用户结果

- 名称、快捷命令、固定顺序说明、当前步骤和延时 5 个静态标签统一进入 `QLabel[role="muted"]` 次级视觉层级。
- 空步骤提示 `_table_empty` 继续使用 `role="subtle"`，错误 label 继续使用 `role="error"`；未出现通过普通字段 helper 覆盖动态状态的情况。
- CommandBatch draft、bounded 文本/Hex payload、校验、popup theme、按钮 action、焦点和 Tab 顺序保持原有语义。

## 架构边界

`src/serialforge/presentation/command_batch_editor.py` 新增局部 `_field_label(text)`。helper 只创建 `QLabel` 并设置
`role="muted"`，复用既有主题 stylesheet；它不读取 ViewModel/DTO，不接收 signal，不写入 `_DraftStep`，不创建 timer，不参与
校验、popup、按钮 action 或 Tab order，也不扩展为跨页面 label factory。

## 实际修改

- `src/serialforge/presentation/command_batch_editor.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0059-command-editor-field-label-hierarchy.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 角色调用与独立质量复核

```text
产品角色       019feb91-c936-7b60-99bf-7e4ceef14360  called before source edit; wait timed out; closed
架构角色       019feb91-c984-7d92-be1c-ef66a4f3c49c  called before source edit; wait timed out; closed
UI 设计角色    019feb91-c9d0-76f2-8558-e0630ee75c28  called before source edit; wait timed out; closed
开发角色       019feb91-ca22-7a31-a4a9-6fe80fc606f5  called before source edit; wait timed out; closed
验证角色       019feb91-ca6b-7f52-b8e1-583999a2dac5  called before source edit; wait timed out; closed
打包/流程角色  019feb91-cab7-7813-b084-ef975a717a2f  called before source edit; wait timed out; closed
独立质量复核   019feb93-267a-7f63-8d16-d9eff1471964  called after implementation; wait timed out; closed
```

六角色与独立复核在运行时窗口内超时，未把超时当成通过。父代理完成五轴审查：

- correctness：5 个目标标签获得 muted role，空态/error role 和实际编辑控件保留。
- readability/simplicity：6 行局部 helper 消除 5 处重复 property 设置，文件共 576 行，未跨越 1000 行门禁。
- architecture：helper 留在 command editor owner，不新增 shared form service 或跨 controller 依赖。
- security：没有输入、存储、网络、密钥或依赖变化；用户编辑内容仍由原 bounded text/domain 校验处理。
- performance：仅在 dialog 构建时创建既有 QLabel，不增加 timer、动画、重绘循环或热路径分支。

简化评估结论：复用既有 `QLabel[role="muted"]` selector 的局部 helper 是最小变化；全局表单抽象会增加耦合，不采用。

## 验证

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI172_COMMAND_LABELS_PASS theme=star_trail fields=5 labels=7
UI172_COMMAND_LABELS_PASS theme=moonlit_ocean fields=5 labels=7
UI172_COMMAND_LABELS_PASS theme=sakura_night fields=5 labels=7
```

vector 只创建短时 Qt offscreen dialog，使用真实默认 draft，不是测试资产。运行环境提示缺少 PySide6 虚拟字体目录；没有据此宣称
完整字体/HIDPI 通过。未运行完整 GUI/EXE 启动、读屏/视觉帧差分、真实 UART/TCP/BLE/RTT/J-Link/OTA、硬件、签名和正式发行验收；没有创建、
修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包状态

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.72` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- size：`47,888,957` bytes
- SHA-256：`2688420A64A7BA7CF20877C259EBF6181F9FA46884061F099A416C75FFFF980B`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
