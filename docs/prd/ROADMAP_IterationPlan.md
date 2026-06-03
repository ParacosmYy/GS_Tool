# 迭代路线图 (EmbedDebug)

> 版本: v1.0 | 状态: 规划中
> 前置文档: `ROADMAP_FutureFeatures.md` (F1~F11), `ROADMAP_MassiveExpansion.md` (F12~F26)
> 参考文档: `docs/architecture/DECOUPLING_PROPOSAL.md`, `CLAUDE.md`

---

## 一、总览

EmbedDebug 迭代路线图共分五个里程碑, 涵盖 26 个特性, 预计新增约 27,100 行代码。M0 是架构层面的前置工作, 必须优先完成, 后续 M1~M4 按用户价值和依赖关系逐步推进。

| 里程碑 | 主题 | 特性范围 | 特性数 | 预估代码量 | 前置条件 |
|--------|------|----------|--------|-----------|---------|
| M0 | 架构解耦 | 4个解耦方向 | 4方向 | ~2,000行 | 无 |
| M1 | 核心差异化 | F1~F5 | 5 | ~4,100行 | M0 |
| M2 | 功能完成度 | F6~F11 | 6 | ~4,600行 | M1 |
| M3 | 专业级增强 | F12~F20 | 9 | ~14,900行 | M2 |
| M4 | 总线级扩展 | F21~F26 | 6 | ~3,100行 | M3 |
| **合计** | | **F1~F26** | **26** | **~28,700行** | |

> 注: 代码量包含 .h + .cpp, 实际新增约 179 个文件。

---

## 二、优先级排序原则

特性排序遵循以下五条原则, 优先级从高到低:

1. **架构解耦第一优先**。M0 必须先完成, 否则后续所有特性都会受限于现有耦合问题, 修改一处牵动全局。
2. **核心差异化功能优先**。用户价值最高的特性 (如 F1 录制回放、F2 协议引擎) 先做, 让工具在最短时间内甩开竞品。
3. **已有半成品的功能优先**。ScriptRecorder、CommandPalette、IconManager 等已有骨架或已实现但未充分集成的组件, 补齐即可见效。
4. **外部依赖少的功能优先**。不依赖第三方 DLL、SDK、硬件适配器的特性开发阻力更小, 适合早期推进。
5. **代码量小的功能优先**。同等价值下优先做代码量小的, 提高交付节奏感。

---

## 三、M0: 架构解耦

> 参考: `docs/architecture/DECOUPLING_PROPOSAL.md`
> 预估代码量: ~2,000行 | 预计周期: 1~2周

M0 不是功能特性, 而是架构层面的基础设施改造。当前代码库存在多处耦合问题, 直接影响后续所有特性的开发效率。

### 3.1 解耦方向

#### D1: MainWindow 瘦身

当前 `MainWindow.cpp` 承载了过多职责, 包括面板创建、信号连接、布局管理、事件分发等。解耦目标是让 MainWindow 只做"组装", 把业务逻辑下沉到各 Controller/Manager。

**涉及文件**: `MainWindow.h/cpp`, `PanelManagerCreation.cpp`

**验收标准**:
- [ ] `MainWindow.cpp` ≤ 500行
- [ ] `PanelManagerCreation.cpp` 的 include 数量从 43 个降到 15 个以内
- [ ] 所有业务逻辑委托给 Controller/Manager, MainWindow 只负责创建和布局
- [ ] 编译通过, `EmbedDebug.bat` 正常启动

#### D2: 循环依赖消除

当前存在 8 处循环依赖 (双向 include 或间接循环), 主要集中在 `core/` 和 `ui/` 之间。解耦手段包括引入接口类、前向声明、信号/槽替代直接调用。

**涉及文件**: 需逐一审计 `src/core/` 和 `src/ui/` 的 include 关系

**验收标准**:
- [ ] 所有循环依赖消除, 编译无警告
- [ ] 依赖方向严格遵循表现层→业务层→数据层→基础设施层
- [ ] 使用 `include-what-you-use` 工具验证无多余 include

#### D3: SendController 拆分

`SendController.h` 当前有 199/200 行, 几乎顶满 200 行上限, 职责包含发送逻辑、历史管理、快捷指令、定时发送。需拆分为 `SendController`(核心发送) + `SendHistoryManager`(历史) + `TimedSender`(定时发送, 已部分存在)。

**涉及文件**: `SendController.h/cpp`, 新增 `SendHistoryManager.h/cpp`

**验收标准**:
- [ ] `SendController.h` ≤ 150行
- [ ] 新增的 `SendHistoryManager` ≤ 200行
- [ ] 发送功能不受影响, 所有现有测试通过

#### D4: IconManager 集成

`IconManager` 已实现 (PRD-062, 138行), SVG 着色管线和缓存机制完备, 但当前图标数量仅 14 个, 且多处 UI 代码仍使用硬编码图标路径而非 `IconManager::icon()`。

**涉及文件**: `IconManager.h/cpp`, 所有使用图标的 UI 文件

**验收标准**:
- [ ] SVG 图标数量扩展到 50+ (Lucide 图标集)
- [ ] 所有 UI 代码通过 `IconManager::icon()` 获取图标, 无硬编码路径
- [ ] 主题切换时图标颜色自动跟随

---

## 四、M1: 核心差异化 (F1~F5)

> 预估代码量: ~4,100行 | 预计周期: 2~3周 | 前置: M0 完成

