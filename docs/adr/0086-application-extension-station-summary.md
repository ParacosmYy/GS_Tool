# ADR-0086：扩展站摘要的 application owner

日期：2026-08-11  
状态：Accepted  
范围：扩展工具站的 capability catalog 到 UI summary 投影

## 背景

UI-1.95 的 overview 为了显示 `7 / 0 / 无`，在 presentation 里解释了
`contract_only` 与 `attach_only`。这没有造成运行时错误，但会让 UI 复制 application
状态策略；将来能力状态扩展时容易形成第二套状态源。

## 决策

- `application/extension_station.py` 提供 frozen、bounded 的 `ExtensionStationSummary`。
- `extension_station_summary()` 是 catalog 到摘要的唯一派生入口，拥有能力计数、激活后端计数和接入前置条件文案。
- `presentation/embedded_station_overview.py` 只渲染 `ExtensionStationSummary`，不再导入或解释 capability state 集合。
- 当前 summary 强制 `read_only=True`；真实 OTA/J-Link 后端仍需目标授权、公开一手资料和独立验收，不在本 ADR 激活。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
ARCH7D_UI196_SUMMARY_BOUNDARY_VECTOR_PASS themes=3 capabilities=7 active=0 action=无 ui_consumes=immutable-summary
```

未启动 GUI/EXE、未连接真实设备或 vendor 工具；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
本轮没有嵌入式 C/C++ 变更，MCU vendor source applicability=N/A。
