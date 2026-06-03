# 07 - 项目目录结构

> 本文档是 EmbedDebug 约束体系的第7模块。涉及文件创建/移动时必须加载。

---

## 一、根目录结构

```
User_Serial/
├── CMakeLists.txt                 # CMake构建配置
├── CLAUDE.md                      # 约束文档主入口（索引+铁律）
├── README.md                      # 企业级项目文档
├── EmbedDebug.bat                 # 双击启动脚本
├── embeddebug_settings.json       # 运行时配置
├── docs/                          # 文档目录
│   ├── constraints/               # 约束文档模块（7个文件）
│   ├── prd/                       # PRD需求文档
│   │   ├── active/                # 当前有效的PRD
│   │   ├── archive/               # 已完成的PRD归档
│   │   └── rejected/              # 被驳回的特性提案
│   ├── architecture/              # 架构文档（设计+审查）
│   ├── reviews/                   # 代码审查 + UI/UX审查 + QA报告
│   └── tracking/                  # 评分追踪
├── src/                           # 源代码
├── resources/                     # 资源文件
└── tests/                         # 测试
```

---

## 二、src/ 模块级目录结构

> 详细文件清单见本文档历史版本。此处仅展示模块级组织。
> 状态标记: ✅ 已有代码 | 🔄 规划中

```
src/
├── interfaces/        🔄 [规划中] 纯虚接口
│   ├── IConnection.h
│   ├── IPanelProvider.h
│   ├── IDataSink.h
│   ├── IProtocolParser.h
│   └── IDevice.h
│
├── shared/            🔄 [规划中] 共享常量+枚举
│   ├── ColorConstants.h
│   ├── LayoutConstants.h
│   ├── FontConstants.h
│   ├── AnimationConstants.h
│   ├── IconConstants.h
│   └── ComponentConstants.h
│
├── core/              ✅ 核心编排（MainWindow, PanelManager, ThemeManager...）
│   ├── mainwindow/    MainWindow + 信号连接 + 面板连接
│   ├── panels/        PanelManager（面板创建/注册/映射）
│   ├── widgets/       BasePanel, EmptyStateWidget, LoadingSpinner, SkeletonWidget,
│   │                  CommandPalette, SmartAutoComplete, ScriptRecorder,
│   │                  DataDiffWidget, IconNavBar, IconManager, AnimatedButton, ToastWidget
│   ├── send/          SendController
│   ├── theme/         ThemeManager, Constants（6个常量域）
│   ├── navigation/    NavigationController + 动画 + 指示器
│   ├── recording/     RecordingController
│   ├── toolbar/       ToolbarController
│   ├── settings/      SettingsController, SessionManager
│   ├── connect/       ConnectionController, ConnectionManager, ConnectionFactory
│   ├── terminal/      TerminalController
│   └── device/        DeviceProfile
│
├── serial/            ✅ 串口功能
│   ├── config/        SerialConfigPanel
│   ├── commands/      QuickCommandBar, TimedSender, SendHistory
│   ├── data/          DataStatistics, BookmarkWidget
│   ├── signals/       SignalMonitor
│   └── port/          PortWatcher, SerialDriverDetector
│
├── connection/        ✅ 连接管理（13种连接方式）
│   ├── interface/     IConnection
│   ├── serial_port/   SerialConnection
│   ├── network/       TcpConnection, UdpConnection
│   ├── ble/           BleConnection (规划中)
│   ├── can/           CanConnection (规划中)
│   ├── mqtt/          MqttConnection (规划中)
│   ├── tcp/           TcpConnectionEx (规划中)
│   ├── spi_i2c/       SpiI2cConnection (规划中)
│   ├── ws/            WebSocketConnection (规划中)
│   └── usb/           UsbConnection (规划中)
│
├── protocol/          ✅ 协议解析（Modbus, DLT645, Custom...）
│   ├── parser/        FrameDefinition, FrameParser
│   ├── editor/        FrameVisualEditor
│   ├── bridge/        IProtocolBridge, JustFloatBridge, FireWaterBridge
│   ├── view/          ProtocolView
│   ├── modbus/        ModbusEngine (规划中)
│   ├── protobuf/      ProtobufEngine (规划中)
│   └── hex/           IntelHexParser
│
├── terminal/          ✅ 终端显示
│   ├── widget/        TerminalWidget（自绘制）
│   ├── model/         TerminalModel
│   ├── search/        TerminalSearchBar, SearchManager, SearchRenderer
│   ├── selection/     SelectionManager
│   ├── layout/        LayoutManager
│   ├── menu/          ContextMenuManager
│   ├── filter/        TerminalFilter (规划中)
│   └── types/         TerminalTypes, DirectionFilter
│
├── chart/             ✅ 数据图表
│   ├── widget/        ChartWidget, ChartColors
│   ├── model/         ChartModel, ChannelConfig
│   ├── overlay/       CursorOverlay
│   ├── zoom/          ZoomController
│   ├── fft/           FftAnalyzer (规划中)
│   └── stats/         ScatterPlot, HistogramWidget (规划中)
│
├── ota/               ✅ 固件升级
│   ├── manager/       OtaManager
│   ├── widget/        OtaWidget, AnimatedProgressBar
│   ├── history/       OtaHistoryModel
│   └── protocols/     BaseTransfer, XModem, YModem, ZModem
│
├── rtt/               🔄 SEGGER RTT（目录已创建，代码待实现）
│   └── RttConnection.h/cpp
│
├── automation/        🔄 脚本自动化（目录已创建，代码待实现）
│   └── TriggerEngine.h/cpp
│
├── dashboard/         🔄 仪表盘（目录已创建，代码待实现）
│   └── DashboardWidget.h/cpp
│
├── plugin/            🔄 插件系统（目录已创建，代码待实现）
│   └── PluginManager.h/cpp
│
└── utils/             ✅ 工具函数
    ├── crypto/        CRC, HexConverter
    ├── data/          RingBuffer, ByteFormat, DataBookmark
    ├── export/        DataExporter（含流式/EDL格式）
    ├── log/           DataLogger（含EDL格式）
    ├── settings/      SettingsManager
    ├── checksum/      ChecksumCalculator (规划中)
    ├── converter/     DataConverter (规划中)
    ├── timestamp/     TimestampFormatter (规划中)
    ├── packet/        PacketAnalyzer (规划中)
    └── perf/          PerfMonitor (规划中)
```

