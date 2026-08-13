# UI-1.90 嵌入式扩展工具站 Capability Page

日期：2026-08-10  
范围：为 OTA/debug 预留边界提供只读 capability page，并将 workspace route/icon 从三节点扩展为四节点。

## 结果与边界

- `application/extension_capabilities.py` 提供不可变、有限的 group/reference/state DTO 与静态 catalog：OTA 传输 3 项、OTA 安全 2 项、debug 输出 2 项。
- `presentation/embedded_extension_panel.py` 只渲染 application DTO；XMODEM/YMODEM/TFTP 和 AES-GCM/CCM 显示为“契约预留”，RTT/J-Link 显示为“仅附着”。
- 页面不提供 OTA、解密、签名激活、memory/halt/run/reset/flash、vendor 命令或设备动作，不导入 adapter、socket、pyserial、SEGGER SDK/DLL、加密库或原始密钥。
- `workspace.py` 新增“扩展 / 工具站”Tab；`WorkspaceRouteSurface` 与 `workspace_tab_icons` 从 3 节点/图标扩为 4 节点/图标，前三个业务页、`protocol_tab_index`、焦点、ViewModel 与 transport 生命周期不变。

## 角色与独立复核

```text
第一轮产品角色       019fec3b-351b-72c0-9e06-22dc2d68628f  called; wait timed out; closed
第一轮架构角色       019fec3b-3562-7e30-ac51-9c449133675b  called; wait timed out; closed
第一轮 UI 设计角色   019fec3b-35b2-71f2-886a-6cb9efd3438c  called; wait timed out; closed
第一轮开发角色       019fec3b-35fd-7451-bac4-177f3e3d4d7b  called; wait timed out; closed
第一轮验证角色       019fec3b-364a-7400-867f-ddc724332973  called; wait timed out; closed
第一轮打包角色       019fec3b-3696-7683-8472-c7dde4d74583  called; wait timed out; closed
修复轮产品角色       019fec3c-cbfd-7200-932c-03d1fccba840  import-fix review; wait timed out; closed
修复轮架构角色       019fec3c-cc47-7430-8595-d78578ff405e  import-fix review; wait timed out; closed
修复轮 UI 设计角色   019fec3c-cc9b-76e0-80df-f681274dcf08  import-fix review; wait timed out; closed
修复轮开发角色       019fec3c-ccf1-7b81-aeea-e2db7e0b0f96  import-fix review; wait timed out; closed
修复轮验证角色       019fec3c-cd4a-77a1-93ca-18ef9bc22735  import-fix review; wait timed out; closed
修复轮打包角色       019fec3c-cd9d-7ad3-a7bc-d47dfa258415  import-fix review; wait timed out; closed
独立质量复核         019fec3d-4458-79c2-bfd3-75ea396f7f59  called after implementation; wait timed out; closed
```

所有角色和独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认 DTO group/reference/state 一致、四节点 route/icon 有界且前三页索引不变；readability/simplicity 确认静态 catalog 与单一 panel builder 足够，不引入 plugin registry 或动态加载；architecture 确认 application DTO → presentation 单向依赖、OTA/debug contract owner 不反向依赖 UI；security 确认无设备/I/O/密钥/vendor 能力；performance 确认只有 7 个有限卡片和一次性 icon 绘制，无常驻 timer/worker。

简化评估结论：用不可变 application catalog 作为唯一展示输入，避免在 `MainWindow`、QSS 或卡片里复制 OTA/debug 状态判断；不提前实现任何未授权 backend。
嵌入式 C/C++ 适用性：N/A；本轮只修改 Python/PySide6 与 contract DTO，不涉及 MCU/固件/驱动。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI190_EXTENSION_CATALOG_PASS groups=ota-transfer:3,ota-security:2,debug-output:2
UI190_EXTENSION_SURFACE_PASS themes=3 cards=7 tab-icons=4 route-nodes=4
UI190_COMPOSITION_PASS tabs=4 protocol_tab_index=1 icons=4
```

向量使用 Qt offscreen，未显示主窗口；覆盖 7 项 DTO、3 组 panel、三主题、第四个 Tab glyph、四节点 route 和真实组合根的四 Tab/`protocol_tab_index`/icon parity。仅出现 PySide6 缺失字体目录 warning，不影响 DTO/QSS 组装证据。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实 OTA/加密/RTT/J-Link、硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.90` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.90
size: 47,911,727 bytes
SHA-256: 82EC5FC027001D4CD34D0114AB0BDC0FC3B5793B14E3B332AD7D0DA5DCC5A36
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
