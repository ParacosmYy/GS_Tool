# UI-1.95 扩展工具站接入概览交接

日期：2026-08-10  
范围：只读展示未来 OTA/debug 工具站的接入状态和前置条件

## 结果

- 新增 `presentation/embedded_station_overview.py`，从既有 immutable capability catalog 派生能力槽位数、已激活后端数和当前动作。
- 扩展页顶部明确显示“只读规划层”、`7` 个能力槽位、`0` 个已激活后端和“无”当前动作，并说明真实接入前置条件。
- 只增加 presentation composition；没有新增 OTA/J-Link 后端、设备探测、动作按钮、密钥、vendor SDK、socket、pyserial、crypto 或 timer。

## 角色与独立复核

```text
产品角色       019fec62-f401-7d43-aafe-d8247779ed79  called; wait timed out; closed
架构角色       019fec62-f44f-72c0-ab0d-a2a31bdf64b1  called; wait timed out; closed
UI 设计角色    019fec62-f49f-7f72-aca1-23dd792e1370  called; wait timed out; closed
开发角色       019fec62-f4e7-7c61-a88c-73200f2ce6e3  called; wait timed out; closed
验证角色       019fec62-f536-7fe3-acc9-91ffbf6b094b  called; wait timed out; closed
打包角色       019fec62-f58a-74a2-9f5d-a48c54af8d30  called; wait timed out; closed
独立质量复核   019fec64-ffb9-7481-b01a-a9245c1cea96  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认 catalog 派生值为 `7/0/无`、
三主题投影与 accessibility 文案一致；readability/simplicity 确认 overview 只负责展示与指标组合；architecture 确认
不反向依赖 backend、transport 或 domain；security 确认无设备、密钥、SDK、socket 或 crypto 输入；performance 确认无 timer、
线程、扫描或每帧工作。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI195_STATION_OVERVIEW_VECTOR_PASS themes=3 capabilities=7 values=7,0,无 state=readonly history-source
```

向量使用 Qt offscreen 内存对象且未显示主窗口；未启动 GUI/EXE，未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.95` 生成并覆盖根目录
`SerialForge.exe`；canonical/root 字节一致，仓库 provenance verifier 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.95
size: 47,919,227 bytes
SHA-256: DBABFB57AD3D63393D57B1C3281C1A1C16CC0E7C572928ABFD940FFE660E5618
archive listing SHA-256: A11F34E225D750036A4AEE1DF2D77E2AE1B09B1C79F71719DB10FE6512852CA6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
