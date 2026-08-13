# ARCH-7d / UI-1.96 扩展站摘要边界交接

日期：2026-08-11  
范围：将扩展工具站摘要派生从 presentation 收回 application

## 结果

- 新增 `application/extension_station.py`，提供 frozen/bounded `ExtensionStationSummary` 与唯一派生函数 `extension_station_summary()`。
- `presentation/embedded_station_overview.py` 只接收 summary DTO 并渲染 `7` 个能力槽位、`0` 个已激活后端和“无”当前动作。
- 保持 contract-only / attach-only、只读规划层、无设备探测、无 OTA/J-Link 动作的安全边界；未新增后端、状态源、timer 或依赖。

## 角色与独立复核

```text
产品角色       019fec69-1c74-7a32-9b98-2e6bf4e5bea8  called; wait timed out; closed
架构角色       019fec69-1d19-74e3-912c-6077a0bd49ab  called; wait timed out; closed
UI 设计角色    019fec69-1cc9-7a90-84b0-a2742c32a555  called; wait timed out; closed
开发角色       019fec69-1d62-7200-983c-af024eb515df  called; wait timed out; closed
验证角色       019fec69-1db0-7ed1-b9d6-837d062769c7  called; wait timed out; closed
打包角色       019fec69-1dfe-7d53-8c0e-3150241e1cfe  called; wait timed out; closed
独立质量复核   019fec6a-31ae-7f90-9d76-892bec633b3a  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认 bounded DTO、`7/0/无` 派生和三主题投影；
readability/simplicity 确认 application 只负责派生、presentation 只负责渲染；architecture 确认状态策略不再复制到 UI；security 确认无设备、密钥、SDK、socket 或 crypto 输入；performance 确认无 timer、线程、扫描或每帧工作。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
ARCH7D_UI196_SUMMARY_BOUNDARY_VECTOR_PASS themes=3 capabilities=7 active=0 action=无 ui_consumes=immutable-summary
```

向量使用 Qt offscreen 内存对象且未显示主窗口；未启动 GUI/EXE，未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.96` 生成并覆盖根目录
`SerialForge.exe`；canonical/root 字节一致，仓库 provenance verifier 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.96
size: 47,921,659 bytes
SHA-256: 23B00CF14295A6F5AC399913ED1C8E46EDD8EB1262A4EC3EB1BE2A88367BD950
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