---

## 二-B、模块归属

> 各模块的负责人、代码状态和规模概览。

| 模块 | 路径 | 负责角色 | 状态 | 预估行数 | 说明 |
|------|------|---------|------|---------|------|
| `core` | `src/core/` | 核心开发 | ✅ 活跃 | ~3000+ | 最大的模块，含所有Controller和共享Widget |
| `connection` | `src/connection/` | 协议开发 | ✅ 活跃 | ~800 | IConnection + 3种已实现连接 + 7种规划中 |
| `protocol` | `src/protocol/` | 协议开发 | ✅ 活跃 | ~1500 | 帧解析引擎 + 协议桥 |
| `terminal` | `src/terminal/` | UI开发 | ✅ 活跃 | ~2000 | 自绘制终端控件 |
| `serial` | `src/serial/` | UI开发 | ✅ 活跃 | ~1000 | 串口配置/指令/数据面板 |
| `chart` | `src/chart/` | UI开发 | ✅ 活跃 | ~800 | 基础波形图 |
| `ota` | `src/ota/` | 核心开发 | ✅ 活跃 | ~1200 | OTA管理 + 3种传输协议 |
| `utils` | `src/utils/` | 核心开发 | ✅ 活跃 | ~800 | 公共工具库 |
| `rtt` | `src/rtt/` | 协议开发 | 🔄 规划中 | 0 | J-Link RTT，待实现 |
| `automation` | `src/automation/` | 核心开发 | 🔄 规划中 | 0 | 触发器引擎，待实现 |
| `dashboard` | `src/dashboard/` | UI开发 | 🔄 规划中 | 0 | 仪表盘，待实现 |
| `plugin` | `src/plugin/` | 系统架构师 | 🔄 规划中 | 0 | 插件系统，待实现 |
| `interfaces` | `src/interfaces/` | 系统架构师 | 🔄 规划中 | 0 | 纯虚接口，解耦Phase 1 |
| `shared` | `src/shared/` | 系统架构师 | 🔄 规划中 | 0 | 共享常量，解耦Phase 2 |

---

## 三、resources/ 目录

```
resources/
├── icons/                         # 图标资源
│   └── lucide/                    #   Lucide图标库 (MIT)
│       ├── cable.svg
│       ├── bluetooth.svg
│       └── ...                    #   ~50个SVG图标
├── themes/                        # QSS主题文件
│   ├── dark_terminal.qss          #   暗色终端风
│   ├── modern_dark.qss            #   现代深色
│   └── light.qss                  #   浅色
├── translations/                  # 翻译文件
└── app.qrc                        # Qt资源文件
```

---

## 四、docs/ 目录

```
docs/
├── constraints/                   # 约束文档（8个模块）
│   ├── 01-project-overview.md
│   ├── 02-workflow.md
│   ├── 03-architecture.md
│   ├── 04-coding-standard.md
│   ├── 05-ui-standard.md
│   ├── 06-git-commit.md
│   ├── 07-directory-structure.md  # 本文件
│   └── 08-icon-standard.md
├── prd/                           # PRD需求文档
├── architecture/                  # 架构设计+审查
├── reviews/                       # 代码/UI/QA审查
└── tracking/                      # 评分追踪
```
