# UI-1.91 扩展 capability card 语义层级交接

日期：2026-08-10  
范围：扩展 / 工具站 capability card 的标题与状态 badge 主题层级

## 结果

- `QLabel#extensionCapabilityTitle` 从通用 status pill 中分离，使用正文文字层级、透明背景和无边框外壳。
- `QLabel#extensionCapabilityState` 独立表达能力状态；`contract_only` 使用 info 语义，`attach_only` 使用 history/purple 语义。
- 未增加主题 token、hex literal、DTO 字段、业务状态源、动画时钟、交互动作或 OTA/debug 依赖。

## 角色与复核

```text
产品角色       019fec44-437e-7612-9b50-e4deda0e40fe  called; wait timed out; closed
架构角色       019fec44-43d0-7a92-bf32-0f423c836506  called; wait timed out; closed
UI 设计角色    019fec44-4422-7e11-bee3-7f9d71188fcc  called; wait timed out; closed
开发角色       019fec44-447d-71b1-b8c6-acaac1be3f85  called; wait timed out; closed
验证角色       019fec44-44c4-7aa0-b1b2-6b47a011f88d  called; wait timed out; closed
打包角色       019fec44-4516-7c30-ae51-7897cdb5658b  called; wait timed out; closed
独立质量复核   019fec44-f13c-7620-9751-d4eb703118c9  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：

- correctness：标题和状态 objectName/state selector 与 panel 现有属性一致，7/7 卡片覆盖两种状态。
- readability/simplicity：只新增三条稳定 QSS 规则，复用现有 token，无重复 palette、DTO 或 helper。
- architecture：样式归 presentation theme owner，application DTO 和 OTA/debug owner 不变。
- security：无 I/O、网络、设备句柄、密钥、vendor SDK 或加密实现。
- performance：静态 QSS，无 timer、动画或额外每帧工作。

嵌入式 C/C++ 适用性：N/A；本轮仅修改 Python/PySide6 presentation 样式。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI191_EXTENSION_HIERARCHY_PASS themes=3 titles=7 states=contract_only,attach_only
```

向量使用 Qt offscreen 内存对象，未显示主窗口；未启动可见 GUI/EXE，未运行 HIDPI、读屏、真实
OTA/AES/RTT/J-Link、硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.91` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致，provenance 校验通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.91
size: 47,914,013 bytes
SHA-256: 30AB41BD37BCE478565413BEB43A19644F608AA0865C023868A77A64889D55AB
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