M1 聚焦核心差异化功能, 让 EmbedDebug 在核心调试场景上明显优于 VOFA+ 等竞品。

### F1: 数据录制回放增强

| 项 | 值 |
|----|-----|
| 优先级 | P0 |
| 模块 | `core/recording/`, `utils/log/` |
| 预估代码量 | ~800行 |
| M0 依赖 | D1 (MainWindow 瘦身后才能干净地嵌入 PlaybackWidget) |
| 涉及模块 | RecordingController, WaveformEngine, TerminalModel |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F1 |

**简述**: 当前 `RecordingController` 只有基础录制(开始/停止/保存)和简单回放。需要增加统一时间轴(所有数据流共用毫秒时间戳)、变速回放(0.25x~8x)、录制标注(快捷键标记关键时刻)以及回放时间轴导航(进度条拖拽 + 事件跳转)。录制文件格式升级为二进制格式, 向后兼容旧 CSV。

**新增类**: `RecordingTimeline`, `PlaybackController`, `RecordingFileFormat`, `RecordingMarker`, `PlaybackWidget`

---

### F2: 自定义协议脚本引擎

| 项 | 值 |
|----|-----|
| 优先级 | P0 |
| 模块 | `protocol/` |
| 预估代码量 | ~1,200行 |
| M0 依赖 | D2 (循环依赖消除后 ProtocolEngine 才能独立演进) |
| 涉及模块 | FrameParser, JustFloatBridge, FireWaterBridge, ChartModel |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F2 |

**简述**: 当前 `FrameParser` 只支持固定帧格式。需要让用户通过 JSON 文件定义自己的协议规则(帧头/长度字段/校验和/CRC), 内置 Modbus RTU、COBS、SLIP 等模板。协议解析后的字段可映射到波形通道, 实现实时显示。现有的 `JustFloatBridge`/`FireWaterBridge` 重构为 `ProtocolSchema` 的模板实例。

**新增类**: `ProtocolSchema`, `ProtocolEngine`, `ProtocolTemplateLibrary`, `ProtocolSchemaEditor`, `ProtocolFieldMapper`

---

### F3: 多通道数据导出增强

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `utils/export/`, `chart/model/` |
| 预估代码量 | ~500行 |
| M0 依赖 | 无直接依赖, 但受 F1 录制文件格式影响 |
| 涉及模块 | DataExporter, ChartModel, WaveformEngine |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F3 |

**简述**: 当前 `DataExporter` 只能导出终端数据。需要支持波形通道数据导出为 CSV(时间列+每通道一列)、Excel(.xlsx, 带格式化)、截图(PNG/SVG, 含图例), 支持时间范围选择和通道选择。`ExportDialog` 作为通用对话框, 终端导出和波形导出共用。

**新增类**: `ChartExporter`, `ExportDialog`

---

### F4: 终端增强 — 分屏 + 标签页

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `terminal/`, `core/panels/` |
| 预估代码量 | ~600行 |
| M0 依赖 | D1 (MainWindow 布局解耦) |
| 涉及模块 | TerminalWidget, TerminalModel |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F4 |

**简述**: 嵌入式调试中经常需要同时观察多个数据源。支持水平/垂直分屏(两个终端视图并排或上下)、多标签页(每个 Tab 可连接不同串口)、HEX 并排视图(同一数据同时显示文本和 HEX, 滚动同步)。参考 MobaXterm 的终端体验。分屏/标签页状态可保存到会话。

**新增类**: `TerminalSplitter`, `TerminalTabManager`, `TerminalDualView`

---

### F5: 仪表盘模式 (Gauge Dashboard)

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | 新增 `dashboard/` |
| 预估代码量 | ~1,000行 |
| M0 依赖 | D3 (SendController 拆分, 避免仪表盘组件与发送逻辑纠缠) |
| 涉及模块 | ChartModel, ThemeManager |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F5 |

**简述**: 参考 Serial Studio 的仪表盘, 提供实时数值监控能力。支持仪表盘(Gauge)、进度条、LED 指示灯、大字号数值显示、迷你折线图等组件类型, 自由拖拽排列在网格布局中。每个组件绑定波形通道, 数值实时更新。支持阈值告警(超限变色+通知)和全屏模式。

**新增类**: `DashboardWidget`, `GaugeWidget`, `ProgressBarWidget`, `LedIndicatorWidget`, `NumericDisplayWidget`, `DashboardModel`

---

## 五、M2: 功能完成度 (F6~F11)

> 预估代码量: ~4,600行 | 预计周期: 3~4周 | 前置: M1 完成

M2 补齐功能空白, 让 EmbedDebug 达到专业级串口调试工具的完整度。

### F6: J-Link RTT 深度集成

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `connection/`, 新增 `rtt/` |
| 预估代码量 | ~800行 |
| M0 依赖 | D1 (连接层需独立于 MainWindow) |
| 涉及模块 | IConnection, TerminalModel, ChartModel |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F6 |

**简述**: RTT 是嵌入式调试的核心能力, 提供零干扰调试输出(不占 UART, 不影响时序)。需要实现 `IConnection` 接口的 RTT 连接, J-Link SDK 通过 `QLibrary` 动态加载(缺失时优雅降级), RTT 多通道数据映射(Channel 0→终端, Channel 1+→波形通道)。包含 RTT 配置面板(设备/接口/速度/通道选择)和连接状态监控。

**新增类**: `JLinkRttConnection`, `JLinkSdkLoader`, `RttConfigPanel`, `RttChannelManager`

---

