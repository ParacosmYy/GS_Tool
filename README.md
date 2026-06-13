# EmbedDebug

> 面向嵌入式研发、硬件联调和测试现场的 Qt 桌面调试工作台。EmbedDebug 将串口工站、协议收发、日志留存、回放预览、测量观察、OTA、自动化和工程诊断收敛到一个可验证、可扩展、可交付的应用入口。

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52?style=flat-square)
![CMake](https://img.shields.io/badge/build-CMake%20%2B%20Ninja-064F8C?style=flat-square)
![Windows](https://img.shields.io/badge/platform-Windows%2010%2B-0078D4?style=flat-square)
![Status](https://img.shields.io/badge/status-active%20engineering%20build-f59e0b?style=flat-square)

![EmbedDebug 界面预览](docs/assets/readme/interface-preview.svg)

## 项目定位

嵌入式调试现场常被拆散在串口助手、协议解析器、波形查看器、日志工具、脚本工具和临时诊断面板中。EmbedDebug 的目标是把这些链路整理成一个稳定的工程工作台，让固件、硬件和测试工程师能在同一个入口完成连接配置、命令发送、帧解析、日志留存、会话回放和问题复现。

当前仓库是持续演进的工程版本。项目以 PRD/Specs、分层架构约束、QTest 自动化、启动探针和评分追踪管理每轮迭代。README 只声明仓库内已有证据支撑的能力；未经过真实设备验证的能力不会包装成量产结论。

## 当前快照

| 项目 | 状态 |
|------|------|
| 应用名称 | `EmbedDebug` |
| 主开发分支 | `feat/embed-debug` |
| 启动入口 | `EmbedDebug.bat` |
| 串口工站直达 | `EmbedDebug.bat --station serial` |
| 档案启动 | `--profile <file.edserialprofile>`、`--last-profile`、`--profile-dir <dir>` |
| 技术栈 | C++17、Qt Widgets、CMake、Ninja、QTest |
| 构建目录 | 仅允许 `build/` |

## 能力成熟度

| 能力域 | 仓库证据 | 工程状态 | 用户路径 | 设备验证 |
|--------|----------|----------|----------|----------|
| Serial Station 串口工站 | `src/apps/serial_station/`、QTest、启动入口 | E5 | U4 | D1 |
| UART 配置链路 | 端口枚举、手动 COM、配置摘要、连接日志 | E4 | U3 | D1 |
| 协议收发与解析 | `ascii_text`、`modbus_rtu`、`custom_md`、`just_float` | E5 | U3 | D1 |
| 测量通道观察 | JustFloat measurement、测量服务、通道摘要面板 | E5 | U3 | D1 |
| 命令历史与配置档案 | 最近命令、`.edserialprofile`、最近/上次/默认目录 | E5 | U4 | D1 |
| 日志、导出、回放预览 | 结构化日志、导出服务、回放服务、UI 闭环 | E4/E5 | U3 | D1 |
| 终端、图表、OTA、BLE/CAN/MQTT/USB/RTT | 阶段性模块和集成代码 | E2-E4 | U1-U3 | D0-D1 |

状态口径：

```text
Engineering: E0 未开始 -> E5 可维护闭环
User path:   U0 不可见 -> U4 完整工作流
Device:      D0 未验证 -> D4 真实设备验证
```

## Serial Station

Serial Station 是当前最活跃的工作台方向，落点为 `src/apps/serial_station/`。它已经覆盖串口配置、协议选择、ASCII/HEX/协议模式发送、结构化日志、导出、回放预览、命令历史、配置档案、最近档案索引、默认档案目录和 JustFloat 测量摘要。

常用入口：

```powershell
.\EmbedDebug.bat --station serial
.\EmbedDebug.bat --station serial --profile .\profiles\line-a.edserialprofile
.\EmbedDebug.bat --last-profile
.\EmbedDebug.bat --station serial --profile-dir .\profiles\line-a
```

协议支持：

| 协议 | 用途 |
|------|------|
| `ascii_text` | 文本终端、AT 类命令 |
| `modbus_rtu` | 基础 Modbus RTU 主站请求与响应解析 |
| `custom_md` | 自定义 MCU 调试帧 |
| `just_float` | 参考 VOFA+ JustFloat 数据路径，解析小端 IEEE754 float 数组 + `00 00 80 7F` 帧尾，并汇总到测量通道面板 |

JustFloat 当前已完成协议注册、流式解析、工作台选择和测量摘要显示；下一步重点是接入波形工作区，并通过虚拟串口或真实硬件样本提升设备验证等级。

## 架构边界

```text
ui/ -> SerialStationController -> core/ + protocols/ + services/
workers/ -> core/
core/ -> ISerialProtocol + SerialProtocolRegistry
protocols/<name>/ -> protocol interface + shared/utils only
services/ -> JSON、日志、导出、回放、档案、测量摘要
```

核心规则：

| 规则 | 要求 |
|------|------|
| 不经 PRD 不写新功能 | 新行为必须先有 PRD/Specs |
| 只允许一个构建目录 | 固定使用 `build/` |
| 不提交死源码 | 新增 `.h/.cpp` 必须加入 CMake |
| 壳层不写业务逻辑 | `MainWindow` 和 `PanelManager` 只装配和导航 |
| 公共能力优先复用 | CRC、HEX、Settings、日志、导出、RingBuffer 等不重复造 |
| 先验证再声明 | 构建、测试、启动探针和状态记录支撑每次提交 |

## 快速开始

首次在已配置的 Windows 工作站启动：

```powershell
.\tools\bootstrap_env.bat
.\EmbedDebug.bat
```

手动配置和构建：

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat
```

常用工具：

```powershell
uv run start-embeddebug
uv run package-embeddebug --skip-build --clean
uv run package-embeddebug --skip-build --clean --zip
uv run verify-package-embeddebug
uv run test-embeddebug-tools
```

## 本地验证

核心验证：

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial
```

Serial Station 聚焦验证：

```powershell
cmake --build build --target test_startup_options test_serial_measurement_service test_serial_measurement_panel test_serial_station_controller test_serial_station_workbench --parallel 4
.\build\tests\test_startup_options.exe
.\build\tests\test_serial_measurement_service.exe
.\build\tests\test_serial_measurement_panel.exe
.\build\tests\test_serial_station_controller.exe
.\build\tests\test_serial_station_workbench.exe
```

协议验证：

```powershell
cmake --build build --target test_ascii_text_protocol test_modbus_rtu_protocol test_custom_md_protocol test_just_float_protocol test_serial_protocol_registry --parallel 4
ctest --test-dir build -R "AsciiTextProtocol|ModbusRtuProtocol|CustomMdProtocol|JustFloatProtocol|SerialProtocolRegistry" --output-on-failure
```

## 仓库结构

```text
GS_Tool/
|-- src/
|   |-- apps/serial_station/   # Serial Station 串口工站
|   |-- core/                  # 应用协调和共享 UI 运行时
|   |-- connection/            # 连接实现与工厂
|   |-- protocol/              # 协议引擎、桥接、Schema
|   |-- terminal/ chart/       # 终端、波形和数据呈现
|   |-- ota/ automation/       # OTA 与自动化
|   |-- utils/ shared/         # 复用工具和轻量类型
|   `-- interfaces/            # 纯接口
|-- tests/                     # QTest 目标
|-- resources/                 # 主题、图标、翻译和资源
|-- docs/                      # 约束、PRD、Specs、评审、追踪
|-- tools/                     # bootstrap、打包、审计和启动工具
|-- cmake/EmbedDebugSources.cmake
|-- CMakeLists.txt
|-- EmbedDebug.bat
`-- README.md
```

## 路线图

| 优先级 | 方向 | 目标 |
|--------|------|------|
| P0 | Serial Station 连接策略 | 补可控连接、异常恢复和现场可诊断反馈 |
| P0 | VOFA+/OmniProbe 式数据观察 | 将 JustFloat measurement 接入波形工作区和虚拟串口样本验证 |
| P0 | 虚拟串口或硬件回环验证 | 将设备证据从 D1 推向更接近真实现场 |
| P1 | QSS token 与 UI 一致性 | 降低主题漂移，统一控件层级 |
| P1 | 发布包体验 | 强化 `dist/` 校验、随包文档和启动路径 |

## 贡献方式

欢迎提交 Issue 和 Pull Request，尤其是以下方向：

- 串口协议适配、真实设备样本、虚拟串口验证脚本。
- VOFA+/OmniProbe 式波形观察、测量面板和数据回放体验。
- Qt Widgets UI 一致性、QSS 主题、图标和可访问性改进。
- Windows 构建、打包、启动和交付链路优化。

提交 PR 前请先阅读 [CLAUDE.md](CLAUDE.md) 与 `docs/constraints/` 下的相关约束文档。新增功能需要配套 PRD/Specs、CMake 注册和可运行验证；涉及真实设备的结论请写明设备、端口、样本和复现步骤。

项目联系邮箱：1264206065@qq.com

## License

MIT License.
