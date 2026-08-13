# UI-1.75 自定义连接配置 Metadata Surface

日期：2026-08-10  
范围：自定义连接配置编辑 dialog 的表单视觉分组与字段语义层级。  
父代理：Codex；父代理是本轮唯一写入者。  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 Git/Codex worktree。

## 用户结果

- `ConnectionPresetEditorDialog` 的名称/备注输入现在位于 `QFrame#presetMetadataFields[role="surface"]` 内，视觉上与主连接页的主题 surface 保持一致。
- “名称”“备注”使用 `QLabel[role="muted"]` 次级字段层级；两个 `QLineEdit` 仍保留原生 bounded 输入和占位文案。
- metadata DTO、key 生成、空名称/长度校验、保存/取消 action、初始 focus、Tab/accessibility、连接 session gate 和 UI-1.74 淡入生命周期不变。

## 架构边界

本轮只修改 `presentation/connection_preset_editor.py`。dialog 自己拥有 layout surface 和局部 `_field_label()`；主题通过现有
`QWidget[role="surface"]`/`QLabel[role="muted"]` selector 提供外观。controller/store 继续拥有 DTO、持久化和 action，未新增
ViewModel、domain/application 依赖、主题 token、QSS 文件、timer、业务状态或资源。

## 实际修改

- `src/serialforge/presentation/connection_preset_editor.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0062-preset-metadata-surface.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 角色调用与独立质量复核

```text
产品角色       019febab-a9b8-78f3-96b2-dcf7480985da  called before source edit; wait timed out; closed
架构角色       019febab-a9fb-7e50-977a-1898eda3e6ad  called before source edit; wait timed out; closed
UI 设计角色    019febab-aa4c-7de2-a064-edee5214da90  called before source edit; wait timed out; closed
开发角色       019febab-aa9c-7152-9510-d25f9be76d4b  called before source edit; wait timed out; closed
验证角色       019febab-aae4-77d2-b800-e692bf9ea2ae  called before source edit; wait timed out; closed
打包/流程角色  019febab-ab32-7c33-bd09-f7da51455795  called before source edit; wait timed out; closed
独立质量复核   019febad-460e-7ac2-9e75-1b64c0b01172  called after implementation; wait timed out; closed
```

六角色与独立复核均未返回完整报告，未把超时当成通过。父代理完成 correctness/readability/simplicity/architecture/security/performance
五轴审查；简化结论是复用既有 surface/muted selector，不引入全局 form factory、dialog 基类、额外 token 或动画时钟。

## 验证

```text
pwsh.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS
.venv\Scripts\python.exe -m compileall -q src  PASS
UI175_PRESET_SURFACE_PASS theme=star_trail fields=2 focus=label validation=pass
UI175_PRESET_SURFACE_PASS theme=moonlit_ocean fields=2 focus=label validation=pass
UI175_PRESET_SURFACE_PASS theme=sakura_night fields=2 focus=label validation=pass
```

vector 使用真实组合根的短时 Qt offscreen 实例；环境提示缺失 PySide6 虚拟字体目录及 `propagateSizeHints()` 不支持，未据此宣称
Windows 字体/HIDPI/真实窗口通过。完整 GUI/HIDPI/读屏/视觉帧差分、EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和
正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包状态

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.75` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.75`
- size：`47,891,717` bytes
- SHA-256：`01E1C090EA020523DCDA4D63FEAEBB1E84BD4EF48B27CE598CE78EBC4F1072D4`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