### F7: 数据流触发器与自动化

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | 新增 `automation/` |
| 预估代码量 | ~700行 |
| M0 依赖 | D2 (触发引擎需独立于 UI 层) |
| 涉及模块 | TerminalModel, ChartModel, RecordingController |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F7 |

**简述**: 参考 Docklight 的触发器功能。用户可定义触发规则: 条件(精确字符串/正则/HEX 字节序列/数值范围) + 动作(发送数据/开始录制/停止录制/Toast 通知/播放声音)。支持触发器列表管理(添加/编辑/删除/启用/禁用), 规则保存为 JSON 与会话一起持久化。触发引擎监听数据流信号, 异步执行动作。

**新增类**: `TriggerRule`, `TriggerEngine`, `TriggerAction`, `TriggerManager`, `TriggerListPanel`

---

### F8: 工程会话管理

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | `core/settings/` |
| 预估代码量 | ~500行 |
| M0 依赖 | D1 + D2 (需所有 Manager 解耦后才能统一收集配置) |
| 涉及模块 | SessionManager, 所有 Controller/Manager |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F8 |

**简述**: 引入"工程"概念, 一键切换多个嵌入式项目的完整配置。工程文件 `.edproj` 是 JSON 格式, 包含连接配置、协议定义、波形通道、快捷指令、触发器、窗口布局等所有状态。支持工程切换(下拉菜单)、最近工程列表(启动欢迎页)、工程模板(STM32/ESP32/RK3588 等预设)。

**新增类**: `Project`, `ProjectManager`, `ProjectWelcomeDialog`

---

### F9: 串口高级调试

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | `serial/` |
| 预估代码量 | ~600行 |
| M0 依赖 | 无直接依赖 |
| 涉及模块 | SerialConnection, IConnection |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F9 |

**简述**: 提供专业级串口分析能力。包括信号线状态实时显示(RTS/CTS/DTR/DSR/DCD/RI 六条线的指示灯)、信号线手动控制(RTS/DTR 开关)、发送时序控制(字节间延迟/帧间延迟)、流量监控(实时收发速率曲线)和串口错误统计面板(Framing/Parity/Overrun 错误计数)。

**新增类**: `SignalLineMonitor`, `SignalLineWidget`, `TrafficMonitor`, `TrafficMonitorWidget`

---

### F10: 性能监控与分析

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | `utils/` |
| 预估代码量 | ~400行 |
| M0 依赖 | 无直接依赖 |
| 涉及模块 | 所有数据管道组件 |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F10 |

**简述**: EmbedDebug 自身在高频数据下的性能监控。提供性能统计面板(CPU/内存/FPS/延迟)、数据管道端到端延迟测量(接收到渲染)、吞吐量统计(接收/解析/渲染各阶段速率)和可选的性能日志记录。`PerformanceOverlay` 类似游戏 FPS 计数器, 显示在窗口角落, 可通过设置开关。

**新增类**: `PerformanceMonitor`, `PerformanceOverlay`

---

### F11: 插件/扩展系统

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | 新增 `plugin/` |
| 预估代码量 | ~1,500行 |
| M0 依赖 | D1 + D2 (插件 API 需要稳定的内部接口) |
| 涉及模块 | 所有模块 (插件 API 横跨全部) |
| PRD 参考 | `ROADMAP_FutureFeatures.md` §F11 |

**简述**: 让高级用户和第三方扩展 EmbedDebug 的能力。定义 `IEmbedDebugPlugin` 接口(初始化/数据接入/UI 扩展), 插件加载器扫描 `plugins/` 目录动态加载 DLL。`PluginApi` 暴露有限接口(注册面板/添加通道/发送数据), 损坏的插件不影响主应用启动。远期可考虑 Python 脚本插件支持和在线插件市场。

**新增类**: `IEmbedDebugPlugin`, `PluginManager`, `PluginApi`, `PluginConfigPanel`

---

## 六、M3: 专业级增强 (F12~F20)

> 预估代码量: ~14,900行 | 预计周期: 14~20周 | 前置: M2 完成

M3 是大规模扩展阶段, 将 EmbedDebug 从串口调试工具升级为全栈嵌入式调试助手。分为四个子阶段。

### 6.1 M3-MS1: 总线级扩展

| 顺序 | 特性 | 代码量 | 说明 |
|------|------|--------|------|
| 1 | F13 CAN/CAN-FD 总线 | ~2,400行 | 汽车电子/工业控制核心, DBC 解码是杀手级功能 |
| 2 | F12 Bluetooth/BLE 调试 | ~2,200行 | IoT/可穿戴核心, PC 端缺好用的集成工具 |

#### F12: Bluetooth/BLE 调试

| 项 | 值 |
|----|-----|
| 优先级 | P0 |
| 模块 | `connection/ble/` |
| 预估代码量 | ~2,200行 |
| M0 依赖 | D1 (IConnection 独立) |
| 外部依赖 | WinRT BLE API (Windows 10+) |
| 涉及模块 | IConnection, TerminalModel, ChartModel, ProtocolEngine |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F12 |

**简述**: BLE 已成为嵌入式设备最常见的无线通信方式。实现 BLE 设备扫描(名称/RSSI/广播数据)、GATT 服务/特征浏览器、特征值读写(HEX/文本模式)、BLE 通知订阅(数据流入终端/波形管道)。Windows 平台使用 WinRT API, 未来可扩展 Linux BlueZ 后端。还支持经典蓝牙 AT 命令控制台(HC-05/HC-06)和 BLE 广播嗅探器。

