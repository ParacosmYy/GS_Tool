# 技术调研与资源清单

调研日期：2026-08-09。当前决定优先快速交付 Windows Python 版本，不将 Rust/Tauri 作为首版环境依赖。

| 资源 | 用途 | 结论 |
|---|---|---|
| [Qt for Python](https://doc.qt.io/qtforpython-6/) | PySide6 官方 Python 绑定 | 作为首版 UI；按 LGPLv3/GPLv3/商业许可确定发行策略。 |
| [pySerial](https://pyserial.readthedocs.io/en/stable/) | Windows UART 枚举和收发 | 首版 UART 适配器。设备 I/O 必须在 worker 中执行。 |
| [Bleak Windows backend](https://bleak.readthedocs.io/en/latest/backends/windows.html) | Windows WinRT BLE GATT | M3 已接入 BLE Central；仍需要独立 asyncio/Qt 线程边界和实机验证。 |
| [PyInstaller](https://pyinstaller.org/en/stable/) | Windows EXE 打包 | 首版打包方案；必须检查隐藏导入、Qt 资源、驱动和第三方许可证。 |
| [Serial Studio](https://github.com/Serial-Studio/Serial-Studio) | 功能范围和界面参考 | 仅借鉴交互与模块范围，不复制 GPL 代码或资源。 |
| [NMEA 0183 — NMEA](https://www.nmea.org/nmea-0183.html) | 标准归属、版本和版权边界 | NMEA 官方页面称标准正文需购买；不复制标准全文/句型表，不宣称认证兼容。 |
| [Quectel LC29H(BS) GNSS Protocol Specification V1.0](https://www.quectel.com/content/uploads/2024/02/Quectel_LC29HBS_GNSS_Protocol_Specification_V1.0.pdf) | 设备公开规格中的 NMEA `$...*HH` 线格式和 XOR 参考 | 仅作为该厂商设备的工程参考；实现必须以实际设备手册/授权标准确认，不能泛化为所有 NMEA 设备要求。 |
| [Modbus Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf) | Modbus RTU ADU 布局、256 B 上限、t1.5/t3.5 时序边界和 CRC16 wire 顺序 | Modbus Organization 官方、2006-12-20；适用于 serial line 规范参考，不是特定设备制造商要求；本轮只实现已分帧 ADU 校验。 |
| [Modbus Application Protocol V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf) | PDU/function-code/exception 语义边界 | 官方应用协议参考；本轮不实现完整 function-code 字段 codec、主从事务或 TX。 |
| [MAVLink overview](https://mavlink.io/en/about/overview.html) | v1/v2 packet、CRC、方言/生成边界 | MAVLink 官方协议文档；本轮只实现已分帧 RX 结构与显式 CRC_EXTRA 映射，不把完整方言全集内置。 |
| [MAVLink packet serialization](https://mavlink.io/en/guide/serialization.html) | v1/v2 header、payload 上限、CRC 覆盖范围和 v2 signature 长度 | MAVLink 官方文档；作为 packet validator 的公开协议参考，不是特定飞控厂商要求。 |
| [MAVLink message signing](https://mavlink.io/en/guide/message_signing.html) | signed flag、13 B signature 和认证边界 | MAVLink 官方文档；M5i 只识别/保留签名并报告未认证，不管理密钥。 |
| [MAVLink common.xml](https://raw.githubusercontent.com/mavlink/mavlink/master/message_definitions/v1.0/common.xml) | 官方 common dialect message definition 来源 | 官方 GitHub raw 定义；示例使用 message id 0/CRC_EXTRA 50，`master` 仅开发参考，生产前必须固定 revision。 |
| [SEGGER RTT Telnet Channel](https://kb.segger.com/J-Link_RTT_TELNET_Channel) | J-Link RTT 外部桥接 | M6 已采用标准库 TCP attach-only；通过 Config String 选择 0/1，不绑定 SDK/DLL。 |
| [SEGGER J-Link RTT Client](https://kb.segger.com/J-Link_RTT_Client) | 官方客户端行为参考 | 它需要活动 debug session，且 RTT Client 本身只支持 up/down channel 0；不随 SerialForge 分发。 |
| [SEGGER RTT Viewer](https://www.segger.com/products/debug-probes/j-link/tools/rtt-viewer/) | RTT 用户体验参考 | 不复制实现；J-Link SDK/DLL 授权和分发仍需单独确认。 |

## 许可证与分发

1. PySide6/Qt Community Edition 涉及 LGPLv3/GPLv3/商业许可，发行前根据应用是否闭源、是否修改 Qt、如何打包动态库确认法律路径；
2. pyserial、可选 bleak、PyInstaller 和其传递依赖在锁定后记录准确许可证；
3. SEGGER SDK/DLL 和工具不默认放入发行包，未经书面授权不分发；M6 只连接用户已启动的 Telnet 服务；
4. PyInstaller 单文件不等于安装器、签名或驱动分发；
5. 产品不默认遥测、远程内容或任意脚本执行。

## 关键判断

Python 对普通 UART、网络调试和中等日志吞吐足够；高吞吐场景必须使用 worker、批量 UI 刷新、有界队列和原始记录优先策略。BLE 与 RTT 的难点主要在 Windows API、设备状态和供应商授权，不是换成 Rust 就会消失。
