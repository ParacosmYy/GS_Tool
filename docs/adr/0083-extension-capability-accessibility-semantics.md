# ADR-0083：扩展 capability card 的可访问语义

日期：2026-08-10  
状态：Accepted  
范围：`presentation/embedded_extension_panel.py`

## 背景

UI-1.91 已把 capability card 的标题和状态 badge 视觉层级分开，但标题仍带有
`role="status"` presentation 属性。这样会把“能力名称”与“能力成熟度”混成同一类状态语义，
不利于读屏和辅助技术理解卡片结构。

## 决策

- `extensionCapabilityTitle` 不设置 `role` 属性，只保留 objectName、文本和现有 QLabel 行为。
- `extensionCapabilityState` 保留 `role="status"` 与 `state` 属性，继续表达 `contract_only` / `attach_only`。
- 复用既有 QSS selectors 和三主题 token；不新增 accessibility 状态源、DTO 字段或业务动作。

## 边界

application capability DTO 仍是能力事实来源，panel 仍是只读 presentation；OTA 传输、安全升级、RTT/J-Link
执行 adapter、密钥、设备句柄和 vendor SDK 不进入本次变更。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI193_EXTENSION_SEMANTIC_VECTOR_PASS themes=3 titles=7 title_status_roles=0 state_status_roles=7 states=contract_only,attach_only
```

Qt offscreen vector 未显示主窗口；未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收。
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。