**新增类**: `BleConnection`, `BleScanner`, `GattServiceModel`, `GattBrowserWidget`, `BleConfigPanel`, `BleAdvSniffer`, `AtCommandConsole`, `BleNativeBackend`, `BleAdvPacketParser`, `HciMonitor`

---

#### F13: CAN/CAN-FD 总线

| 项 | 值 |
|----|-----|
| 优先级 | P0 |
| 模块 | `connection/can/` |
| 预估代码量 | ~2,400行 |
| M0 依赖 | D1 (IConnection 独立) |
| 外部依赖 | PCANBasic.dll / SLCAN 适配器 |
| 涉及模块 | IConnection, ChartModel, ProtocolEngine |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F13 |

**简述**: CAN 总线是汽车电子和工业控制最主流的通信总线。支持 PCAN-USB 和 SLCAN(CANable)适配器, CAN 帧监视器(实时显示 ID/DLC/Data/时间戳), DBC 文件导入与信号提取(物理值含因子/偏移/单位), 信号自动绑定到波形通道。CAN 数据可流入 `ProtocolEngine` 做上层协议解析(UDS/ISO-TP)。

**新增类**: `CanConnection`, `CanAdapter`, `PcanAdapter`, `SlcanAdapter`, `CanFrameParser`, `DbcParser`, `DbcDatabase`, `CanSignalExtractor`, `CanMonitorWidget`, `CanSendWidget`, `CanConfigPanel`, `DbcBrowserWidget`, `CanStatsWidget`

---

### 6.2 M3-MS2: 通信层全覆盖

| 顺序 | 特性 | 代码量 | 说明 |
|------|------|--------|------|
| 3 | F15 TCP/UDP 增强 | ~1,000行 | 多连接/Server/TLS 等基础能力 |
| 4 | F17 WebSocket 客户端 | ~800行 | 实现成本低, IoT 常见 |
| 5 | F14 MQTT 客户端 | ~1,600行 | IoT 标准协议, 配合 WS 传输 |
| 6 | F16 SPI/I2C 桥接 | ~1,400行 | 芯片级调试, 差异化功能 |

#### F14: MQTT 客户端

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `connection/mqtt/` |
| 预估代码量 | ~1,600行 |
| M0 依赖 | D1 |
| 外部依赖 | QtMqtt / Mosquitto |
| 涉及模块 | IConnection, TerminalModel, ProtocolEngine |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F14 |

**简述**: MQTT 是 IoT 领域的事实标准协议。支持 MQTT 5.0 连接(TCP/WebSocket, TLS, 认证), 主题订阅/发布(QoS 0/1/2, 通配符), 主题树浏览器, 消息流视图。MQTT payload 数据可通过协议引擎解析并接入波形通道, 在同一工具里验证设备的发布/订阅行为。

**新增类**: `MqttConnection`, `MqttClientBackend`, `MqttTopicModel`, `MqttMessageQueue`, `MqttConfigPanel`, `MqttTopicTreeWidget`, `MqttMessageView`, `MqttPublishWidget`

---

#### F15: TCP/UDP 增强

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `connection/tcp/` |
| 预估代码量 | ~1,000行 |
| M0 依赖 | D1 |
| 外部依赖 | Qt SSL 模块 |
| 涉及模块 | IConnection, TcpConnection, UdpConnection |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F15 |

**简述**: 将当前单连接 TCP/UDP 客户端升级为多连接管理。支持同时维护多个独立 TCP/UDP 连接(标签页切换)、TCP Server 监听模式(设备主动连接 EmbedDebug)、UDP 组播/广播、SSL/TLS 加密连接(证书验证, 自签名证书)。提供连接管理面板, 统一展示所有活跃连接的状态。

**新增类**: `MultiConnectionManager`, `TcpServerConnection`, `UdpMulticastConnection`, `TlsConnection`, `ConnectionManagerWidget`

---

#### F16: SPI/I2C 桥接

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `connection/spi_i2c/` |
| 预估代码量 | ~1,400行 |
| M0 依赖 | D1 |
| 外部依赖 | ftd2xx.dll / CH347DLL.DLL |
| 涉及模块 | IConnection, ProtocolEngine |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F16 |

**简述**: SPI 和 I2C 是嵌入式系统最常用的芯片级总线。通过 FTDI FT232H/FT2232 或 CH347 适配器, 让 EmbedDebug 独立于 MCU 直接与传感器、存储器、显示屏通信。支持 SPI 全双工读写(CPOL/CPHA 配置)、I2C 读写(7/10 位地址)、I2C 设备自动扫描(0x00~0x7F)、寄存器读写器(8/16/32 位地址)。

**新增类**: `SpiConnection`, `I2cConnection`, `FtdiBridge`, `Ch347Bridge`, `I2cScanner`, `RegisterEditor`, `SpiI2cConfigPanel`, `I2cScanWidget`

---

#### F17: WebSocket 客户端

| 项 | 值 |
|----|-----|
| 优先级 | P1 |
| 模块 | `connection/ws/` |
| 预估代码量 | ~800行 |
| M0 依赖 | D1 |
| 外部依赖 | Qt WebSocket 模块 |
| 涉及模块 | IConnection, TerminalModel |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F17 |

**简述**: WebSocket 正在成为嵌入式设备的远程管理接口。基于 Qt 自带 `QWebSocket`, 实现成本很低。支持 ws:// 和 wss:// 连接, 文本帧和二进制帧自动检测, 消息历史按时间序列显示, Ping/Pong 心跳检测和延迟测量。数据直接接入终端和协议解析管道。

