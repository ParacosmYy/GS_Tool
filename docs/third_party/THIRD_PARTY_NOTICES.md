# SerialForge third-party inventory

更新时间：2026-08-09。版本来自当前 `uv.lock`；本文件是归属/许可盘点，不构成法律意见，也不宣称已经完成发行许可审查。

| 组件 | 锁定版本 | 用途 | 发行前复核 |
|---|---:|---|---|
| Python | 3.12.13（当前环境） | Windows 运行时 | 随发行物确认 Python 许可文本与 NOTICE |
| PySide6 / Qt for Python | 6.11.1 | Qt Widgets 桌面 UI | 依据 Qt for Python 官方许可选项确认 LGPL/GPL/商业许可及对应文本 |
| pyserial | 3.5 | UART 枚举与收发 | 随发行物保留项目许可与归属文本 |
| PyInstaller | 6.22.0 | onedir/onefile 打包 | 复核 PyInstaller 及其 bootloader 许可/例外说明 |
| Ruff | 0.16.2 | 开发期静态检查 | 不进入运行时功能，发行归属仍随开发环境记录 |
| Bleak（可选） | 3.0.2 | Windows BLE GATT 适配器（M3 已接入） | 只有启用 BLE 包时纳入发行物，并复核 WinRT 传递依赖许可 |
| winrt-*（可选传递依赖） | 3.2.1 | Bleak 的 Windows Runtime 绑定 | 只有启用 BLE 包时纳入发行物，按锁定清单逐项复核 |

当前默认 UART/network/RTT 桌面包不包含 Bleak、SEGGER/J-Link SDK、供应商 DLL 或 RTT 二进制；RTT 只包含标准库 Telnet attach 代码；BLE-enabled 变体由 `scripts/package.ps1 -Ble` 显式生成。`uv tree --locked --all-groups` 是传递依赖的完整版本证据；生成发行包时仍要检查 PyInstaller 实际收集内容。

发行前必须完成：

- 保存每个实际分发组件的许可证全文和归属说明；
- 确认 PySide6/Qt 的选定许可路径与反向工程、动态链接和修改义务；
- 对 onefile/onedir 做实际内容盘点；
- RTT 用户必须自行安装并按 SEGGER 条款使用 J-Link 工具、驱动和目标硬件；SerialForge 不随包分发这些组件。若未来增加进程托管或 DLL 后端，必须单独记录书面授权范围。
