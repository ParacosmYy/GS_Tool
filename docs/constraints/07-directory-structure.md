# 07 - 项目目录结构

> 本文档是 EmbedDebug 约束体系的第7模块。涉及文件创建/移动时必须加载。

---

## 一、根目录结构

```
User_Serial/
├── CMakeLists.txt                 # CMake 构建配置
├── AGENTS.md                      # Agent 强制入口（索引+稳定规则）
├── CLAUDE.md                      # 约束文档主入口（索引+铁律）
├── README.md                      # 项目说明文档
├── EmbedDebug.bat                 # 双击启动脚本；最低运行入口，必须保持可用
├── Beta.bat                       # 兼容启动入口，调用 EmbedDebug.bat
├── embeddebug_settings.json       # 运行时配置
├── docs/                          # 文档目录
│   ├── constraints/               # 约束文档模块
│   ├── prd/                       # PRD 需求文档
│   ├── architecture/              # 架构文档（设计+审查）
│   ├── reviews/                   # 代码审查 + UI/UX 审查 + QA 报告
│   └── tracking/                  # 评分追踪
├── tools/                         # 本地开发、启动、审计和 Agent 执行工具
├── resources/                     # 资源文件
├── src/                           # 源代码
└── tests/                         # 测试
```

`EmbedDebug.bat` 是用户侧最低运行入口，不属于可随意替换的临时脚本。任何改变构建输出目录、可执行文件名、Qt 部署路径或启动参数的改动，都必须同步检查并验证该 bat 双击启动链路。
`Beta.bat` 仅保留兼容调用，不得承载独立构建/部署逻辑。

构建目录只能有一个：`build/`。根目录下禁止出现或引用 `build2/`、`build-debug/`、`build-release/`、`cmake-build-*` 等平行构建目录；`EmbedDebug.bat` 也不得为这些目录保留 fallback。

---

## 二、src/ 顶层目录

> 下面按当前仓库真实目录写法整理。`shared/` 与 `interfaces/` 已落地为正式基础层，`core/theme/Constants.h` 仅保留兼容承接。

| 目录 | 当前状态 | 角色口径 | 说明 |
|------|----------|----------|------|
| `apps/` | 规划新增 | 独立工站 app 层 | 新串口工站等可独立演进的工作站模块 |
| `interfaces/` | 已存在 | 纯虚接口层 | 放 `IConnection`、`IPanelProvider`、`IDataSink` 等契约 |
| `shared/` | 已存在 | 公共基础层 | 共享常量、枚举、轻量类型的唯一真相路径 |
| `core/` | 已存在 | 应用协调 + 基础 UI | `MainWindow`、`PanelManager`、`ThemeManager`、导航、基础 Widget |
| `serial/` | 已存在 | 串口功能模块 | 配置、指令、数据、端口、信号线 |
| `connection/` | 已存在 | 连接接入模块 | 串口、网络、BLE、CAN、MQTT、USB、WS 等连接实现 |
| `protocol/` | 已存在 | 协议解析模块 | 帧解析、协议桥、协议编辑、Modbus、Hex、Schema |
| `terminal/` | 已存在 | 终端显示模块 | 自绘终端、搜索、选择、过滤、布局 |
| `chart/` | 已存在 | 图表展示模块 | 波形、FFT、热力图、统计图、游标、缩放 |
| `ota/` | 已存在 | 固件升级模块 | OTA 管理、历史、传输协议、升级界面 |
| `rtt/` | 已存在 | RTT 接入模块 | J-Link SDK 适配和 RTT 通道管理 |
| `automation/` | 已存在 | 自动化模块 | 触发器与脚本自动化 |
| `dashboard/` | 已存在 | 仪表盘模块 | Gauge、LED、数值、进度条等可视化控件 |
| `plugin/` | 已存在 | 插件扩展模块 | 插件管理、加载器、配置面板 |
| `utils/` | 已存在 | 通用工具层 | 编解码、导出、日志、缓存、性能、时间戳等 |
| `widgets/` | 已存在 | 独立复用控件库 | autocomplete、dialog、diff、palette、recorder、toast 等 |
| `features/` | 迁移骨架 | 未来功能入口 | 只放归属说明和迁移骨架，不承载现有实现 |

---

## 三、真实子目录与分叉现状

> 这里不写“理想树”，只写当前仓库里已经出现的真实子目录。带 `2` 的目录、同义复数目录、重复能力目录都属于历史分叉，后续只能收敛，不要继续扩张。

