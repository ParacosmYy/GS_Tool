# ADR-0085：扩展工具站只读接入概览

日期：2026-08-10  
状态：Accepted  
范围：`presentation/embedded_extension_panel.py` 的顶部信息层

## 背景

工具站已经展示 XMODEM/YMODEM/TFTP、AES-256-GCM/AES-128-CCM 和 RTT/J-Link
的能力卡片，但用户需要逐卡阅读才能判断当前是否真的有可执行后端。当前没有目标型号、
bootloader、授权和硬件验收上下文，不能把预留能力伪装成可用功能。

## 决策

- 新增 `embedded_station_overview.py`，只消费 `extension_capabilities()` 返回的 immutable catalog。
- ARCH-7d / UI-1.96 将能力计数和状态派生收回 `application/extension_station.py` 的 `ExtensionStationSummary`；presentation 只消费该 DTO。
- 派生能力槽位总数、已激活后端数和当前动作；当前 `contract_only`/`attach_only` catalog 明确呈现 `7 / 0 / 无`。
- 明确标注“只读规划层”和真实接入的前置条件，不新增 action、后端探测、设备连接、timer 或状态源。
- 复用 `QFrame[role="stationBand"]` 与现有 semantic token，不引入新的 palette 或白色系统回退；accessible name/description 与指标说明同步提供。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI195_STATION_OVERVIEW_VECTOR_PASS themes=3 capabilities=7 values=7,0,无 state=readonly history-source
```

未显示 GUI、未启动 EXE、未探测设备或连接真实 OTA/AES/RTT/J-Link；未创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。本轮没有嵌入式 C/C++ 变更，MCU vendor source applicability=N/A。