**新增类**: `WebSocketConnection`, `WsConfigPanel`, `WsMessageLog`

---

### 6.3 M3-MS3: 协议深度支持

| 顺序 | 特性 | 代码量 | 说明 |
|------|------|--------|------|
| 7 | F18 Modbus 主站/从站 | ~1,800行 | 工业自动化标准协议 |
| 8 | F19 Protobuf/FlatBuffers | ~1,200行 | 现代嵌入式数据序列化 |
| 9 | F20 USB 调试 | ~1,500行 | USB 固件开发利器 |

#### F18: Modbus 主站/从站

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | `protocol/modbus/` |
| 预估代码量 | ~1,800行 |
| M0 依赖 | D1, D2 |
| 依赖特性 | F2 (协议引擎) |
| 涉及模块 | ProtocolEngine, IConnection (Serial/TCP) |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F18 |

**简述**: Modbus 是工业自动化领域元老级协议, 至今仍是 PLC、变频器、仪表最常用的通信协议。支持 Modbus RTU(串口, CRC16)、Modbus TCP(以太网, 端口 502)、主站操作(读写寄存器/线圈, 功能码 FC01~FC16)、设备扫描(1~247 地址)、寄存器监视器(周期轮询, 表格/波形显示)和从站模拟。异常码正确解析和显示。

**新增类**: `ModbusMaster`, `ModbusSlave`, `ModbusFrameCodec`, `ModbusDataModel`, `ModbusScanner`, `ModbusRegisterMonitor`, `ModbusMasterPanel`, `ModbusRegisterView`, `ModbusConfigPanel`

---

#### F19: Protobuf/FlatBuffers 解码

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | `protocol/protobuf/` |
| 预估代码量 | ~1,200行 |
| M0 依赖 | D2 |
| 依赖特性 | F2 (协议引擎) |
| 涉及模块 | ProtocolEngine, ProtocolFieldMapper, ChartModel |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F19 |

**简述**: 现代嵌入式设备越来越多使用 Protocol Buffers 和 FlatBuffers 序列化数据。支持导入 `.proto` 和 `.fbs` schema 文件, 二进制 payload 自动解码为结构化数据。手写 varint/zigzag/length-delimited 解析器, 不依赖 protoc 或 libprotobuf。解码后的数值字段可映射到波形通道, 嵌套 message 可展开/折叠显示。

**新增类**: `ProtobufSchema`, `ProtobufDecoder`, `FlatBuffersSchema`, `FlatBuffersDecoder`, `SchemaFieldMapper`, `ProtobufSchemaWidget`, `DecodedMessageView`

---

#### F20: USB 调试

| 项 | 值 |
|----|-----|
| 优先级 | P2 |
| 模块 | `connection/usb/` |
| 预估代码量 | ~1,500行 |
| M0 依赖 | D1 |
| 外部依赖 | libusb / WinUSB |
| 涉及模块 | IConnection, TerminalModel |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F20 |

**简述**: USB 是嵌入式设备与 PC 连接最通用的接口。通过 libusb 直接与 USB 设备通信(绕过内核驱动)。支持设备枚举(VID/PID/厂商/产品名)、描述符查看器(Device/Configuration/Interface/Endpoint)、端点监控(实时 Bulk/Interrupt IN/OUT 传输数据)、批量/中断传输读写。对开发自定义 USB 设备(HID/自定义类)的固件工程师非常有价值。

**新增类**: `UsbConnection`, `UsbDeviceList`, `UsbDescriptorParser`, `UsbTransferEngine`, `UsbDeviceTreeWidget`, `UsbDescriptorView`, `UsbConfigPanel`

---

## 七、M4: 效率工具集 (F21~F26)

> 预估代码量: ~3,100行 | 预计周期: 3~4周 | 前置: M3 完成

M4 是日常调试效率工具集, 特性之间相互独立, 大部分无外部依赖, 可并行开发。

### F21: 正则终端过滤器

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | `terminal/filter/` |
| 预估代码量 | ~600行 |
| M0 依赖 | 无 |
| 涉及模块 | TerminalModel, TerminalWidget |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F21 |

**简述**: 嵌入式调试中终端输出常被大量日志淹没。提供实时正则过滤(只显示匹配行或排除匹配行)、多规则颜色高亮、捕获组提取(匹配结果以表格展示)。过滤规则可保存到会话, 支持匹配行数/总行数/匹配率统计。过滤逻辑在数据层完成, 不阻塞 UI。

**新增类**: `TerminalFilter`, `FilterRule`, `CaptureGroupExtractor`, `FilterRuleWidget`, `FilterResultWidget`

---

### F22: 校验和计算器

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | `utils/checksum/` |
| 预估代码量 | ~500行 |
| M0 依赖 | 无 |
| 涉及模块 | CrcCalculator (已有公共组件) |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F22 |

**简述**: 校验和计算是嵌入式开发高频操作。复用已有 `CrcCalculator`, 扩展 XOR/Sum 算法。支持 CRC8/CRC16/CRC32/XOR8/Sum8/Sum16/Sum32, 内置 CRC 标准预设(CCITT/Modbus/MPEG2), 支持自定义多项式/初始值/异或输出。输入支持 HEX 和文本模式, 结果一键复制。作为停靠面板随时可用。

