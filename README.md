# EmbedDebug

> 面向嵌入式研发与调试现场的 Qt 桌面工作台：把串口工站、协议收发、终端日志、波形分析、OTA、录制回放、自动化和工程诊断收敛到一个可验证、可扩展、可交付的应用中。

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52?style=flat-square)
![CMake](https://img.shields.io/badge/build-CMake%20%2B%20Ninja-064F8C?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2B-0078D4?style=flat-square)
![Status](https://img.shields.io/badge/status-active%20engineering%20build-f59e0b?style=flat-square)

![EmbedDebug 界面预览](docs/assets/readme/interface-preview.svg)

## 项目定位

嵌入式调试现场通常被拆散在多个工具里：串口助手、协议解析器、波形查看器、日志记录器、OTA 工具、脚本工具和临时诊断面板。EmbedDebug 的目标是把这些链路整理成一个稳定的工程工作台，让固件、硬件和测试工程师能在同一个入口完成连接配置、命令发送、帧解析、日志留存、会话回放和问题复现。

当前仓库不是对外宣称完成的商业发行版，而是一个持续演进的工程版本。项目使用 PRD/Specs 约束、分层架构边界、QTest 自动化覆盖、Windows 启动探针和评分追踪来管理每一轮迭代。README 中的能力状态只描述已经能在仓库中找到证据的内容；未做真实硬件验证的能力不会被包装成已量产能力。

## 产品快照

| 项目 | 当前状态 |
|------|----------|
| 应用名称 | `EmbedDebug` |
| 主开发分支 | `feat/embed-debug` |
| 用户入口 | 仓库根目录 `EmbedDebug.bat` |
| 串口工站直达 | `EmbedDebug.bat --station serial` |
| 档案化串口工站 | `EmbedDebug.bat --station serial --profile <file.edserialprofile>` |
| 构建系统 | CMake + Ninja，只允许使用 `build/` |
| UI 技术栈 | Qt Widgets、QSS 主题、SVG 图标资源 |
| 工程治理 | PRD、Specs、TDD、启动验证、架构约束、评分追踪 |

## 能力成熟度

| 能力域 | 仓库证据 | 工程状态 | 用户路径 | 设备验证 |
|--------|----------|----------|----------|----------|
| Serial Station 串口工站 | `src/apps/serial_station/`、QTest、README 启动入口 | E5 | U4，档案/日志/命令闭环 | D1，自动化测试 |
| UART 配置链路 | 端口枚举、手动 COM、UART 摘要、连接/断开 UI | E4 | U3 | D1，未声明真实硬件验证 |
| 协议收发与解析 | `ascii_text`、`modbus_rtu`、`custom_md`、registry 测试 | E4 | U3 | D1 |
| 命令历史与配置档案 | 最近命令、`.edserialprofile` 保存/加载、启动加载 | E5 | U4 | D1 |
| 日志、导出、回放预览 | 结构化日志服务、导出服务、回放服务、UI 流程测试 | E4/E5 | U3 | D1 |
| 终端与数据视图 | terminal、chart、FFT、heatmap、histogram/scatter、dashboard 模块 | E3/E4，按模块不同 | U2/U3 | D0-D1 |
| OTA 与文件链路 | X/Y/ZMODEM、HEX/BIN、导出基础设施 | E3/E4 | U2 | D0-D1 |
| BLE/CAN/MQTT/USB/RTT | 框架代码和阶段性集成 | E2-E4，按模块不同 | U1-U2 | D0-D1 |

状态口径遵循项目三轴标准：

```text
Engineering: E0 未开始 -> E5 可维护闭环
User path:   U0 不可见 -> U4 完整工作流
Device:      D0 未验证 -> D4 真实设备验证
```

## Serial Station

Serial Station 是当前最活跃的工作台方向，落点为 `src/apps/serial_station/`。它已经从“串口助手入口”推进到“可保存、可加载、可启动套用配置档案”的调试工站。

典型使用路径：

1. 使用 `.\EmbedDebug.bat --station serial` 直达串口工站。
2. 选择真实端口或手动输入 COM 端口。
3. 配置波特率、数据位、校验位、停止位、流控、DTR 和 RTS。
4. 选择协议：`ascii_text`、`modbus_rtu` 或 `custom_md`。
5. 使用 ASCII、HEX 或协议模式发送命令。
6. 查看 TX/RX/System 日志和状态计数。
7. 导出日志，或生成回放预览。
8. 保存或加载 UART/协议/命令档案。
9. 使用 `.\EmbedDebug.bat --station serial --profile <file.edserialprofile>` 直接打开预配置工站。

内部边界：

```text
ui/ -> SerialStationController -> core/ + protocols/ + services/
workers/ -> core/
core/ -> ISerialProtocol + SerialProtocolRegistry
protocols/<name>/ -> protocol interface + shared/utils only
services/ -> JSON、日志、导出、回放、档案
```

UI 面板不直接解析字节流，不直接写档案文件。协议层不接触 QWidget。`MainWindow` 和 `PanelManager` 只负责装配、导航和面板编排。

## 架构边界

EmbedDebug 使用分层依赖模型。新增代码必须先阅读对应约束文档，并保持依赖方向清晰。

```text
L6   src/core/                 应用协调、主窗口、导航、主题运行时
L5   src/ota/ automation/      场景工作流：OTA、自动化、仪表盘、插件
L4   src/terminal/ chart/ rtt/ 数据呈现、终端、波形和实时观察
L3   src/connection/ protocol/ serial/ 连接实现、协议引擎和历史串口模块
L5A  src/apps/serial_station/  独立串口工站，内部继续分层
L2   src/utils/                CRC、HEX、导出、日志、缓存、算法等复用能力
L1   src/shared/               共享常量、枚举、轻量类型
L0   src/interfaces/           纯接口契约
```

核心规则：

| 规则 | 含义 |
|------|------|
| 不经 PRD 不写新功能 | 新行为必须先有 PRD/Specs |
| 只允许一个构建目录 | 固定使用 `build/`，禁止平行构建目录 |
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

直达 Serial Station：

```powershell
.\EmbedDebug.bat --station serial
```

直达 Serial Station 并套用已保存的 UART/协议/命令档案：

```powershell
.\EmbedDebug.bat --station serial --profile .\profiles\line-a.edserialprofile
```

手动配置和构建：

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat
```

仓库只支持 `build/`。不要创建 `build2/`、`build-debug/`、`build-release/` 或 IDE 自动生成的平行构建目录。

## 工具入口

```powershell
uv run start-embeddebug
uv run package-embeddebug --skip-build --clean
uv run package-embeddebug --skip-build --clean --zip
uv run verify-package-embeddebug
uv run test-embeddebug-tools
```

`uv run start-embeddebug` 最终仍委托 `EmbedDebug.bat`，批处理文件是最低可用启动入口。打包工具复用 `build/EmbedDebug.exe`，执行 Qt 部署，并将忽略提交的产物写入 `dist/`。

## 本地验证

核心验证：

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial
.\EmbedDebug.bat --station serial --profile <file.edserialprofile>
```

Serial Station 聚焦验证：

```powershell
cmake --build build --target test_startup_options test_serial_command_panel test_serial_port_panel test_serial_station_workbench --parallel 4
.\build\tests\test_startup_options.exe
.\build\tests\test_serial_command_panel.exe
.\build\tests\test_serial_port_panel.exe
.\build\tests\test_serial_station_workbench.exe
```

协议验证：

```powershell
cmake --build build --target test_ascii_text_protocol test_modbus_rtu_protocol test_custom_md_protocol test_serial_protocol_registry --parallel 4
ctest --test-dir build -R "AsciiTextProtocol|ModbusRtuProtocol|CustomMdProtocol|SerialProtocolRegistry" --output-on-failure
```

## 仓库结构

```text
GS_Tool/
|-- src/
|   |-- apps/serial_station/   # Serial Station 串口工站
|   |-- core/                  # 应用协调和共享 UI 运行时
|   |-- connection/            # 连接实现与工厂
|   |-- protocol/              # 协议引擎、桥接、Schema
|   |-- terminal/              # 终端模型、过滤、渲染
|   |-- chart/                 # 波形、FFT、游标、热力图、缩放
|   |-- ota/                   # 固件更新工作流
|   |-- automation/            # 触发器和规则自动化
|   |-- dashboard/             # 仪表盘控件
|   |-- rtt/                   # SEGGER RTT 集成层
|   |-- utils/                 # 可复用工具层
|   |-- shared/                # 共享常量和轻量类型
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

## 工程工作流

本仓库刻意采用约束驱动流程，目的是让功能推进、架构边界和验证证据同步增长。

| 阶段 | 作用 |
|------|------|
| PRD | 定义用户问题、边界、非目标和状态目标 |
| Specs | 将 PRD 转成实现约束、调用链和验证命令 |
| TDD | 先补或更新 QTest，再改生产代码 |
| GO loop | 执行、检查、修复、验证，避免盲目扩大范围 |
| Commit | 每个闭环迭代提交一次，写清状态、验证和评分 |

入口文档：

- [CLAUDE.md](CLAUDE.md)
- [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md)
- [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md)
- [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md)
- [docs/serial_station_architecture.md](docs/serial_station_architecture.md)

## 路线图

近期重点是把已有工程体量继续收敛成可靠的一体化工作站。

| 优先级 | 方向 | 目标 |
|--------|------|------|
| P0 | Serial Station 档案列表和一键套用/连接 | 配置档案成为日常工作流，而不是文件导入导出能力 |
| P0 | 虚拟串口或硬件回环验证 | 将串口工站设备证据从 D1 提升到更接近真实现场 |
| P1 | QSS token 生成和 UI 一致性 | 降低手写主题漂移，统一控件层级 |
| P1 | 对话框和错误反馈统一 | 用一致的应用级反馈替代零散消息流 |
| P2 | BLE/CAN/MQTT/USB/RTT 强化 | 将阶段性集成推进到可验证工作流 |
| P2 | 发布包体验打磨 | 让 `dist/` 校验、随包文档和启动路径适合交付 |

## 贡献约束

修改代码前先阅读受影响区域的约束文档。新增功能必须先有 PRD/Specs。新增 `.h/.cpp` 必须加入 CMake。任何影响构建、启动、入口、资源、依赖或路径的改动，收口前必须验证 `EmbedDebug.bat`，无法验证时需要说明具体原因。

## License

MIT License.
