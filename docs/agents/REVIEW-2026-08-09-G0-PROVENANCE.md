# G0 provenance gate review

日期：2026-08-09  
范围：Windows PyInstaller core/BLE × onedir/onefile packaging, provenance sidecars,
CI matrix, and release-boundary documentation.

## 六角色记录

| 角色 | agent id | 结论 |
|---|---|---|
| 产品 | 019fe503-ac5e-7a01-b589-f4173deab2c8 | 通过；要求四矩阵隔离、manifest/hash、PE metadata、未签名/硬件待验收可见 |
| 架构 | 019fe503-aca2-77e3-a6c3-d257392962ce | 通过；单一 package.ps1、唯一版本源、隔离 staging、CI 矩阵边界 |
| UI 设计 | 019fe503-ad20-75a0-8951-d645e6b25945 | 通过；本切片不增加 About UI，NOTICE/manifest 足以承载工程构建状态 |
| 开发 | 019fe503-ace1-76c1-8670-9a1c3bbad72a | 通过；实现 provenance helper、版本资源、内容门和四矩阵输出 |
| 验证 | 019fe503-ad5c-7740-84c1-565c50f5363a | 通过；要求四包静态/归档/启动门；COM、BLE radio 未运行 |
| 打包/流程 | 019fe503-ad9d-79d1-8d60-26d635fa9dd9 | 通过；确认 unsigned engineering build、license inventory pending、vendor binary 禁止 |

六角色均为只读审查；架构建议明确将 BLE collection 收敛至 Windows WinRT 后端，
开发实现后由父代理重新静态检查和重建 BLE 两种模式。

## 实现范围

- scripts/package.ps1 是唯一入口；版本从 src/serialforge/__init__.py 读取；
- scripts/provenance.py 只使用 Python 标准库，生成 PyInstaller version resource、manifest、
  SHA256 sidecar 和 manifest verification；
- 输出按 version/variant/mode 隔离，PyInstaller work/spec 位于 build/pyinstaller/provenance；
- core 不收集 Bleak/WinRT，BLE 只显式启用 ble extra 并收集 bleak.backends.winrt、manufacturer
  table 和 winrt；任何变体禁止 J-Link/SEGGER/probe-rs vendor binary；
- 每个模式随包保存 archive listing、locked dependency tree、NOTICE 和第三方 inventory；
- CI 改为 core/BLE × onedir/onefile 四矩阵，并为每个矩阵项上传唯一名称；
- 没有修改业务 transport、协议、RTT 驱动或 J-Link SDK。

## 独立复核与简化评估

独立复核确认 package script 的删除范围只指向明确的 generated release/build 子目录，
core 与 BLE 不共享 staging；manifest 使用 artifact 的相对路径、大小和 SHA256，
并记录 PE file/product version、signature status、source revision、uv.lock hash、
hardware acceptance 和 license inventory。核心组合根没有新增包装逻辑或运行时依赖。

简化评估结论：单一 PowerShell 入口加一个标准库 helper 足以覆盖版本资源、内容扫描和
sidecar 生成，不引入 spec 手工分叉、插件系统、安装器或签名假象。BLE 收集从全平台 Bleak
子模块收窄到 Windows WinRT 后端，避免将 macOS/Android 后端无谓放入 Windows 包；保留
PyInstaller/pyserial 的既有显式收集边界。

## 公共来源适用性

本轮没有修改嵌入式 C/C++、MCU、BSP/HAL、RTOS、ISR/DMA、驱动或固件，因此不存在适用于
本代码切片的制造商固件要求；vendor-public-source applicability 为 N/A。J-Link RTT
仍是 attach-only 的最后阶段，未引入 SEGGER 依赖，也未作 J-Link 兼容性或授权声明。
PyInstaller、PySide6、pyserial、Bleak 和 winrt 的归属/许可只记录在 inventory，未冒充
完整法律许可 bundle。

## 非破坏性验证证据

静态与元数据：

- uv lock --check：通过；
- scripts/check.ps1：通过，包含 src/scripts compileall 与 Ruff；
- package.ps1 PowerShell parser：通过；
- version-file 生成 PE version resource 0.1.0.0：通过；
- 无测试代码、mock、fixture、harness 或 test-only 资产。

最终四矩阵产物均由 package.ps1 生成并再次执行 provenance.py verify：

| variant | mode | artifact bytes | SHA256 | BLE matches | vendor matches |
|---|---|---:|---|---:|---:|
| core | onedir | 3056967 | 4A857AA2E879B0A68A4D600F9A028B0218BDED657FFEC13B41060B046C389CA6 | 0 | 0 |
| core | onefile | 47552847 | 0E5CF2D8B201BFD2F64F53F061306139B0CAFE94466601041023F139B07F7683 | 0 | 0 |
| ble | onedir | 3406591 | B196AD1289A4AABBB50215962E4F854DC8EE884DDCE42995D337D22B89F5852F | 69 | 0 |
| ble | onefile | 48964715 | 24EE4E2C4326FC17B4BBAA4A250127DC3286607914072E6D50C6FF251CBC216F | 69 | 0 |

四包 PE file/product version 均为 0.1.0.0，ProductName 均为 SerialForge，
Authenticode 均为 NotSigned，release_eligible 均为 false，license status 为
inventory_only_pending_full_texts。core 的 Bleak/WinRT archive match 为 0；
BLE 两种模式均包含 Bleak 与 WinRT 内容。

GUI 启动/关闭：

| artifact | launcher PID | GUI PID | residual |
|---|---:|---:|---:|
| core onedir | 58552 | 58552 | 0 |
| core onefile | 27516 | 4088 | 0 |
| ble onedir | 70032 | 70032 | 0 |
| ble onefile | 106272 | 83448 | 0 |

以上启动验证只是本地窗口启动和 WM_CLOSE/进程清理；没有 COM 设备、BLE 广播、
J-Link 驱动、目标板、签名服务或正式发行许可环境。

## 未决项

- 本地 checkout 无 Git revision，manifest 使用 local-unpinned；CI 使用 GITHUB_SHA；
- 包未签名，完整第三方许可证文本和法律复核仍未完成；
- UART 当前设备发现数量为 0，真实 UART、网络设备、BLE radio 和协议实时 stream acceptance
  仍需授权硬件/loopback 门；
- J-Link RTT 继续最后实施，不在本轮下载、安装、加载或分发驱动/SDK/DLL/厂商工具。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
