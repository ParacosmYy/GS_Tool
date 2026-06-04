<div align="center">

# EmbedDebug

**企业级嵌入式调试工作站**

*串口终端 | SEGGER RTT | OTA 固件升级 | 协议分析 | 波形引擎 | 数据仪表盘*

[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)]()
[![Qt 6.8](https://img.shields.io/badge/Qt-6.8.3-41CD52?logo=qt&logoColor=white)]()
[![CMake](https://img.shields.io/badge/CMake-4.0-064F8C?logo=cmake&logoColor=white)]()
[![GCC 14.2](https://img.shields.io/badge/GCC-14.2_MinGW-FF8C00?logo=gnu&logoColor=white)]()
[![Platform](https://img.shields.io/badge/Platform-Windows_10%2B-0078D6?logo=windows&logoColor=white)]()
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Commits](https://img.shields.io/badge/Commits-310-8B5CF6.svg)]()
[![Score](https://img.shields.io/badge/Quality_Score-295%2F1000-EAB308.svg)]()

</div>

---

## 目录

- [项目概述](#项目概述)
- [核心特性](#核心特性)
- [系统架构](#系统架构)
- [模块索引](#模块索引)
- [快速上手](#快速上手)
- [质量指标](#质量指标)
- [开发路线图](#开发路线图)
- [贡献指南](#贡献指南)
- [常见问题](#常见问题)
- [开源协议](#开源协议)

---

## 项目概述

EmbedDebug 是一款面向嵌入式系统工程师的一体化调试工作站。它将串口通信、协议分析、实时波形可视化、OTA 固件升级和仪表盘监控整合到单一应用中，消除了在 WindTerm、sscom、VOFA+、SEGGER RTT Viewer 和 Serial Studio 等工具之间频繁切换的痛点。

**核心优势：**

- **统一连接抽象** -- 串口、TCP、UDP、TLS、WebSocket、BLE 自由切换，工作流不变
- **QPainter 自绘终端** -- 绕过 QTextEdit 渲染瓶颈，高波特率下依然流畅
- **状态机协议解析器** -- 实时帧检测，可视化字段编辑器，5 种内置协议模板
- **高级波形引擎** -- FFT 频谱分析、多 Y 轴、游标测量、直方图、散点图、瀑布图
- **完整 OTA 升级栈** -- XMODEM/YMODEM/ZMODEM 协议，自动 Intel HEX 转换
- **7 层严格分层架构** -- 单向依赖强制执行，接口驱动解耦

| 指标 | 数值 |
|:-----|:-----|
| 源文件数量 (`.cpp` + `.h`) | 760+ |
| 总代码行数 | 93,000+ |
| 功能模块 | 14 |
| 可复用公共组件 | 30+ |
| 设计模式应用 | 8 种 |
| 累计提交 | 310 |
| 质量评分 | 295 / 1000 |

---

## 核心特性

### 统一连接接口

| 连接类型 | 状态 | 说明 |
|:---------|:----:|:-----|
| Serial (UART) | 已完成 | 多端口，全参数配置（波特率/数据位/停止位/校验/流控） |
| TCP 客户端/服务端 | 已完成 | 远程设备 TCP 服务与本地监听 |
| UDP | 已完成 | 单播与广播通信 |
| TLS/SSL | 已完成 | 加密 TCP 连接与证书管理 |
| WebSocket | 已完成 | 全双工 WebSocket 客户端 |
| SEGGER RTT | 框架就绪 | J-Link SDK 动态加载（需 DLL 集成） |
| BLE | 框架就绪 | BLE 扫描器与 GATT 浏览器 |
| CAN/CAN-FD | 框架就绪 | CAN 总线监控与 DBC 解析器 |
| SPI/I2C | 框架就绪 | USB 适配器桥接模式 |
| MQTT | 框架就绪 | 发布/订阅客户端 |
| USB | 框架就绪 | libusb 设备检测 |

### 终端引擎

- **QPainter 自绘渲染** -- 绕过 QTextEdit DOM 开销，适配高吞吐场景
- **三种显示模式** -- 文本 / HEX / 混合模式，随时切换
- **毫秒级时间戳** -- 所有收发事件精确标注
- **快捷指令栏** -- 可配置按钮栏，一键发送命令
- **定时发送器** -- 可配置间隔与队列的循环发送
- **发送历史** -- 最近 50 条命令，支持频率分析
- **高级搜索** -- 关键词 / HEX / 正则 / 大小写敏感 / 全词匹配
- **分屏视图** -- 水平/垂直分屏，多标签页，双视图模式
- **数据对比** -- Myers diff 算法，并排对比，颜色高亮
- **智能补全** -- 前缀匹配，频率排序推荐

### 协议分析

- **可视化帧编辑器** -- 拖拽式字段配置（帧头/长度/数据/CRC/帧尾）
- **状态机解析器** -- 实时帧检测与字段提取
- **5 种内置模板** -- Modbus RTU、COBS、SLIP、JustFloat、FireWater
- **自定义协议引擎** -- JSON Schema 定义，CRC 校验
- **协议桥自动识别** -- 数据流自动协议类型判断
- **Protobuf 解码器** -- 嵌套消息解码支持
- **错误检测** -- CRC 校验失败与帧长度异常高亮

### 波形引擎

| 能力 | 代码行数 | 说明 |
|:-----|--------:|:-----|
| FFT 频谱分析 | 842 | 实时频域分析，支持多种窗函数 |
| 多 Y 轴 | 391 | 独立轴标度，按通道分组 |
| 游标测量 | 535 | 可拖拽测量游标，带差值显示 |
| 直方图 + 散点图 | 903 | 分布与相关性可视化 |
| 缩放控制器 | 478 | 交互式缩放、平移与区域选择 |
| 瀑布图 | -- | 时频瀑布频谱图 |

### OTA 固件升级

| 协议 | 块大小 | 特性 |
|:-----|:------:|:-----|
| XMODEM-Checksum | 128 B | 算术校验和 |
| XMODEM-CRC | 128 B | CRC16 校验 |
| XMODEM-1K | 1024 B | CRC16，大块传输 |
| YMODEM | 可变 | Block 0 含文件名/大小，批量传输 |
| ZMODEM | 可变 | CRC32，连续发送，断点续传 |

支持自动 Intel HEX 到 BIN 转换、实时进度追踪（百分比/吞吐量/剩余时间）、OTA 历史记录（时间戳/成功/失败状态）、CRC32 固件完整性校验。

### 仪表盘与自动化

- **仪表盘组件** -- 模拟/数字仪表盘、进度条、LED 指示器、数值显示器、热力图
- **数据触发引擎** -- 基于规则的自动化，支持正则匹配、阈值检测和动作分发
- **脚本录制/回放** -- 命令序列录制、回放、JSON 保存

### 数据管理

- **会话持久化** -- `.edproj` 工程文件，完整工作区保存/恢复
- **录制/回放** -- `.edl` 日志格式，统一时间轴，变速回放，书签标注
- **多格式导出** -- CSV、XLSX、PNG，支持时间范围选择
- **数据转换器** -- HEX/ASCII/Base64 编解码，校验和工具
- **时间戳工具** -- Unix 时间戳转换，多种格式支持

---

## 系统架构

### 七层分层架构

EmbedDebug 采用严格的 7 层依赖体系。依赖只能向下流动，反向依赖、同层横向引用、跨层跳级均被禁止。

```
+=====================================================================+
| L6  core/ -- 应用协调层                                             |
|     MainWindow, PanelManager, NavigationController, ThemeManager    |
|     SessionManager, ConnectionController, SendController            |
+=====================================================================+
       |  (可依赖所有下层; 永不被下层依赖)
       v
+=====================================================================+
| L5  ota/ automation/ dashboard/ plugin/ -- 业务层                   |
|     OtaManager, TriggerEngine, DashboardWidgets, PluginLoader       |
+=====================================================================+
       |
       v
+=====================================================================+
| L4  terminal/ chart/ rtt/ -- 表现层                                 |
|     TerminalWidget, ChartWidget, FftEngine, WaterfallWidget         |
+=====================================================================+
       |
       v
+=====================================================================+
| L3  connection/ protocol/ serial/ -- 外设/协议层                    |
|     SerialConnection, ModbusSlave, FrameParser, ProtocolEngine      |
+=====================================================================+
       |
       v
+=====================================================================+
| L2  utils/ -- 基础工具层                                            |
|     CRC, HexConverter, RingBuffer<T>, DataLogger, DataExporter      |
|     SettingsManager, ClipboardManager, ChecksumCalculator           |
+=====================================================================+
       |
       v
+=====================================================================+
| L1  shared/ -- 共享常量层                                           |
|     AppConstants, AnimationConstants, LayoutConstants               |
|     ConnectionConstants, TimerConstants                             |
+=====================================================================+
       |
       v
+=====================================================================+
| L0  interfaces/ -- 纯虚接口契约                                     |
|     IConnection, IPanelProvider, IDataSink, IProtocolParser,        |
|     IDevice                                                        |
+=====================================================================+
```

### 依赖矩阵

```
L6 core/             ->  L5 / L4 / L3 / L2 / L1 / L0
L5 ota/ automation/  ->  L4 / L3 / L2 / L1 / L0
L4 terminal/ chart/  ->  L3 / L2 / L1 / L0
L3 connection/ proto/ ->  L2 / L1 / L0
L2 utils/            ->  L1 / L0
L1 shared/           ->  仅 Qt 核心类型
L0 interfaces/       ->  (无依赖)
```

### 数据流

```
IConnection.open()
       |
       v
  dataReceived(QByteArray)
       |
       +---> TerminalModel  -----> TerminalWidget   (显示)
       |         |
       |         +---> FrameParser ---> ProtocolView (字段表)
       |                           |
       |                           +---> ChartModel ---> ChartWidget (波形)
       |
       +---> DataLogger    (录制到 .edl)
       +---> OtaManager    (固件升级)
       +---> TriggerEngine (自动化规则)
```

### 设计模式

| 模式 | 应用场景 |
|:-----|:---------|
| Strategy | OTA 协议切换（XMODEM/YMODEM/ZMODEM） |
| Observer | 数据流分发（Qt 信号/槽机制） |
| Factory | 连接创建（`ConnectionFactory`） |
| State | 连接状态管理、OTA 传输状态机 |
| Singleton | `SettingsManager`、`ThemeManager` |
| Template Method | OTA 传输流程（`BaseTransfer::execute()`） |
| Adapter | J-Link SDK 适配（`JLinkBridge -> IConnection`） |
| Command | 快捷指令、发送历史 |

---

## 模块索引

| 模块 | 路径 | 文件数 | 代码行数 | 层级 | 说明 |
|:-----|:-----|-------:|--------:|:-----|:-----|
| **core** | `src/core/` | 189 | 22,222 | L6 | 应用协调、主窗口、控制器、导航、主题、基础组件 |
| **connection** | `src/connection/` | 142 | 17,612 | L3 | 连接抽象、Serial/TCP/UDP/TLS/WebSocket/BLE/CAN/MQTT/USB/SPI/I2C |
| **protocol** | `src/protocol/` | 95 | 12,164 | L3 | 帧解析器、Modbus RTU 从站/主站、JustFloat/FireWater 桥、Protobuf |
| **utils** | `src/utils/` | 75 | 9,600 | L2 | CRC、HexConverter、RingBuffer、DataLogger、导出、校验、转换、时间戳 |
| **chart** | `src/chart/` | 55 | 7,179 | L4 | 波形显示、FFT、散点图、直方图、瀑布图、游标、多 Y 轴 |
| **terminal** | `src/terminal/` | 50 | 5,813 | L4 | QPainter 自绘终端、搜索、过滤、选区、上下文菜单、布局、标签页 |
| **serial** | `src/serial/` | 46 | 5,715 | L3 | 串口配置面板、快捷指令、信号线监控、数据统计 |
| **ota** | `src/ota/` | 36 | 4,797 | L5 | XMODEM/YMODEM/ZMODEM 协议、OTA 管理器、进度组件、历史模型 |
| **dashboard** | `src/dashboard/` | 25 | 3,012 | L5 | 仪表盘、进度条、LED、数值显示、热力图、布局序列化 |
| **plugin** | `src/plugin/` | 15 | 1,732 | L5 | 插件系统，DLL 动态加载与 API 桥接 |
| **automation** | `src/automation/` | 11 | 1,384 | L5 | 触发引擎、规则管理、动作分发 |
| **rtt** | `src/rtt/` | 12 | 1,315 | L4 | SEGGER RTT 连接，J-Link SDK 动态加载 |
| **shared** | `src/shared/` | 6 | 244 | L1 | 跨模块常量、枚举、轻量类型别名 |
| **interfaces** | `src/interfaces/` | 4 | 226 | L0 | 纯虚接口（IConnection, IPanelProvider, IDataSink, IProtocolParser, IDevice） |

---

## 快速上手

### 环境要求

| 依赖 | 最低版本 | 推荐版本 |
|:-----|:---------|:---------|
| Windows | 10 | 11 22H2+ |
| GCC (MinGW) | 13.0 | 14.2.0 |
| CMake | 3.20 | 4.0+ |
| Ninja | 1.11 | 1.13+ |
| Qt | 6.5 | 6.8.3 |
| GDB（可选） | 14.0 | 16.2 |

### 安装 Qt（通过 aqtinstall）

```bash
pip install aqtinstall
aqt install-qt windows desktop 6.8.3 win64_mingw \
    --outputdir E:\Tool\DevEnv\Qt \
    --modules qtserialport qtcharts qtnetworkauth
```

### 从源码构建

```bash
# 克隆仓库
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout feat/embed-debug

# 配置（请根据实际 Qt 安装路径调整）
cmake -G Ninja -B build \
    -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64

# 构建
cmake --build build

# 部署 Qt 运行时 DLL
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/windeployqt.exe build/EmbedDebug.exe
```

### 运行

双击项目根目录的 `EmbedDebug.bat`，或直接执行：

```bash
cd build && EmbedDebug.exe
```

### 验证

启动后请确认：
1. IDE 风格主窗口正常显示，左侧导航树 + 右侧内容区布局正确
2. 中央终端组件可见且可交互
3. 点击导航项时面板切换带有过渡动画

---

## 质量指标

### 评分追踪

EmbedDebug 采用量化质量评分体系。每次有效提交可获得积分，评分记录在 `docs/tracking/SCORE_TRACKING.md`。

| 指标 | 当前值 | 目标值 |
|:-----|:-------|:-------|
| 质量评分 | **295** | 1000 |
| 累计提交 | 310 | -- |
| 文件体积合规率 | 100% | 100% |
| Doxygen 覆盖率 | ~95% | 100% |
| `tr()` 国际化覆盖率 | ~90% | 100% |
| `objectName` 合规率 | ~90% | 100% |

### 文件体积约束（强制执行）

| 文件类型 | 上限 | 执行方式 |
|:---------|:-----|:---------|
| `.cpp` 实现文件 | 500 行 | Linter 强制 |
| `.h` 头文件 | 200 行 | Linter 强制 |
| 单个方法体 | 80 行 | 代码审查 |

### 代码质量标准

- C++17 标准，头文件引用顺序：Qt -> STL -> 项目内部，使用相对 `src` 路径
- 仅使用新式 `connect()` 语法（禁止 `SIGNAL`/`SLOT` 宏）
- 所有公开方法和成员变量必须有 Doxygen 文档注释
- QObject 父子树或智能指针管理内存（禁止裸 `new` 无配对 `delete`）
- 所有 `QWidget` 实例必须设置 `objectName`（QSS 依赖）
- 所有用户可见字符串必须用 `tr()` 包裹（国际化）

---

## 开发路线图

### 阶段一：基础建设（评分 1-100）-- 已完成

串口通信、HEX 显示、快捷指令、定时发送、搜索、导出、多端口、主题切换、协议帧解析器、实时波形、Intel HEX 解析、TCP/UDP、OTA 升级协议栈。

### 阶段二：现代化改造（评分 101-200）-- 已完成

UI 基础组件（BasePanel、IconManager、EmptyState、LoadingSpinner、Skeleton、CommandPalette、IconNavBar）、智能补全、脚本录制/回放、数据对比、FFT/多 Y 轴/游标/直方图/散点图、带书签的录制/回放、自定义协议引擎、仪表盘组件、数据触发器、会话管理、串口诊断、性能监控。

### 阶段三：功能扩展（评分 201-300）-- 进行中

BLE GATT 模型、MQTT TopicModel、CAN DBC 解析器、USB 设备检测、Dashboard 布局持久化、ProtocolEngine CRC 校验、键盘快捷键体系、Lucide 图标扩展、终端多规则搜索、OTA CRC32、串口芯片识别（21 家厂商）、协议桥自动识别、Protobuf 嵌套解码。

### 阶段四：打磨完善（评分 300-500）-- 计划中

- QSS 主题生成器（自动化 4,740 行手写 QSS）
- 响应式布局（断点系统，窗口 < 900px 自动折叠导航树）
- 统一弹窗体系（替换所有 `QMessageBox` 为自定义弹窗）
- BLE/CAN/MQTT/SPI-I2C 完整集成（填充剩余 stub 方法）
- J-Link SDK 真实 DLL 集成（实现 RTT 功能）
- 完整键盘快捷键体系

### 阶段五：正式发布（评分 500+）-- 计划中

- 国际化完善（中文 + 英文）
- 性能优化与 Profiling
- 安装包打包与代码签名
- 文档完善
- CI/CD 流水线搭建

---

## 贡献指南

### 贡献者前置条件

1. 阅读 `docs/constraints/` 目录下的约束文档（尤其是 `01-project-overview.md` 和 `03-architecture.md`）
2. 理解 7 层依赖模型及禁止依赖规则
3. 按照 [快速上手](#快速上手) 搭建构建环境

### 强制规则

| 规则 | 说明 |
|:-----|:-----|
| **PRD 必须先行** | 不允许无 PRD 的代码提交 |
| **架构审查** | 所有新增类必须通过架构检查清单 |
| **最低提交量** | 每次提交至少包含 300 行有效变更 |
| **零编译错误** | 提交前代码必须编译通过 |
| **启动验证** | 每次提交后 `EmbedDebug.bat` 必须能正常启动 |
| **文件体积限制** | `.cpp` <= 500 行，`.h` <= 200 行，方法 <= 80 行 |
| **MainWindow 禁写业务** | 业务逻辑委托给 Controller/Manager |

### Commit 消息格式

```
<模块名>: <简述改了什么>

<详细说明为什么这样改>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```

### 开发工作流

1. Fork 本仓库
2. 从 `feat/embed-debug` 创建功能分支（`git checkout -b feat/your-feature`）
3. 在 `docs/prd/` 中编写或更新 PRD
4. 按约束文档规范实现功能
5. 验证构建和启动
6. 使用标准消息格式提交
7. 推送并创建 Pull Request

### 代码审查清单

- [ ] 无反向或横向层级依赖
- [ ] 所有公开方法有 Doxygen 文档注释
- [ ] 所有用户可见字符串使用 `tr()`
- [ ] 所有 `QWidget` 实例已设置 `objectName`
- [ ] 无 C++ 硬编码颜色（使用 QSS 主题）
- [ ] 按钮在 QSS 中有 hover/pressed/disabled 三种状态
- [ ] 面板切换使用过渡动画（禁止突然出现/消失）

---

## 常见问题

**Q: 需要哪个版本的 Qt？**

A: 测试版本为 Qt 6.8.3。项目使用 CMake `find_package(Qt6)`，依赖 Core、Gui、Widgets、Svg、SerialPort、Charts 和 Network 模块。

**Q: 可以用 MSVC 替代 MinGW 吗？**

A: 项目使用 MinGW GCC 14.2 开发和测试。MSVC 理论上可以，但未经官方测试，可能需要调整 CMake 配置。

**Q: 如何新增连接类型？**

A: 继承 `src/interfaces/` 中的 `IConnection` 接口，实现接口方法，在 `ConnectionFactory` 中注册，创建配置面板，并在 `PanelManager` 中注册。完整步骤见 `docs/constraints/03-architecture.md` 第八节。

**Q: 如何新增面板？**

A: 按照 `docs/constraints/03-architecture.md` 第七节的 6 步面板注册清单操作：创建面板 -> 用 `BasePanel` 包装 -> 在 `PanelManager` 注册 -> 添加导航条目 -> 配置 QSS。

**Q: 质量评分体系是什么？**

A: 每次有效提交获得积分，记录在 `docs/tracking/SCORE_TRACKING.md`。当前评分 295，目标 1000。评分对应开发阶段和里程碑。

**Q: 为什么最低提交量是 300 行？**

A: 本项目追求原子性、有意义的提交。每次提交应代表一个完整的工作单元 -- 一个功能、一次重构或一批相关改进。

**Q: 如何创建自定义协议解析器？**

A: 创建 JSON Schema 文件描述帧格式（帧头、字段、CRC、帧尾），注册到 `ProtocolEngine`，配置 `ProtocolFieldMapper` 将字段映射到波形通道。内置模板包括 Modbus RTU、COBS、SLIP、JustFloat 和 FireWater。

**Q: SEGGER RTT 可以使用吗？**

A: RTT 框架（718 行）已实现，包含 J-Link SDK 动态加载基础设施，但 J-Link DLL 集成为 stub 状态。需要真实的 J-Link 硬件和 SDK 才能实现完整 RTT 功能。

---

## 开源协议

本项目基于 MIT 协议开源。详见 [LICENSE](LICENSE) 文件。

---

<div align="center">

*基于 Qt 6.8 | C++17 | CMake + Ninja 构建*

*由嵌入式工程师设计，为嵌入式工程师服务。*

</div>