**新增类**: `ChecksumCalculator`, `ChecksumWidget`, `CrcPresetLibrary`

---

### F23: ASCII/表格转换器

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | `utils/converter/` |
| 预估代码量 | ~400行 |
| M0 依赖 | 无 |
| 涉及模块 | HexConverter (已有) |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F23 |

**简述**: 嵌入式开发者日常编码转换工具箱。支持 Hex↔ASCII 互转、Base64 编码/解码、URL 编码/解码(RFC 3986)、字节序翻转(大端↔小端, 16/32/64 位)、数值进制转换(HEX/DEC/OCT/BIN)。双栏 UI 实时转换, 输入任意一侧另一侧自动更新。

**新增类**: `DataConverter`, `ConverterWidget`

---

### F24: 时间戳分析器

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | `utils/timestamp/` |
| 预估代码量 | ~300行 |
| M0 依赖 | 无 |
| 涉及模块 | 无 |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F24 |

**简述**: 嵌入式设备常输出 Unix 时间戳, 调试时需要快速转换。支持秒级/毫秒级/微秒级时间戳与可读日期时间互转, 自动检测精度。提供当前时间戳实时刷新、时间差计算、ISO 8601/自定义格式输出和批量转换。

**新增类**: `TimestampConverter`, `TimestampWidget`

---

### F25: 数据包构建器

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | `utils/packet/` |
| 预估代码量 | ~700行 |
| M0 依赖 | 无 |
| 依赖特性 | F22 (校验和计算) |
| 涉及模块 | SendController, ChecksumCalculator |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F25 |

**简述**: 可视化数据包构建器, 省去手动计算偏移量和校验和的麻烦。通过表单界面定义每个字段(名称/类型/字节序/值), 自动计算校验和并插入, 实时预览 HEX+ASCII 结果。支持长度字段自动计算、数据包模板保存/加载、一键发送到当前连接。

**新增类**: `PacketField`, `PacketBuilder`, `PacketTemplate`, `PacketBuilderWidget`, `PacketFieldEditor`

---

### F26: 设备注册表

| 项 | 值 |
|----|-----|
| 优先级 | P3 |
| 模块 | `core/device/` |
| 预估代码量 | ~600行 |
| M0 依赖 | D1 (需所有 Manager 独立后才能聚合配置) |
| 涉及模块 | 所有 Controller/Manager |
| PRD 参考 | `ROADMAP_MassiveExpansion.md` §F26 |

**简述**: 开发者在多个嵌入式设备间切换时, 需要保存每个设备的完整调试配置。设备档案包含设备名称/型号/连接配置/协议配置/快捷命令列表。支持按标签/名称筛选, 一键连接(自动应用所有配置并建立连接), 快捷命令库(按设备维护常用命令), 配置导入/导出 JSON 文件可在团队间分享。

**新增类**: `DeviceProfile`, `DeviceRegistry`, `QuickCommandLibrary`, `DeviceListWidget`, `DeviceProfileEditor`

---

## 八、依赖关系图

```
M0: 架构解耦
├── D1: MainWindow 瘦身
├── D2: 循环依赖消除
├── D3: SendController 拆分
└── D4: IconManager 集成
        │
        ▼
M1: 核心差异化
├── F1: 数据录制回放增强  ←── D1
├── F2: 自定义协议脚本引擎  ←── D2
├── F3: 多通道数据导出  ←── F1 (录制文件格式)
├── F4: 终端分屏+标签页  ←── D1
└── F5: 仪表盘模式  ←── D3
        │
        ▼
M2: 功能完成度
├── F6: J-Link RTT  ←── D1
├── F7: 数据流触发器  ←── D2
├── F8: 工程会话管理  ←── D1 + D2
├── F9: 串口高级调试  (独立)
├── F10: 性能监控  (独立)
└── F11: 插件系统  ←── D1 + D2 + F2
        │
        ▼
M3: 专业级增强
│
├── M3-MS1: 总线级扩展
│   ├── F12: BLE 调试  ←── D1, F15(TLS)
│   └── F13: CAN/CAN-FD  ←── D1
│
├── M3-MS2: 通信层全覆盖
│   ├── F14: MQTT 客户端  ←── F17(WS)
│   ├── F15: TCP/UDP 增强  ←── D1
│   ├── F16: SPI/I2C 桥接  ←── D1
│   └── F17: WebSocket  ←── D1
│
└── M3-MS3: 协议深度支持
    ├── F18: Modbus  ←── F2(协议引擎)
    ├── F19: Protobuf/FlatBuffers  ←── F2
    └── F20: USB 调试  ←── D1
        │
        ▼
M4: 效率工具集
├── F21: 正则终端过滤器  (独立)
├── F22: 校验和计算器  (独立)
├── F23: ASCII/表格转换器  (独立)
├── F24: 时间戳分析器  (独立)
├── F25: 数据包构建器  ←── F22
└── F26: 设备注册表  ←── D1
```

### 依赖说明

| 依赖关系 | 原因 |
|---------|------|
| M1 → M0 | M0 的解耦是所有后续特性的基础, 必须先完成 |
| F3 → F1 | 导出增强依赖录制文件格式定义 |
| F4 → D1 | 分屏需要 MainWindow 布局解耦 |
| F8 → D1+D2 | 工程管理需收集所有 Manager 配置 |
| F11 → F2 | 协议引擎为插件系统提供扩展 API 基础 |
| F12 ↔ F15 | BLE Secure 可能需要 TLS 能力 |
| F14 → F17 | MQTT 可选 WebSocket 作为传输层 |
| F18 → F2 | Modbus 作为协议引擎的内置模板 |
| F19 → F2 | Protobuf 解码结果需通过协议字段映射 |
| F25 → F22 | 数据包构建器依赖校验和计算 |
| F4, F6, F7, F9, F10 | 相互独立, 可并行开发 |
| F21, F22, F23, F24 | 工具类特性完全独立, 可并行开发 |

