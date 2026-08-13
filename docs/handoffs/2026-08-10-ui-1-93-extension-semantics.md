# UI-1.93 扩展 capability card 语义交接

日期：2026-08-10  
范围：标题与状态 badge 的 status/accessibility 语义分离

## 结果

- capability title 不再设置 `role="status"`，避免标题被解释成状态。
- capability state badge 保留 `role="status"`、`state` 属性和既有 contract-only/attach-only 三主题样式。
- DTO、卡片布局、无障碍文案、只读边界和 OTA/debug contract-only/attach-only 约束不变。

## 角色与独立复核

```text
产品角色       019fec53-6bbb-7b93-bf5b-bd09b52fa51c  called; wait timed out; closed
架构角色       019fec53-6c07-7d93-b99d-c6f2dd2df36f  called; wait timed out; closed
UI 设计角色    019fec53-6c58-7f23-b977-832e98d56a30  called; wait timed out; closed
开发角色       019fec53-6ca1-7180-a17c-d57c0b13678f  called; wait timed out; closed
验证角色       019fec53-6cf6-7291-bb3b-c8c169db5518  called; wait timed out; closed
打包角色       019fec53-6d3e-7130-bb15-21d1343d41b3  called; wait timed out; closed
独立质量复核   019fec54-681b-7b82-b68b-befc10967c5c  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认 title=0/status=7
角色分布与两种 state 完整；readability/simplicity 确认只删除一个错误属性；architecture 确认 application
DTO 和 presentation/QSS owner 不变；security 确认无 I/O、设备、密钥或依赖；performance 确认无新增运行时开销。
嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI193_EXTENSION_SEMANTIC_VECTOR_PASS themes=3 titles=7 title_status_roles=0 state_status_roles=7 states=contract_only,attach_only
```

向量使用 Qt offscreen 内存对象且未显示主窗口；未启动可见 GUI/EXE，未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link、
硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.93` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致，provenance verify 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.93
size: 47,913,940 bytes
SHA-256: 69D6713DFD819D1569792A82A5870D8B2927A52D831674E01B3EC94DF2A699D3
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
