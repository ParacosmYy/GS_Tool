# UI-1.94 UART 参数摘要 rail 交接

日期：2026-08-10  
范围：UART 表单当前 wire format 的即时只读确认

## 结果

- 新增 `presentation/uart_timing_surface.py`，显示 `baud · data/parity/stop · flow` 摘要并同步 accessible description。
- `connection_builder.py` 只接入现有 UART combo signal；手动选择和 ConnectionPreset 填入都会即时刷新。
- 摘要位于 UART panel 内，复用 semantic QSS；不改变连接、协议、ViewModel、TransportConfig、Tab 或业务状态。

## 角色与独立复核

```text
产品角色       019fec5a-2724-7a83-8ab4-4e4c82420642  called; wait timed out; closed
架构角色       019fec5a-2768-77d3-a8a1-ba564ab15ca1  called; wait timed out; closed
UI 设计角色    019fec5a-27b4-7401-8fe3-f97f275173ee  called; wait timed out; closed
开发角色       019fec5a-2803-7031-9881-7c6e0efdff63  called; wait timed out; closed
验证角色       019fec5a-2851-7852-ba14-5b3cceab2a4a  called; wait timed out; closed
打包角色       019fec5a-289e-7280-8bf4-0c8403eb9d7a  called; wait timed out; closed
独立质量复核   019fec5c-b357-70b1-8ae0-1941cd296bd8  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认默认/手动/preset 三条
投影路径和枚举映射；readability/simplicity 确认 surface 只负责格式化展示、builder 只负责 wiring；architecture
确认 application/domain/transport 不依赖 presentation summary；security 确认无 I/O、设备、密钥或新增依赖；
performance 确认无 timer、线程或每帧工作。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI194_UART_SUMMARY_VECTOR_PASS default=1 manual=1 preset=1 themes=3
```

向量使用 Qt offscreen 内存对象且未显示主窗口；未启动可见 GUI/EXE，未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link、
硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.94` 生成并覆盖根目录
`SerialForge.exe`；canonical/root 字节一致，仓库 provenance verifier 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.94
size: 47,916,119 bytes
SHA-256: E6D317841A8983C81CACED40C9798DB70444016E661418589436BBA844F5328D
archive listing SHA-256: D41985C7165A66BD66752652C63615CA83A29BC83F22CEED3360D0E176A7F09F
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