---

## 九、技术债务清单

当前代码库存在以下技术债务, 需在 M0 阶段或迭代过程中逐步清理:

| # | 债务项 | 严重程度 | 影响范围 | 处理时机 |
|---|--------|---------|---------|---------|
| TD-01 | **IconManager 未被调用**: 已实现但仅 14 个 SVG, 多处 UI 硬编码图标路径 | 中 | 全局 UI | M0-D4 |
| TD-02 | **SendController.h 199/200 行**: 逼近文件体积上限, 职责过多 | 高 | 发送模块 | M0-D3 |
| TD-03 | **PanelManagerCreation.cpp 43 个 include**: 严重耦合, 修改牵动全局 | 高 | MainWindow | M0-D1 |
| TD-04 | **8 个循环依赖**: 主要在 `core/` 和 `ui/` 之间 | 高 | 架构 | M0-D2 |
| TD-05 | **07-directory-structure.md 过时**: 实际目录结构与文档不一致 | 低 | 开发规范 | M0 后更新 |
| TD-06 | **QMessageBox 仍被使用**: 违反 05-ui-standard 统一弹窗规范 | 中 | 全局 UI | M1 阶段 |
| TD-07 | **tr() 国际化不完整**: 新增模块部分字符串未包裹 tr() | 低 | 国际化 | 每次迭代检查 |
| TD-08 | **objectName 缺失**: 部分新增 QWidget 未设置 objectName | 中 | QSS 主题 | 每次迭代检查 |

---

## 十、风险评估

### 10.1 高风险项

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| **J-Link SDK 无法获取**: SEGGER 的 SDK 分发有限制 | 中 | F6 延期 | F6 不阻塞其他特性, 可推迟; 先用 stub 验证架构 |
| **WinRT BLE API 限制**: Windows 10+ 才支持, 旧系统不可用 | 高 | F12 受限 | 检测系统版本, 旧系统显示"不支持 BLE"提示; 未来加 Linux BlueZ 后端 |
| **PCAN 驱动需单独安装**: 用户可能无适配器 | 低 | F13 受限 | 优先支持 SLCAN(最便宜方案, 几十元 CANable), PCAN 为可选 |
| **libusb 许可证问题**: LGPL 2.1 静态链接需注意 | 中 | F20 许可合规 | 动态链接 libusb, 或评估 libusbk 替代方案 |

### 10.2 中风险项

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| **QtMqtt 模块不可用**: Qt 6.8.3 可能不含 QtMqtt | 中 | F14 备选方案 | 备选 Mosquitto 嵌入式客户端 (EPL 许可) |
| **FTDI D2XX 驱动限制**: 厂商 DLL 分发受控 | 低 | F16 受限 | 同时支持 CH347, 用户至少有一个选项 |
| **DBC 解析复杂度**: CAN DBC 规范细节多 | 中 | F13 开发周期 | 实现核心 DBC 3.0 子集, 复杂特性后续迭代 |
| **Protobuf schema 解析**: proto3 语法较复杂 | 低 | F19 开发周期 | 只解析 message/enum/scalar, 不支持 oneof/map |

### 10.3 低风险项

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| **文件体积铁律**: 新增模块可能超标 | 低 | 代码质量 | 严格代码审查, 每次提交检查文件行数 |
| **M0 解耦引入回归**: 重构可能破坏现有功能 | 低 | 稳定性 | 每次解耦后运行 `EmbedDebug.bat` 验证, 灰度发布 |
| **工具类特性重复造轮子**: Hex 转换等已有部分实现 | 低 | 冗余代码 | 先审计现有组件, 复用为主 |

---

## 十一、特性索引表 (F1~F26 快速参考)

