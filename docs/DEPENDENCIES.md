# 依赖与许可证记录

当前锁定环境（2026-08-09，`uv.lock`）：Python 3.12.13、PySide6 6.11.1、pyserial 3.5、PyInstaller 6.22.0、Ruff 0.16.2；BLE extra 为 Bleak 3.0.2。以下是职责边界，不替代发行前的许可证审查。

| 依赖 | 用途 | 选择理由 | 发行前动作 |
|---|---|---|---|
| Python 3.12 | 运行时 | Windows 兼容、部署简单 | 记录解释器分发说明 |
| PySide6 | Qt Widgets/QPainter 界面 | 官方 Python 绑定，现有 QtGui 已足够自绘 bounded curve | 按 LGPLv3/GPLv3/商业许可选择并附带通知；不额外引入 QtCharts/WebEngine |
| pyserial | UART 枚举与收发 | Windows 串口 API 封装成熟 | 锁定版本并核对许可证 |
| bleak（可选 `ble` extra） | Windows BLE GATT | 使用 Windows WinRT 后端 | 实机验证扫描、配对、通知和 MTU |
| PyInstaller | EXE 打包 | 不要求用户安装 Python | 默认包不含 BLE；`package.ps1 -Ble` 时检查 Bleak/WinRT 隐藏导入、资源和许可证 |
| Python socket/asyncio | TCP/UDP 与 RTT Telnet attach-only 桥接 | 无额外网络依赖；RTT 复用标准库 TCP stream | 明确 RTT host/port/channel 和超时；不把 SEGGER SDK/DLL 放进发行包 |
| Python `json`/`struct` 标准库 | schema v2 JSON Pointer/TLV RX codec | 不增加安装复杂度和 PyInstaller hidden-import | 保持声明式、有界、无表达式/动态导入；验证坏 payload 不影响 raw |
| Python 标准库整数/bytes 运算 | M5h 已分帧 Modbus RTU ADU validator/CRC16/component projection | 不引入 Modbus runtime 或第三方协议库 | 保持 CRC 归属、256 B ADU/data 上限和 t1.5/t3.5 未实现边界；验证坏 ADU 可见且不进入 Dataset |
| Python 标准库整数/bytes 运算 | M5i 已分帧 MAVLink v1/v2 validator、CRC-16/MCRF4XX 和 component projection | 不引入 MAVLink runtime、dialect generator 或 crypto 库 | 保持 packet/签名有界、CRC_EXTRA 显式映射和 `UNVERIFIED` 语义；完整 dialect、signature authentication 和 stream resync 另行评估 |
| Python `csv`/`collections`/`threading` 标准库 | typed transform、bounded dataset worker 和 CSV 导出 | 不增加运行时依赖，适合有界内存窗口 | 保持 chain、队列、retention 和导出上限；不引入数据库/图表运行时 |
| Python `json`/`base64`/`pathlib`/`time` 标准库 | 严格 JSONL 历史 RX 回放、时间轴和有界文件读取 | 复用现有 recorder 格式，不引入数据库或回放 SDK | 保持字段集合、行/文件/记录预算与历史来源隔离；坏记录必须可见 |
| Python `threading`/`time` 标准库 | M5e 有界批量命令的可取消 enqueue 调度 | 复用现有 SessionPort，不引入 scheduler、脚本解释器或宏运行时 | 只允许固定步骤和有限延时；记录 enqueue 语义，不宣称设备 ACK |
| 外部 J-Link Commander/GDB Server/IDE | 提供活动 RTT Telnet 服务 | 由用户按 SEGGER 许可和设备环境安装/启动 | SerialForge 不分发、不静默安装、不接管其进程 |

首版不引入 Rust、MinGW、WSL、数据库、动态插件运行时或云 SDK。schema v2 JSON/TLV 不新增
运行时依赖。只有当产品需求证明标准库和现有依赖不足时，才增加新依赖。
