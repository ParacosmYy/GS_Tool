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

## 二、src/ 目录结构（与实际代码同步）

```
src/
├── main.cpp
│
├── core/                              # 应用核心层
│   ├── mainwindow/                    #   主窗口组装
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   ├── MainWindowSignalConnect.cpp
│   │   └── MainWindowPanelConnect.cpp
│   ├── background/                    #   背景外观
│   │   ├── BackgroundWidget.h/cpp
│   │   └── BackgroundSettingsPopup.h/cpp
│   ├── navigation/                    #   导航控制
│   │   ├── NavigationController.h/cpp
│   │   ├── NavigationControllerAnimations.cpp
│   │   └── NavIndicatorWidget.h
│   ├── connect/                       #   连接编排（应用层）
│   │   ├── ConnectionController.h/cpp
│   │   ├── ConnectionControllerHandlers.cpp
│   │   ├── ConnectionControllerReconnect.cpp
│   │   ├── ConnectionManager.h/cpp
│   │   └── ConnectionFactory.h/cpp
│   ├── send/                          #   发送逻辑编排
│   │   └── SendController.h/cpp
│   ├── toolbar/                       #   工具栏编排
│   │   └── ToolbarController.h/cpp
│   ├── recording/                     #   录制编排
│   │   └── RecordingController.h/cpp
│   ├── settings/                      #   设置编排
│   │   ├── SettingsController.h/cpp
│   │   └── SessionManager.h/cpp
│   ├── terminal/                      #   终端编排
│   │   └── TerminalController.h/cpp
│   ├── panels/                        #   面板管理
│   │   └── PanelManager.h/cpp
│   ├── theme/                         #   外观系统
│   │   ├── ThemeManager.h/cpp
│   │   └── Constants.h
│   └── widgets/                       #   共享自定义控件
│       ├── AnimatedButton.h/cpp
│       └── ToastWidget.h
│
├── connection/                        # 基础设施层: 连接抽象
│   ├── interface/                     #   连接接口
│   │   └── IConnection.h
│   ├── serial_port/                   #   串口连接
│   │   ├── SerialConnection.h/cpp
│   │   └── SerialConnectionError.cpp
│   └── network/                       #   网络连接
│       ├── TcpConnection.h/cpp
│       └── UdpConnection.h/cpp
│
├── terminal/                          # 表现层: 终端显示
│   ├── widget/                        #   终端控件核心
│   │   ├── TerminalWidget.h/cpp
│   │   └── TerminalWidgetPaint.cpp
│   ├── model/                         #   终端数据模型
│   │   └── TerminalModel.h/cpp
│   ├── search/                        #   搜索功能
│   │   ├── TerminalSearchBar.h/cpp
│   │   ├── TerminalSearchManager.h/cpp
│   │   └── TerminalSearchRenderer.h/cpp
│   ├── selection/                     #   文本选择
│   │   └── TerminalSelectionManager.h/cpp
│   ├── layout/                        #   布局管理
│   │   └── TerminalLayoutManager.h/cpp
│   ├── menu/                          #   右键菜单
│   │   └── TerminalContextMenuManager.h/cpp
│   └── types/                         #   类型定义
│       ├── TerminalTypes.h
│       └── DirectionFilter.h/cpp
│
├── serial/                            # 表现层: 串口功能UI
│   ├── config/                        #   配置面板
│   │   ├── SerialConfigPanel.h/cpp
│   │   └── SerialConfigPanelUI.cpp
│   ├── commands/                      #   快捷指令和定时发送
│   │   ├── QuickCommandBar.h/cpp
│   │   ├── TimedSender.h/cpp
│   │   └── SendHistory.h/cpp
│   ├── data/                          #   数据显示控件
│   │   ├── DataStatistics.h/cpp
│   │   └── BookmarkWidget.h/cpp
│   └── port/                          #   端口监控
│       ├── PortWatcher.h/cpp
│       └── SerialDriverDetector.h/cpp
│
├── protocol/                          # 业务层: 协议解析
│   ├── parser/                        #   帧解析引擎
│   │   ├── FrameDefinition.h/cpp
│   │   ├── FrameParser.h/cpp
│   │   ├── FrameParserStateHandlers.cpp
│   │   └── FrameParserHelpers.cpp
│   ├── editor/                        #   可视化帧编辑器
│   │   ├── FrameVisualEditor.h/cpp
│   │   └── FrameVisualEditorUI.cpp
│   ├── bridge/                        #   协议桥系统
│   │   ├── IProtocolBridge.h
│   │   ├── JustFloatBridge.h/cpp
│   │   ├── FireWaterBridge.h/cpp
│   │   └── ProtocolBridgeManager.h/cpp
│   ├── view/                          #   协议结果显示
│   │   └── ProtocolView.h/cpp
│   └── hex/                           #   Intel HEX解析
│       └── IntelHexParser.h/cpp
│
├── chart/                             # 表现层: 波形显示
│   ├── widget/                        #   波形控件
│   │   ├── ChartWidget.h/cpp
│   │   └── ChartColors.h
│   ├── model/                         #   数据模型
│   │   ├── ChartModel.h/cpp
│   │   └── ChannelConfig.h/cpp
│   ├── overlay/                       #   游标叠加层
│   │   └── CursorOverlay.h/cpp
│   └── zoom/                          #   缩放控制
│       └── ZoomController.h/cpp
│
├── ota/                               # 业务层: OTA升级
│   ├── manager/                       #   OTA调度管理器
│   │   └── OtaManager.h/cpp
│   ├── widget/                        #   OTA操作面板
│   │   ├── OtaWidget.h/cpp
│   │   ├── OtaWidgetSlots.cpp
│   │   └── AnimatedProgressBar.h
│   ├── history/                       #   OTA历史记录
│   │   └── OtaHistoryModel.h/cpp
│   └── protocols/                     #   传输协议
│       ├── base/                      #     模板方法基类
│       │   └── BaseTransfer.h/cpp
│       ├── xmodem/                    #     XMODEM
│       │   ├── XModemTransfer.h/cpp
│       │   └── XModemTransferHandlers.cpp
│       ├── ymodem/                    #     YMODEM
│       │   ├── YModemTransfer.h/cpp
│       │   └── YModemTransferHandlers.cpp
│       └── zmodem/                    #     ZMODEM
│           ├── ZModemTransfer.h/cpp
│           └── ZModemTransferHandlers.cpp
│
└── utils/                             # 基础设施层: 公共工具
    ├── crypto/                        #   校验工具
    │   ├── CRC.h
    │   └── HexConverter.h
    ├── data/                          #   数据格式工具
    │   ├── RingBuffer.h
    │   ├── ByteFormat.h
    │   └── DataBookmark.h
    ├── export/                        #   数据导出子系统
    │   ├── DataExporter.h/cpp
    │   ├── DataExporterStreamed.cpp
    │   └── DataExporterEdl.cpp
    ├── log/                           #   数据日志子系统
    │   ├── DataLogger.h/cpp
    │   └── DataLoggerEdl.cpp
    └── settings/                      #   配置持久化
        └── SettingsManager.h/cpp
```

---

## 三、resources/ 目录

```
resources/
├── icons/                         # 图标资源
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
├── constraints/                   # 约束文档（7个模块）
│   ├── 01-project-overview.md
│   ├── 02-workflow.md
│   ├── 03-architecture.md
│   ├── 04-coding-standard.md
│   ├── 05-ui-standard.md
│   ├── 06-git-commit.md
│   └── 07-directory-structure.md  # 本文件
├── prd/                           # PRD需求文档
├── architecture/                  # 架构设计+审查
├── reviews/                       # 代码/UI/QA审查
└── tracking/                      # 评分追踪
```