| ID | 特性名称 | 优先级 | 里程碑 | 模块 | 预估代码量 | 依赖 | 竞品参考 |
|----|---------|--------|--------|------|-----------|------|---------|
| F1 | 数据录制回放增强 | P0 | M1 | `core/recording/` | ~800行 | D1 | VOFA+ |
| F2 | 自定义协议脚本引擎 | P0 | M1 | `protocol/` | ~1,200行 | D2 | Serial Studio |
| F3 | 多通道数据导出增强 | P1 | M1 | `utils/export/` | ~500行 | F1 | Docklight |
| F4 | 终端分屏+标签页 | P1 | M1 | `terminal/` | ~600行 | D1 | MobaXterm |
| F5 | 仪表盘模式 | P1 | M1 | `dashboard/` | ~1,000行 | D3 | Serial Studio |
| F6 | J-Link RTT 深度集成 | P1 | M2 | `rtt/` | ~800行 | D1 | J-Link RTT Viewer |
| F7 | 数据流触发器与自动化 | P2 | M2 | `automation/` | ~700行 | D2 | Docklight |
| F8 | 工程会话管理 | P2 | M2 | `core/settings/` | ~500行 | D1+D2 | VS Code |
| F9 | 串口高级调试 | P2 | M2 | `serial/` | ~600行 | 无 | Docklight |
| F10 | 性能监控与分析 | P2 | M2 | `utils/` | ~400行 | 无 | Serial Studio |
| F11 | 插件/扩展系统 | P3 | M2 | `plugin/` | ~1,500行 | D1+D2+F2 | VS Code |
| F12 | Bluetooth/BLE 调试 | P0 | M3 | `connection/ble/` | ~2,200行 | D1, F15 | nRF Connect |
| F13 | CAN/CAN-FD 总线 | P0 | M3 | `connection/can/` | ~2,400行 | D1 | CANoe |
| F14 | MQTT 客户端 | P1 | M3 | `connection/mqtt/` | ~1,600行 | F17 | MQTTX |
| F15 | TCP/UDP 增强 | P1 | M3 | `connection/tcp/` | ~1,000行 | D1 | Hercules |
| F16 | SPI/I2C 桥接 | P1 | M3 | `connection/spi_i2c/` | ~1,400行 | D1 | Aardvark |
| F17 | WebSocket 客户端 | P1 | M3 | `connection/ws/` | ~800行 | D1 | Postman |
| F18 | Modbus 主站/从站 | P2 | M3 | `protocol/modbus/` | ~1,800行 | F2 | ModScan |
| F19 | Protobuf/FlatBuffers | P2 | M3 | `protocol/protobuf/` | ~1,200行 | F2 | Protobuf Inspector |
| F20 | USB 调试 | P2 | M3 | `connection/usb/` | ~1,500行 | D1 | Wireshark USB |
| F21 | 正则终端过滤器 | P3 | M4 | `terminal/filter/` | ~600行 | 无 | MobaXterm |
| F22 | 校验和计算器 | P3 | M4 | `utils/checksum/` | ~500行 | 无 | CRC Calculator |
| F23 | ASCII/表格转换器 | P3 | M4 | `utils/converter/` | ~400行 | 无 | CyberChef |
| F24 | 时间戳分析器 | P3 | M4 | `utils/timestamp/` | ~300行 | 无 | Epoch Converter |
| F25 | 数据包构建器 | P3 | M4 | `utils/packet/` | ~700行 | F22 | Packet Sender |
| F26 | 设备注册表 | P3 | M4 | `core/device/` | ~600行 | D1 | VS Code Profile |

---

## 十二、里程碑时间线总览

```
2026年
  │
  ├─ W1~W2    M0: 架构解耦
  │              D1 MainWindow 瘦身 + D2 循环依赖 + D3 SendController + D4 IconManager
  │
  ├─ W3~W5    M1: 核心差异化
  │              F1 录制回放 → F2 协议引擎 → F3 导出 → F4 终端 → F5 仪表盘
  │
  ├─ W6~W9    M2: 功能完成度
  │              F6 RTT → F7 触发器 → F8 工程管理 → F9 串口高级 → F10 性能 → F11 插件
  │
  ├─ W10~W29  M3: 专业级增强
  │   ├─ W10~W15   M3-MS1: F12 BLE + F13 CAN
  │   ├─ W16~W23   M3-MS2: F15 TCP+ → F17 WS → F14 MQTT → F16 SPI/I2C
  │   └─ W24~W29   M3-MS3: F18 Modbus → F19 Protobuf → F20 USB
  │
  └─ W30~W33  M4: 效率工具集
                 F21 过滤器 + F22 校验和 + F23 转换器 + F24 时间戳 + F25 数据包 + F26 设备
```

---

## 十三、代码量汇总

### 按里程碑

| 里程碑 | 特性数 | 新增代码(行) | 新增文件数 | 预计周期 |
|--------|--------|-------------|-----------|---------|
| M0 架构解耦 | 4方向 | ~2,000 | ~8 | 1~2 周 |
| M1 核心差异化 | 5 | ~4,100 | ~21 | 2~3 周 |
| M2 功能完成度 | 6 | ~4,600 | ~22 | 3~4 周 |
| M3 专业级增强 | 9 | ~14,900 | ~96 | 14~20 周 |
| M4 效率工具集 | 6 | ~3,100 | ~22 | 3~4 周 |
| **合计** | **26** | **~28,700** | **~169** | **23~33 周** |

### 按层级分布

| 层级 | 新增代码(行) | 占比 | 说明 |
|------|-------------|------|------|
| 基础设施层 | ~9,000 | 31% | IConnection 实现 + 平台适配 + 传输引擎 |
| 数据层 | ~7,000 | 24% | 数据模型 + 解析器 + 编解码 |
| 业务层 | ~5,000 | 18% | 控制器 + 引擎 + 管理器 |
| 表现层 | ~7,700 | 27% | 配置面板 + 监视器 + 编辑器 + 组件 |

### 与现有代码库对比

| 范围 | 特性数 | 代码行 | 文件数 |
|------|--------|--------|--------|
| 已有代码库 (当前) | — | ~15,000 | ~120 |
| F1~F11 (M1+M2) | 11 | ~8,700 | ~43 |
| F12~F26 (M3+M4) | 15 | ~17,500 | ~118 |
| M0 架构解耦 | 4方向 | ~2,000 | ~8 |
| **路线图总计** | **26** | **~28,200** | **~169** |

---

> **注意**: 本路线图是活文档, 随开发推进和用户反馈持续更新。优先级可根据实际需求调整。
> 每个特性在正式开发前需输出独立的 `FEATURE_XXX.md` 详细设计文档。
> 所有新增代码遵守 `CLAUDE.md` 中的铁律: 四层架构、IConnection 接口、文件体积限制、中文注释。
