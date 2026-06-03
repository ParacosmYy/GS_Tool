# 07 - 项目目录结构

> 本文档是 EmbedDebug 约束体系的第7模块。涉及文件创建/移动时必须加载。

---

## 一、根目录结构

```
User_Serial/
├── CMakeLists.txt                 # CMake 构建配置
├── CLAUDE.md                      # 约束文档主入口（索引+铁律）
├── README.md                      # 项目说明文档
├── EmbedDebug.bat                 # 双击启动脚本
├── embeddebug_settings.json       # 运行时配置
├── docs/                          # 文档目录
│   ├── constraints/               # 约束文档模块
│   ├── prd/                       # PRD 需求文档
│   ├── architecture/              # 架构文档（设计+审查）
│   ├── reviews/                   # 代码审查 + UI/UX 审查 + QA 报告
│   └── tracking/                  # 评分追踪
├── resources/                     # 资源文件
├── src/                           # 源代码
└── tests/                         # 测试
```

---

## 二、src/ 顶层目录

> 下面按当前仓库真实目录写法整理。`shared/` 是正式目标层，但仓库里还没有独立目录，当前仍由 `core/theme/Constants.h` 兼容承接。

| 目录 | 当前状态 | 角色口径 | 说明 |
|------|----------|----------|------|
| `interfaces/` | 已存在 | 纯虚接口层 | 放 `IConnection`、`IPanelProvider`、`IDataSink` 等契约 |
| `shared/` | 当前未创建 | 公共基础层 | 共享常量、枚举、轻量类型的唯一真相路径 |
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

---

## 四、唯一真相路径与收敛规则

| 主题 | 口径 | 约束 |
|------|------|------|
| 共享常量与枚举 | `src/shared/` | 新增跨模块常量、枚举、轻量类型优先放这里 |
| 旧常量伞头 | `src/core/theme/Constants.h` | 只允许转发旧 include，不允许新增域定义 |
| 应用协调入口 | `src/core/mainwindow/MainWindow.*` | 只负责初始化、组装 UI、连接信号/槽 |
| 面板编排中心 | `src/core/panels/PanelManager.*` | 只负责面板创建、注册、包装、映射和统计 |
| 基础 UI 组件 | `src/core/widgets/` | 只放可复用壳层，不放功能桶里的业务逻辑 |
| 主题运行时 | `src/core/theme/ThemeManager.*` | 只管主题切换、QSS 加载和主题状态 |

### 收敛建议

1. `shared/` 先落地，再把新常量从 `core/theme/Constants.h` 迁出去。
2. `core/` 只保留协调、导航、基础 UI 和会话管理，不再吸纳新功能桶。
3. `MainWindow` 和 `PanelManager` 继续做编排，但新增业务流必须优先下沉到独立 Controller/Manager。
4. `font/` 与 `fonts/`、`icon/` 与 `icons/`、`responsive/` 与 `layout/`、`shortcut/` 与 `managers/`、`widgets/` 与 `widgets2/`、`animation/` 与 `animation2/`、`loader/` 与 `loader2/` 这些分叉目录只允许冻结，不允许继续复制新分支。
5. 目录命名优先沿用已有主线目录，不要再创造“更像”的新桶。

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
├── prd/                           # PRD 需求文档
├── architecture/                  # 架构设计+审查
├── reviews/                       # 代码/UI/QA 审查
└── tracking/                      # 评分追踪
```