| 模块 | 当前真实子目录 |
|------|----------------|
| `core/` | `animation/`, `animation2/`, `background/`, `clipboard/`, `connect/`, `device/`, `event/`, `factory/`, `font/`, `fonts/`, `icon/`, `icons/`, `iconprovider/`, `layout/`, `mainwindow/`, `managers/`, `navigation/`, `notification/`, `panels/`, `pipeline/`, `recording/`, `responsive/`, `send/`, `shortcut/`, `terminal/`, `theme/`, `toolbar/`, `widgets/`, `widgets2/`, `workspace/` |
| `connection/` | `ble/`, `can/`, `interface/`, `monitor/`, `mqtt/`, `network/`, `pool/`, `serial_port/`, `spi_i2c/`, `tcp/`, `usb/`, `ws/` |
| `protocol/` | `bridge/`, `can/`, `editor/`, `engine/`, `hex/`, `modbus/`, `parser/`, `protobuf/`, `schema/`, `view/` |
| `serial/` | `commands/`, `config/`, `data/`, `detector/`, `port/`, `signals/` |
| `terminal/` | `filter/`, `layout/`, `menu/`, `model/`, `search/`, `selection/`, `types/`, `widget/` |
| `chart/` | `fft/`, `heatmap/`, `heatmap2/`, `model/`, `overlay/`, `scale/`, `stats/`, `waterfall/`, `widget/`, `zoom/` |
| `ota/` | `history/`, `manager/`, `protocols/`, `widget/` |
| `rtt/` | 平铺结构，直接放连接/SDK/通道管理相关类 |
| `automation/` | 平铺结构，直接放触发器引擎与列表面板 |
| `dashboard/` | 平铺结构，直接放仪表盘模型与控件 |
| `plugin/` | `loader/`, `loader2/` |
| `utils/` | `aggregator/`, `checksum/`, `clipboard/`, `converter/`, `crypto/`, `data/`, `export/`, `log/`, `packet/`, `perf/`, `pipeline/`, `timestamp/` |
| `widgets/` | `audio/`, `autocomplete/`, `dialog/`, `diff/`, `freq/`, `palette/`, `recorder/`, `scope/`, `toast/` |
| `features/` | 迁移骨架目录，默认平铺，后续仅承接归属说明 |

### Serial Station 目标子目录

> 这是 C++/Qt 目标结构，不是 Python 目录。所有生产文件使用 `.h/.cpp`，测试使用 QTest。

```text
src/apps/serial_station/
├── SerialStationApp.h/.cpp
├── SerialStationWindow.h/.cpp
├── SerialStationController.h/.cpp
├── SerialStationModels.h
├── SerialStationConstants.h
├── SerialStationConfig.h/.cpp
├── ui/
│   ├── SerialMainPanel.h/.cpp
│   ├── SerialPortPanel.h/.cpp
│   ├── SerialProtocolPanel.h/.cpp
│   ├── SerialLogPanel.h/.cpp
│   ├── SerialCommandPanel.h/.cpp
│   └── SerialStatusBar.h/.cpp
├── core/
│   ├── SerialPort.h/.cpp
│   ├── SerialManager.h/.cpp
│   ├── SerialSession.h/.cpp
│   ├── SerialDispatcher.h/.cpp
│   ├── SerialCodec.h/.cpp
│   └── SerialError.h
├── protocols/
│   ├── ISerialProtocol.h
│   ├── SerialProtocolRegistry.h/.cpp
│   ├── SerialProtocolEvent.h
│   ├── modbus_rtu/
│   ├── custom_md/
│   └── ascii_text/
├── services/
│   ├── SerialLogService.h/.cpp
│   ├── SerialExportService.h/.cpp
│   ├── SerialReplayService.h/.cpp
│   └── DeviceProfileService.h/.cpp
└── workers/
    ├── SerialReaderWorker.h/.cpp
    └── SerialCommandWorker.h/.cpp
```

对应测试目录：

```text
tests/serial_station/
├── test_serial_manager.cpp
├── test_serial_dispatcher.cpp
├── test_serial_protocol_registry.cpp
├── test_modbus_rtu_protocol.cpp
├── test_custom_md_protocol.cpp
└── test_ascii_text_protocol.cpp
```

---

## 四、唯一真相路径与收敛规则

| 主题 | 口径 | 约束 |
|------|------|------|
| 共享常量与枚举 | `src/shared/` | 新增跨模块常量、枚举、轻量类型优先放这里 |
| 旧常量伞头 | `src/core/theme/Constants.h` | 只允许转发旧 include，不允许新增域定义 |
| 未来功能骨架 | `src/features/` | 仅作为迁移骨架和未来归属说明，不承载现有实现 |
| 应用协调入口 | `src/core/mainwindow/MainWindow.*` | 只负责初始化、组装 UI、连接信号/槽 |
| Serial Station 新工站 | `src/apps/serial_station/` | 新串口上位机重构落点，内部按 ui/controller/core/protocols/services/workers 分层 |
| 面板编排中心 | `src/core/panels/PanelManager.*` | 只负责面板创建、注册、包装、映射和统计 |
| 基础 UI 组件 | `src/core/widgets/` | 只放可复用壳层，不放功能桶里的业务逻辑 |
| 主题运行时 | `src/core/theme/ThemeManager.*` | 只管主题切换、QSS 加载和主题状态 |

### 收敛建议

1. `shared/` 先落地，再把新常量从 `core/theme/Constants.h` 迁出去。
2. `core/` 只保留协调、导航、基础 UI 和会话管理，不再吸纳新功能桶。
3. `MainWindow` 和 `PanelManager` 继续做编排，但新增业务流必须优先下沉到独立 Controller/Manager。
4. `font/` 与 `fonts/`、`icon/` 与 `icons/`、`responsive/` 与 `layout/`、`shortcut/` 与 `managers/`、`widgets/` 与 `widgets2/`、`animation/` 与 `animation2/`、`loader/` 与 `loader2/` 这些分叉目录只允许冻结，不允许继续复制新分支。
5. 目录命名优先沿用已有主线目录，不要再创造“更像”的新桶。

### 四-A、目标骨架

> 这部分描述兼容迁移阶段的目标落点，不要求一次性移动现有源码。

| 目录 | 角色 | 新增规则 |
|------|------|----------|
| `src/shared/` | 共享基础层 | 新常量、枚举、轻量值类型优先落这里 |
| `src/interfaces/` | 契约层 | 纯接口、抽象协议、回调类型放这里 |
| `src/core/` | 应用协调层 | 只保留装配、导航、主题、基础 UI、会话 |
| `src/features/` | 迁移骨架层 | 只做归属说明、骨架 README、未来功能入口说明 |
| `src/apps/serial_station/` | 新串口工站层 | 按专项文档落地，不回流到旧 `src/serial/` |
| `src/connection/` 等现有业务模块 | 业务实现层 | 继续按领域收敛，不再新建平行实现目录 |

新增目录规则:
- 新功能优先寻找 canonical 目录，不要新建 `2`、`new`、`old`、`backup` 之类平行目录。
- 如果历史分叉已经存在，只能冻结，不能继续复制。
- 如果确实需要未来迁移入口，先在 `src/features/` 里放归属说明，再讨论是否新增具体目录。
- 如果是串口上位机重构或新增串口业务协议，优先进入 `src/apps/serial_station/`，不要继续扩张旧 `src/serial/` 和 `src/protocol/` 的耦合点。

### 四-B、冻结目录

以下目录仅保留兼容和历史引用，不再承载新实现：

- `animation2/`
- `widgets2/`
- `loader2/`
- `fonts/`
- `icons/`
- `responsive/`
- `font/` 与 `fonts/`
- `icon/` 与 `icons/`
- `shortcut/` 与 `managers/`

冻结规则:
- 可以保留旧 include、旧资源引用和转发适配。
- 不允许把新需求继续写入冻结目录。
- 任何新骨架都应先落到 `src/features/` 或 canonical 路径。

---

## 五、resources/ 目录

```
resources/
├── icons/                         # 图标资源
│   └── lucide/                    #   Lucide 图标库 (MIT)
│       ├── cable.svg
│       ├── bluetooth.svg
│       └── ...
├── themes/                        # QSS 主题文件
│   ├── dark_terminal.qss          #   暗色终端风
│   ├── modern_dark.qss            #   现代深色
│   └── light.qss                  #   浅色
├── translations/                  # 翻译文件
└── app.qrc                        # Qt 资源文件
```

---

## 六、docs/ 目录

```
docs/
├── constraints/                   # 约束文档
│   ├── 01-project-overview.md
│   ├── 02-workflow.md
│   ├── 03-architecture.md
│   ├── 04-coding-standard.md
│   ├── 05-ui-standard.md
│   ├── 06-git-commit.md
│   ├── 07-directory-structure.md  # 本文件
│   └── 08-icon-standard.md
├── architecture/                  # 架构骨架与迁移说明
│   ├── README.md
│   ├── target-structure.md
│   ├── frozen-dirs.md
│   ├── module-boundaries.md
│   └── migration-roadmap.md
├── superpowers/                   # Specs、BATCH、LOOP 和执行计划
│   ├── specs/                     # Specs 模板
│   ├── plans/                     # 分步执行计划
│   ├── BATCH_PROTOCOL.md
│   └── LOOP_PROTOCOL.md
├── prd/                           # PRD 需求文档
├── reviews/                       # 代码/UI/QA 审查
│   ├── debug/                     # LOOP Debug 追踪报告
│   └── simplify/                  # LOOP Simplify 只读扫描报告
└── tracking/                      # 评分追踪
```

---

## 七、tools/ 目录

```
tools/
├── bootstrap_env.bat              # 本机环境探测，生成 local_env.bat
├── debug-trace.ps1                # LOOP Debug 追踪报告生成入口
├── doctor.ps1                     # LOOP Doctor 系统体检，只读诊断，构建/启动需显式参数
├── launch_embeddebug.ps1          # EmbedDebug.bat 调用的启动/构建/部署脚本
├── simplify-scan.ps1              # LOOP Simplify 只读扫描入口
├── qss-generator/                 # QSS 生成工具
├── project-audit/                 # 项目审计工具
└── agent-loop/                    # Specs 驱动的 Go 执行循环工具
```

`tools/agent-loop/` 只用于 Agent 迭代执行辅助，不属于 EmbedDebug 产品运行时，不接入 CMake，不生成或引用第二构建目录。
