# EmbedDebug 模块解耦架构提案

> **项目**: EmbedDebug — Qt 6.8.3 嵌入式串口调试工具
> **分支**: `feat/embed-debug` (2626f47)
> **日期**: 2026-06-03
> **状态**: 提案阶段

---

## 目录

1. [现状分析](#1-现状分析)
2. [解耦方案](#2-解耦方案)
3. [模块依赖规则](#3-模块依赖规则)
4. [迁移计划](#4-迁移计划)
5. [预期收益](#5-预期收益)

---

## 1. 现状分析

### 1.1 项目规模

| 指标 | 数值 |
|------|------|
| 源文件总数 | ~358（.h 180 + .cpp 178） |
| 模块数量 | 12 个顶级模块 |
| 构建系统 | CMake + Ninja, MinGW GCC 14.2.0 |
| Qt 版本 | 6.8.3（Core/Gui/Widgets/Svg/SerialPort/Charts/Network） |
| C++ 标准 | C++17 |
| 单文件约束 | .h ≤ 200 行, .cpp ≤ 500 行 |

### 1.2 模块概览

| 模块 | 文件数 | 职责 |
|------|--------|------|
| core | 86 | 应用壳（MainWindow、PanelManager、Controllers、Theme） |
| connection | 66 | 连接抽象层（13 种连接类型） |
| utils | 39 | 基础工具（导出、日志、校验、转换） |
| protocol | 45 | 协议解析（帧解析器、Modbus、DBC、Protobuf） |
| terminal | 32 | 终端显示（Widget、Model、Search、Filter） |
| serial | 25 | 串口功能（配置、数据统计、命令栏） |
| chart | 21 | 图表（波形、FFT、散点、直方图） |
| ota | 19 | 固件升级（OTA Manager、烧录 Widget） |
| dashboard | 12 | 仪表盘（DashboardWidget） |
| rtt | 8 | SEGGER RTT 调试通道 |
| automation | 9 | 自动化（宏录制、触发规则） |
| plugin | 7 | 插件系统（PluginConfigPanel） |

### 1.3 跨模块依赖矩阵

以下矩阵统计每个模块中 `.h` 和 `.cpp` 文件对**其他模块**的 `#include` 引用数量：

```
依赖矩阵（跨模块 #include 计数）:
                auto  chart  conn  core  dash  ota  plug  proto  rtt  serial  term  utils
──────────────────────────────────────────────────────────────────────────────────────────
automation (9f)   0     0     0     0     0    0    0     0     0     0      0     0
chart     (21f)   0     -     0     7     0    0    0     2     0     0      0     0
connection(66f)   0     0     -     3     0    0    0     0     0     0      0     0
core      (86f)   2    11    42     -     2    9    2    16     3    29     18    24   ← 上帝模块
dashboard (12f)   0     0     0     0     -    0    0     0     0     0      0     0
ota       (19f)   0     0     2     6     0    -    0     1     0     0      0     8
plugin     (7f)   0     0     0     0     0    0    -     0     0     0      0     0
protocol  (45f)   0     0     1     4     0    0    0     -     0     0      0     4
rtt        (8f)   0     0     1     0     0    0    0     0     -     0      0     0
serial    (25f)   0     0     3     8     0    0    0     0     0     -      0     3
terminal  (32f)   0     0     0    10     0    0    0     0     0     0      -     3
utils     (39f)   0     0     0     5     0    0    0     0     0     0      1     -
```

**关键发现**:

- **core 模块向 11 个其他模块发出 158 条跨模块引用** — 这是全项目耦合的绝对中心
- **connection 模块收到 49 条入站引用**（fan-in 最高）— 所有模块都依赖它
- **完全解耦的叶子模块**: automation(0)、dashboard(0)、plugin(0) — **这是解耦的黄金标准**
- core → connection(42) 和 core → serial(29) 是最严重的单向依赖

### 1.4 上帝类分析

#### PanelManagerCreation.cpp & PanelManager.cpp

```
路径:  src/core/panels/PanelManagerCreation.cpp
       src/core/panels/PanelManager.cpp
跨模块 #include: 各 57 条（两文件完全相同的引用集）
涉及模块数: 10 个（几乎全部）
```

**具体分布**:

| 目标模块 | include 数量 | 典型头文件 |
|----------|-------------|-----------|
| connection | 11 | BleConfigPanel, CanConfigPanel, MqttConfigPanel, SpiI2cConfigPanel... |
| serial | 7 | SerialConfigPanel, DataStatistics, QuickCommandBar, BookmarkWidget... |
| protocol | 7 | ProtocolView, FrameVisualEditor, ProtocolSchemaEditor, ModbusConfigPanel... |
| terminal | 5 | TerminalWidget, TerminalModel, TerminalSearchBar, TerminalFilterBar... |
| utils | 4 | ChecksumPanel, ConverterPanel, TimestampPanel, PacketBuilderPanel... |
| chart | 4 | ChartWidget, FftWidget, ScatterWidget, HistogramWidget... |
| ota | 2 | OtaManager, OtaWidget |
| rtt | 1 | RttConfigPanel |
| automation | 1 | TriggerListPanel |
| plugin | 1 | PluginConfigPanel |

**问题**: 每次新增一个面板，需要同时修改 `PanelManagerPanels.h`（前向声明）、`PanelManagerCreation.cpp`（创建）、`PanelManager.cpp`（getter + 映射）三处。这是典型的"修改一个功能需要改 N 个文件"的紧耦合症状。

#### MainWindow.h

```
路径:  src/core/mainwindow/MainWindow.h
跨模块 #include: 20 条，涉及 8 个模块
成员指针: 21 个跨模块类型
设计模式标注: 中介者模式(Mediator)
```

MainWindow 直接持有 `TerminalModel*`、`FrameParser*`、`OtaManager*`、`DataExporter*` 等 8 个模块的具体类型指针。信号接线分散在两个文件中：
- `MainWindowSignalConnect.cpp`（424 行）
- `MainWindowPanelConnect.cpp`（137 行）

#### ConnectionFactory.cpp

```
路径:  src/core/connect/ConnectionFactory.cpp
跨模块 #include: 15 条，涉及 2 个模块
  - connection: 13 条（所有具体连接类）
  - rtt: 1 条（JLinkRttConnection）
```

工厂通过 `switch(ConnectionType)` 硬编码 13 种连接类型的实例化逻辑，每新增连接类型必须修改此文件。

### 1.5 循环依赖

项目中共发现 **8 对循环依赖**，每一对都涉及 core 模块：

| 循环对 | A→B | B→A | 严重度 |
|--------|-----|-----|--------|
| **core ↔ connection** | 42 | 3 | 🔴 严重 |
| **core ↔ serial** | 29 | 8 | 🔴 严重 |
| **core ↔ utils** | 24 | 5 | 🟠 高 |
| **core ↔ terminal** | 18 | 10 | 🟠 高 |
| **core ↔ protocol** | 16 | 4 | 🟠 高 |
| **core ↔ ota** | 9 | 6 | 🟡 中 |
| **chart ↔ core** | 7 | 11 | 🟡 中 |
| **terminal ↔ utils** | 3 | 1 | 🟢 低 |

**核心矛盾**: core 既是"顶层协调者"（需要引用所有模块），又是"基础设施提供者"（Constants、BasePanel、ThemeManager），导致双向依赖。

### 1.6 常量耦合现状

Constants 头文件目前位于 `src/core/theme/`，包含 5 个域特定子文件：

| 头文件 | 命名空间 | 内容 |
|--------|---------|------|
| AppConstants.h | `App`, `Language` + 枚举 | 应用标识、ConnectionType[13值]等 |
| LayoutConstants.h | `Layout`, `OtaLayout` | 面板尺寸、间距 |
| ConnectionConstants.h | `BaudRates`, `ConnectionDefaults` | 波特率列表、网络默认值 |
| AnimationConstants.h | `Animations` | 12 个 UI 动画时长 |
| TimerConstants.h | `Timers`, `TerminalDefaults` | 定时器间隔、终端字体 |

**问题**: 全项目有 **41 个文件（46 条 #include）** 引用 `core/theme/Constants.h`。这些纯数据常量放在 core/ 中，导致任何使用常量的模块都产生了对 core 的依赖，制造了不必要的循环。

### 1.7 现状总结

```
                    ┌─────────────────────────────────────┐
                    │              core (86 files)         │
                    │  ┌──────────┐ ┌──────────┐ ┌──────┐ │
     ┌──────────────┤  │MainWindow│ │PanelMgr  │ │Const │ │
     │              │  └────┬─────┘ └────┬─────┘ └──┬───┘ │
     │              │       │            │           │     │
     │              └───────┼────────────┼───────────┼─────┘
     │                      │            │           │
     │    ┌─────────────────┼────────────┼───────────┼──────┐
     │    │                 │            │           │      │
     ▼    ▼                 ▼            ▼           ▼      ▼
  connection  serial  terminal  protocol  chart  ota  utils  rtt
   (66f)     (25f)    (32f)     (45f)    (21f)  (19f) (39f)  (8f)
     ▲         ▲        ▲                   │     │    ▲
     │         │        │                   │     │    │
     └─────────┴────────┴───────────────────┴─────┴────┘
                      全指向 core，core 也反向指向全部
```

**一句话**: core 模块承担了"协调者"和"基础设施"双重角色，成为所有循环依赖的交汇点。

---

## 2. 解耦方案

本提案提出 **4 个正交的解耦方向**，每个方向可独立实施，也可组合使用。

### 方向 A: interfaces/ 纯虚接口层

#### 目标

在所有模块之间插入一个零依赖的接口层，使模块间通过接口指针通信，而非直接引用具体类。

#### 设计

创建 `src/interfaces/` 目录，包含以下纯虚接口：

```cpp
// src/interfaces/IConnection.h
class IConnection : public QObject {
    Q_OBJECT
public:
    virtual ~IConnection() = default;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual qint64 send(const QByteArray& data) = 0;
    virtual ConnectionState state() const = 0;
    virtual ConnectionType type() const = 0;
    virtual QString deviceInfo() const = 0;
signals:
    void dataReceived(const QByteArray& data);
    void stateChanged(ConnectionState state, const QString& msg);
    void errorOccurred(const QString& error);
};
```

```cpp
// src/interfaces/IPanelProvider.h
class IPanelProvider {
public:
    virtual ~IPanelProvider() = default;
    virtual QString panelName() const = 0;
    virtual QWidget* createPanel(QWidget* parent) = 0;
    virtual QIcon panelIcon() const = 0;
    virtual void registerCommands(CommandPalette* palette) = 0;
};
```

```cpp
// src/interfaces/IDataSink.h
class IDataSink {
public:
    virtual ~IDataSink() = default;
    virtual void onDataReceived(const QByteArray& data) = 0;
    virtual void onDataSent(const QByteArray& data) = 0;
    virtual void clearData() = 0;
};
```

```cpp
// src/interfaces/IProtocolParser.h
class IProtocolParser {
public:
    virtual ~IProtocolParser() = default;
    virtual bool parse(const QByteArray& raw) = 0;
    virtual QByteArray serialize() const = 0;
    virtual bool validate() const = 0;
};
```

```cpp
// src/interfaces/IDevice.h
class IDevice {
public:
    virtual ~IDevice() = default;
    virtual bool connect(const QString& address) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual QString deviceInfo() const = 0;
};
```

#### 约束

- **interfaces/ 目录零出站依赖** — 除了 Qt Core 类型（`QObject`、`QString`、`QByteArray`、`QIcon`），不 `#include` 任何项目头文件
- 每个接口文件不超过 200 行（符合 `.h` 约束）
- 所有模块通过接口指针与其它模块通信

#### 收益

- ConnectionFactory 改为返回 `IConnection*` 后，调用方不再需要知道具体连接类
- 新增连接类型（如未来添加 LoRa、NB-IoT）只需在 connection/ 模块内部实现，对外零影响
- 每个模块可以独立进行单元测试（通过 mock 接口）

### 方向 B: PanelManager 自注册模式

#### 目标

消除 PanelManagerCreation.cpp 的 57 条跨模块 `#include`，改为各模块自行注册面板。

#### 当前问题

```
PanelManagerCreation.cpp 引用链:
  #include "serial/config/SerialConfigPanel.h"          ← serial 模块
  #include "protocol/view/ProtocolView.h"               ← protocol 模块
  #include "connection/ble/BleConfigPanel.h"             ← connection 模块
  #include "chart/widget/ChartWidget.h"                  ← chart 模块
  #include "ota/manager/OtaManager.h"                    ← ota 模块
  ...（共 57 条，涉及 10 个模块）
```

新增一个面板需要修改 3 个文件、影响 10 个模块的编译。

#### 提议方案

**Step 1**: 定义注册接口

```cpp
// src/interfaces/IPanelProvider.h（方向 A 中已定义）
// 每个 PanelProvider 描述一个可注册的面板
```

**Step 2**: 每个模块提供 `registerPanels()` 自由函数

```cpp
// src/serial/SerialPanelRegistrar.cpp
#include "serial/config/SerialConfigPanel.h"
#include "serial/data/DataStatistics.h"
#include "serial/commands/QuickCommandBar.h"
// ...仅引用 serial 内部头文件

void Serial_registerPanels(PanelManager& mgr) {
    mgr.registerPanel("serial_config", []() -> QWidget* {
        return new SerialConfigPanel();
    }, Icons::Serial);
    mgr.registerPanel("data_stats", []() -> QWidget* {
        return new DataStatistics();
    }, Icons::Chart);
    // ...
}
```

**Step 3**: PanelManager 改为数据驱动

```cpp
// src/core/panels/PanelManager.h — 新设计
class PanelManager : public QObject {
public:
    using PanelFactory = std::function<QWidget*(QWidget*)>;

    void registerPanel(const QString& id,
                       PanelFactory factory,
                       const QIcon& icon);

    QWidget* getPanel(const QString& id) const;
    QStringList allPanelIds() const;

private:
    QMap<QString, PanelFactory> m_factories;
    QMap<QString, QPointer<QWidget>> m_cache;
    QMap<QString, QIcon> m_icons;
};
```

**Step 4**: 主初始化调用各模块注册函数

```cpp
// src/core/AppInit.cpp（新建）
#include "serial/SerialPanelRegistrar.h"   // 仅 1 条/模块
#include "protocol/ProtocolPanelRegistrar.h"
#include "connection/ConnectionPanelRegistrar.h"
// ...

void registerAllPanels(PanelManager& mgr) {
    Serial_registerPanels(mgr);
    Protocol_registerPanels(mgr);
    Connection_registerPanels(mgr);
    Chart_registerPanels(mgr);
    Ota_registerPanels(mgr);
    Terminal_registerPanels(mgr);
    Utils_registerPanels(mgr);
    Rtt_registerPanels(mgr);
    Automation_registerPanels(mgr);
    Plugin_registerPanels(mgr);
}
```

#### 效果对比

| 指标 | 改造前 | 改造后 |
|------|--------|--------|
| PanelManagerCreation.cpp 跨模块 include | 57 条 | **0 条** |
| 新增面板需修改文件数 | 3 个 | **1 个**（仅本模块 Registrar） |
| 模块间编译耦合 | 全联动 | **独立编译** |
| PanelManager.h 对其他模块的依赖 | 10 个模块 | **0 个** |

### 方向 C: Constants 提取到 shared/

#### 目标

将 Constants 从 `core/theme/` 移至独立的 `src/shared/` 目录，消除因使用常量而产生的对 core 的假性依赖。

#### 现状

46 条 `#include "core/theme/Constants.h"` 分布在 41 个文件、9 个模块中。这些文件只需要 `Layout::PADDING`、`Timers::REFRESH_MS` 等纯数值常量，却因此产生了对 core 模块的编译依赖。

#### 提议目录结构

```
src/shared/
├── constants/
│   ├── Constants.h              ← 向后兼容的聚合头文件
│   ├── AppConstants.h           ← 应用标识 + 全局枚举
│   ├── LayoutConstants.h        ← 布局尺寸
│   ├── ConnectionConstants.h    ← 波特率、连接默认值
│   ├── AnimationConstants.h     ← 动画时长
│   └── TimerConstants.h         ← 定时器间隔
└── types/
    └── CommonTypes.h            ← 跨模块共享的数据类型（可选）
```

#### 迁移步骤

1. **创建** `src/shared/constants/` 目录
2. **移动** 5 个 Constants 文件（内容不变，仅路径变更）
3. **更新** 聚合头文件 `Constants.h` 为 `#include` 6 个子文件的聚合器（新增 `CommonTypes.h`）
4. **全局替换** 引用路径：
   - `"core/theme/Constants.h"` → `"shared/constants/Constants.h"`
   - `"core/theme/AppConstants.h"` → `"shared/constants/AppConstants.h"`
   - （其余类推）
5. **更新** CMakeLists.txt 中 HEADERS 列表

#### 约束

- **shared/ 依赖 NOTHING** — 仅使用 Qt Core 基础类型（`QString`、`QColor`、`int`、`constexpr`）
- 不 `#include` 任何项目内部头文件
- 向后兼容：旧的 `#include "core/theme/Constants.h"` 路径可通过 core/ 转发头文件保持编译通过（过渡期）

#### 收益

- 消除 9 个模块对 core 的 46 条假性依赖
- 打破 `chart → core`、`ota → core`、`protocol → core` 等循环依赖中的常量路径
- 新模块可以直接引用 shared/ 而不需要知道 core 的存在

### 方向 D: 事件总线替代直接信号链

#### 目标

引入发布-订阅模式的事件总线（EventBus），解耦信号发送方和接收方。

#### 现状问题

`ConnectionController` 发出 12 个信号，`MainWindowSignalConnect.cpp`（424 行）中手动 `connect()` 了数十条点对点信号链：

```cpp
// 当前: MainWindowSignalConnect.cpp 中的手动接线
connect(m_connectionController, &ConnectionController::dataReceived,
        m_terminalModel, &TerminalModel::appendData);
connect(m_connectionController, &ConnectionController::dataReceived,
        m_recordingController, &RecordingController::onDataReceived);
connect(m_connectionController, &ConnectionController::connectionStateChanged,
        this, &MainWindow::onConnectionStateChanged);
connect(m_connectionController, &ConnectionController::connectionHealth,
        m_terminalWidget, &TerminalWidget::updateHealthIndicator);
// ... 数十条类似连线
```

**问题**:
- MainWindow 必须知道所有接收方的具体类型（`TerminalModel*`、`RecordingController*`...）
- 增加新的数据消费者必须修改 MainWindow
- 不同信号链的接线分散在两个文件（共 561 行），维护困难

#### 提议方案

**Step 1**: 定义事件类型

```cpp
// src/shared/events/EventTypes.h
#include <QByteArray>
#include <QString>
#include <QVariantMap>

// 事件类型标识
enum class EventType {
    ConnectionStateChanged,
    DataReceived,
    DataSent,
    ProtocolFrameParsed,
    OtaProgressUpdated,
    DeviceConnected,
    DeviceDisconnected,
    ErrorOccurred,
};

// 通用事件载荷
struct Event {
    EventType type;
    QVariantMap payload;
};

// 具体: 连接状态事件
struct ConnectionEvent {
    ConnectionState state;
    QString message;
    ConnectionType connType;
};

// 具体: 数据事件
struct DataEvent {
    QByteArray data;
    DataDirection direction;  // Rx / Tx
    qint64 timestamp;
};
```

**Step 2**: EventBus 单例

```cpp
// src/core/eventbus/EventBus.h
class EventBus : public QObject {
    Q_OBJECT
public:
    static EventBus& instance();

    // 发布事件（任意线程安全）
    void publish(EventType type, const QVariant& data);

    // 订阅事件（返回订阅 ID，用于取消订阅）
    int subscribe(EventType type, std::function<void(const QVariant&)> handler);

    // 取消订阅
    void unsubscribe(int subscriptionId);

private:
    EventBus() = default;
    QMap<EventType, QVector<QPair<int, std::function<void(const QVariant&)>>>> m_subscribers;
    int m_nextId = 0;
};
```

**Step 3**: 发送方改造示例

```cpp
// ConnectionController — 改造后
void ConnectionController::onDataReady() {
    QByteArray data = m_connection->readAll();

    // 旧: emit dataReceived(data);  (需要接收方直接连接)
    // 新: 发布事件，不关心谁接收
    DataEvent event{data, DataDirection::Rx, QDateTime::currentMSecsSinceEpoch()};
    EventBus::instance().publish(EventType::DataReceived, QVariant::fromValue(event));
}
```

**Step 4**: 接收方自行订阅

```cpp
// TerminalModel — 自行订阅，无需 MainWindow 中转
TerminalModel::TerminalModel() {
    EventBus::instance().subscribe(EventType::DataReceived,
        [this](const QVariant& v) {
            auto event = v.value<DataEvent>();
            this->appendData(event.data);
        });
}

// RecordingController — 自行订阅
RecordingController::RecordingController() {
    EventBus::instance().subscribe(EventType::DataReceived,
        [this](const QVariant& v) {
            auto event = v.value<DataEvent>();
            this->onDataReceived(event.data);
        });
}
```

#### 效果对比

| 指标 | 改造前 | 改造后 |
|------|--------|--------|
| MainWindow 信号接线代码 | 561 行（2 文件） | **< 50 行** |
| 新增数据消费者 | 改 MainWindow | **只改消费者自己** |
| ConnectionController 知道的接收方 | 5+ 个具体类 | **0 个** |
| 模块间编译依赖 | 发送方→接收方头文件 | **仅→EventTypes.h** |

#### 注意事项

- EventBus 是**可选**机制 — 不强制所有信号都走事件总线，Qt 原生信号槽在模块内部继续使用
- 优先对**跨模块**通信使用 EventBus，**模块内部**继续用直接信号
- 事件载荷使用 `QVariant` + `Q_DECLARE_METATYPE`，注册自定义类型

---

## 3. 模块依赖规则

### 3.1 目标依赖 DAG（有向无环图）

解耦后的依赖方向必须是严格的 DAG — 只允许向下游依赖，禁止向上游和同级横向依赖：

```
层级 0（零出站依赖）:
  ┌─────────────────────────────────────────────────┐
  │               interfaces/                        │
  │  IConnection, IPanelProvider, IDataSink,         │
  │  IProtocolParser, IDevice                        │
  │  依赖: 仅 Qt Core                                │
  └─────────────────────────────────────────────────┘
                         ▲
                         │ (所有模块依赖)
  ┌─────────────────────────────────────────────────┐
  │               shared/                            │
  │  constants/, types/CommonTypes.h                 │
  │  依赖: 仅 Qt Core + interfaces/                  │
  └─────────────────────────────────────────────────┘
                         ▲
                         │
  层级 1（基础层）:
  ┌───────────┐
  │   utils/   │  依赖: shared/, interfaces/
  └───────────┘
                         ▲
                         │
  层级 2（传输层）:
  ┌──────────┐  ┌──────────┐
  │  serial/  │  │ protocol/ │  依赖: utils, shared, interfaces
  └──────────┘  └──────────┘
                         ▲
                         │
  层级 3（功能层）:
  ┌────────────┐  ┌──────────┐  ┌───────┐  ┌─────┐
  │ connection/ │  │ terminal/ │  │ chart/ │  │ rtt/ │
  └────────────┘  └──────────┘  └───────┘  └─────┘
  依赖: serial/protocol/utils/shared/interfaces

  ┌───────────┐  ┌────────────┐  ┌───────────┐
  │    ota/    │  │ automation/ │  │ dashboard/ │
  └───────────┘  └────────────┘  └───────────┘
  依赖: connection/protocol/utils/shared/interfaces 等

  ┌───────────┐
  │  plugin/   │  依赖: interfaces/, shared/（最少依赖）
  └───────────┘
                         ▲
                         │
  层级 4（协调层）:
  ┌─────────────────────────────────────────────────┐
  │               core/                              │
  │  MainWindow, PanelManager, Controllers           │
  │  依赖: ALL above — 但仅通过 interfaces/ 指针      │
  │  不直接 #include 任何具体实现头文件                  │
  └─────────────────────────────────────────────────┘
```

### 3.2 规则总结

| 规则 | 说明 |
|------|------|
| ✅ 下游依赖 | 只允许依赖 DAG 中更深层级的模块 |
| ❌ 上游依赖 | 禁止层级 2 的模块 `#include` 层级 3/4 的头文件 |
| ❌ 同级横向依赖 | serial 和 protocol 互不可见，需通过 interfaces/ 交互 |
| ❌ 跨层依赖 | 层级 2 禁止直接依赖层级 4（必须通过事件总线或接口） |
| ✅ interfaces/ 是通用依赖 | 所有层级都可依赖 interfaces/ 和 shared/ |

### 3.3 依赖矩阵：改造前 vs 改造后

```
改造前:
                auto chart conn core dash ota plug proto rtt serial term utils
core      (86f)   2   11   42   -    2   9    2   16    3   29    18   24
serial    (25f)   0    0    3   8    0   0    0    0    0    -     0    3
terminal  (32f)   0    0    0  10    0   0    0    0    0    0     -    3
ota       (19f)   0    0    2   6    0   -    0    1    0    0     0    8
protocol  (45f)   0    0    1   4    0   0    0    -    0    0     0    4
chart     (21f)   0    -    0   7    0   0    0    2    0    0     0    0
connection(66f)   0    0    -   3    0   0    0    0    0    0     0    0
utils     (39f)   0    0    0   5    0   0    0    0    0    0     1    -
rtt        (8f)   0    0    1   0    0   0    0    0    -    0     0    0
automation(9f)    0    0    0   0    0   0    0    0    0    0     0    0
dashboard(12f)    0    0    0   0    -   0    0    0    0    0     0    0
plugin     (7f)   0    0    0   0    0   0    -    0    0    0     0    0

循环依赖: 8 对（全部涉及 core）
───────────────────────────────────────────────────────────────

改造后:
                auto chart conn core dash ota plug proto rtt serial term utils  intf  shared
core      (86f)   0    0    0   -    0   0    0    0    0    0     0    0    1    1   ← 仅接口
serial    (25f)   0    0    0   0    0   0    0    0    0    -     0    1    1    1
terminal  (32f)   0    0    0   0    0   0    0    0    0    0     -    1    1    1
ota       (19f)   0    0    0   0    0   -    0    0    0    0     0    1    1    1
protocol  (45f)   0    0    0   0    0   0    0    -    0    0     0    1    1    1
chart     (21f)   0    -    0   0    0   0    0    0    0    0     0    0    1    1
connection(66f)   0    0    -   0    0   0    0    0    0    0     0    0    1    1
utils     (39f)   0    0    0   0    0   0    0    0    0    0     0    -    1    1
rtt        (8f)   0    0    0   0    0   0    0    0    -    0     0    0    1    1
automation(9f)    -    0    0   0    0   0    0    0    0    0     0    0    1    1
dashboard(12f)    0    0    0   0    -   0    0    0    0    0     0    0    1    1
plugin     (7f)   0    0    0   0    0   0    -    0    0    0     0    0    1    1

循环依赖: 0 对
```

### 3.4 依赖规则检查

建议在 CI 中添加依赖方向检查脚本：

```bash
# scripts/check-dependencies.py — 伪代码
# 检查每个 #include 是否符合 DAG 规则
for file in src/**/*.cpp src/**/*.h:
    module = get_module(file)         # e.g., "serial"
    for include in get_includes(file):
        target = get_module(include)  # e.g., "core"
        if not is_allowed(module, target):
            report_violation(file, include)
```

---

## 4. 迁移计划

四个阶段按风险从低到高排列，每个阶段**独立可提交、独立可构建**。

### Phase 1: 基础建设（低风险，无行为变更）

**目标**: 建立基础设施，不影响任何运行时行为。

| 步骤 | 任务 | 影响文件数 | 风险 |
|------|------|-----------|------|
| 1.1 | 创建 `src/interfaces/` 目录 | 0（新增） | 🟢 |
| 1.2 | 将现有 `connection/interface/IConnection.h` 移至 `src/interfaces/` | 1 移动 | 🟢 |
| 1.3 | 创建 `IPanelProvider.h`、`IDataSink.h`、`IProtocolParser.h`、`IDevice.h` | 4 新增 | 🟢 |
| 1.4 | 创建 `src/shared/constants/` 目录 | 0（新增） | 🟢 |
| 1.5 | 移动 5 个 Constants 文件至 `src/shared/constants/` | 5 移动 | 🟢 |
| 1.6 | 在 `core/theme/Constants.h` 保留转发头文件（`#include "shared/constants/Constants.h"`） | 1 修改 | 🟢 |
| 1.7 | 更新 CMakeLists.txt 中 HEADERS 路径 | 1 修改 | 🟢 |
| 1.8 | 全局替换 46 条 `#include` 路径（可选，可延后） | 41 | 🟢 |

**预估工作量**: 1-2 天
**验证标准**: 编译通过，运行时行为零变更

### Phase 2: 接口适配（中风险）

**目标**: 将核心模块改为通过接口指针通信。

| 步骤 | 任务 | 影响文件数 | 风险 |
|------|------|-----------|------|
| 2.1 | `ConnectionFactory::create()` 返回类型改为 `IConnection*`（已是） | 0 | 🟢 |
| 2.2 | `ConnectionManager` 内部存储改为 `QList<IConnection*>`（已是） | 0 | 🟢 |
| 2.3 | `ConnectionController` 移除对具体连接类的 `#include`，仅保留 `IConnection*` | 3-5 | 🟡 |
| 2.4 | `SendController::setConnection(IConnection*)` — 已是接口，无需改动 | 0 | 🟢 |
| 2.5 | `OtaManager::setConnection(IConnection*)` — 已是接口，无需改动 | 0 | 🟢 |
| 2.6 | 新增连接类型不再需要修改 core 代码（仅需在 connection/ 模块注册） | 验证 | 🟢 |

**预估工作量**: 2-3 天
**验证标准**: 编译通过，13 种连接类型全部功能正常

### Phase 3: PanelManager 解耦（中风险，最大重构）

**目标**: PanelManager 从"包含所有面板头文件"改为"数据驱动自注册"。

| 步骤 | 任务 | 影响文件数 | 风险 |
|------|------|-----------|------|
| 3.1 | 在 PanelManager.h 中实现 `registerPanel(id, factory, icon)` | 1 | 🟢 |
| 3.2 | 创建各模块的 `XxxPanelRegistrar.cpp`（10 个文件） | 10 新增 | 🟡 |
| 3.3 | 将 PanelManagerCreation.cpp 中的创建逻辑迁移至各 Registrar | 10 新增 + 1 清空 | 🟡 |
| 3.4 | 将 PanelManager.cpp 中的 getter 改为 `getPanel(id)` 查表 | 1 修改 | 🟡 |
| 3.5 | 删除 `PanelManagerPanels.h`（前向声明不再需要） | 1 删除 | 🟢 |
| 3.6 | 创建 `AppInit.cpp` 调用所有 Registrar | 1 新增 | 🟢 |
| 3.7 | 更新 `MainWindow` 中对面板类型指针的使用（改为 `QWidget*` 或接口） | 3-5 | 🟡 |

**预估工作量**: 3-5 天
**验证标准**: 所有面板正常创建、切换、销毁，导航功能不受影响

### Phase 4: 事件总线（低风险，纯增量）

**目标**: 引入 EventBus 替代跨模块直接信号链。

| 步骤 | 任务 | 影响文件数 | 风险 |
|------|------|-----------|------|
| 4.1 | 创建 `src/shared/events/EventTypes.h` 定义事件类型 | 1 新增 | 🟢 |
| 4.2 | 创建 `src/core/eventbus/EventBus.h/.cpp` 单例 | 2 新增 | 🟢 |
| 4.3 | 将 `ConnectionController` 的 12 个信号逐个迁移为 EventBus 发布 | 1-3 | 🟡 |
| 4.4 | 将 `TerminalModel`、`RecordingController` 等接收方改为订阅 | 3-5 | 🟡 |
| 4.5 | 逐步删除 `MainWindowSignalConnect.cpp` 和 `MainWindowPanelConnect.cpp` 中的手动接线 | 2 | 🟡 |
| 4.6 | 最终目标：`MainWindowSignalConnect.cpp` 从 424 行降至 < 50 行 | 验证 | 🟢 |

**预估工作量**: 3-5 天
**验证标准**: 全部信号传递功能正常，无信号丢失或重复

### 4.1 阶段关系图

```
Phase 1 (基础建设)    ← 前置条件，无风险
    │
    ├──→ Phase 2 (接口适配)  ← 依赖 Phase 1 的 interfaces/
    │
    ├──→ Phase 3 (面板解耦)  ← 依赖 Phase 1 的 interfaces/ + shared/
    │
    └──→ Phase 4 (事件总线)  ← 依赖 Phase 1 的 shared/，可与 Phase 2/3 并行
```

每个 Phase 完成后进行完整编译 + 功能验证，确保增量提交不断主干。

---

## 5. 预期收益

### 5.1 编译时间

| 指标 | 改造前 | 改造后 | 改善 |
|------|--------|--------|------|
| 修改一个面板头文件影响范围 | 全项目重新编译（~358 文件） | 仅本模块 + core | **~80% 降低** |
| 修改 Constants.h 影响范围 | 41 个文件重编译 | 41 个文件（不变） | N/A |
| 新增连接类型影响范围 | core/ 重编译 | connection/ 重编译 | **~90% 降低** |
| 理想增量编译时间 | ~120 秒（全量） | ~20 秒（单模块） | **~6x 加速** |

### 5.2 模块可测试性

| 场景 | 改造前 | 改造后 |
|------|--------|--------|
| 单元测试 SerialConfigPanel | 需要 mock core/MainWindow 整个依赖树 | **仅 mock IConnection 接口** |
| 单元测试 OtaManager | 需要 mock ConnectionController + ConnectionFactory | **仅注入 IConnection* mock** |
| 单元测试 ChartWidget | 需要 mock core/ThemeManager + Constants | **仅引用 shared/constants/** |
| 测试代码编译时间 | 慢（大量依赖） | **快（仅接口）** |

### 5.3 团队可扩展性

| 场景 | 改造前 | 改造后 |
|------|--------|--------|
| 新成员理解代码 | 需要理解 core/ 的 86 个文件和全部 12 模块 | **只需理解 interfaces/（5 个接口）+ 自己的模块** |
| 新增一个模块 | 需要修改 core/PanelManager + MainWindow + Constants | **仅实现 5 个接口 + 提交 Registrar** |
| 两人并行开发 | 合并冲突频繁（PanelManagerCreation.cpp 共享） | **各自模块独立，零冲突** |
| 模块所有权 | 无明确边界 | **每人可负责 1-2 个模块** |

### 5.4 新功能开发效率

以"新增 LoRa 连接类型"为例：

| 步骤 | 改造前 | 改造后 |
|------|--------|--------|
| 1. 创建 LoRaConnection 类 | ✅ | ✅ |
| 2. 在 ConnectionType 枚举添加 LoRa | 改 AppConstants.h（core/） | **改 shared/constants/AppConstants.h** |
| 3. 在 ConnectionFactory switch 添加 case | 改 ConnectionFactory.cpp（core/） | **在 connection/ 模块内注册** |
| 4. 在 PanelManager 添加 LoRaConfigPanel | 改 3 个 core/ 文件 | **connection/ 模块内注册** |
| 5. 在 MainWindow 接线 | 改 2 个文件 | **不需要（EventBus 自订阅）** |
| **总计改动文件数** | **7+ 个** | **2-3 个（全部在 connection/ 内）** |

### 5.5 量化指标总结

| 指标 | 改造前 | 改造后 | 改善幅度 |
|------|--------|--------|---------|
| core 出站跨模块引用 | 158 条 | ~10 条（仅接口） | **-94%** |
| 循环依赖对数 | 8 对 | 0 对 | **-100%** |
| PanelManager 跨模块 include | 114 条（57×2） | 0 条 | **-100%** |
| MainWindow 信号接线代码 | 561 行 | < 50 行 | **-91%** |
| 新增功能需改动模块数 | 3-5 个 | 1 个 | **-75%** |
| 可独立单元测试的模块 | 3/12（automation/dashboard/plugin） | **12/12** | **+300%** |

---

## 附录 A: 文件清单参考

### interfaces/ 目录规划

```
src/interfaces/
├── IConnection.h          (已有，从 connection/interface/ 迁移)
├── IPanelProvider.h       (新增)
├── IDataSink.h            (新增)
├── IProtocolParser.h      (新增)
└── IDevice.h              (新增)
```

### shared/ 目录规划

```
src/shared/
├── constants/
│   ├── Constants.h              (聚合头文件，向后兼容)
│   ├── AppConstants.h           (从 core/theme/ 移动)
│   ├── LayoutConstants.h        (从 core/theme/ 移动)
│   ├── ConnectionConstants.h    (从 core/theme/ 移动)
│   ├── AnimationConstants.h     (从 core/theme/ 移动)
│   └── TimerConstants.h         (从 core/theme/ 移动)
├── events/
│   └── EventTypes.h             (新增)
└── types/
    └── CommonTypes.h            (新增，可选)
```

### PanelRegistrar 文件规划

```
src/serial/SerialPanelRegistrar.cpp
src/protocol/ProtocolPanelRegistrar.cpp
src/connection/ConnectionPanelRegistrar.cpp
src/chart/ChartPanelRegistrar.cpp
src/ota/OtaPanelRegistrar.cpp
src/terminal/TerminalPanelRegistrar.cpp
src/utils/UtilsPanelRegistrar.cpp
src/rtt/RttPanelRegistrar.cpp
src/automation/AutomationPanelRegistrar.cpp
src/plugin/PluginPanelRegistrar.cpp
```

---

## 附录 B: 术语表

| 术语 | 定义 |
|------|------|
| DAG | 有向无环图，此处指模块依赖关系不能有环 |
| 上帝模块 | 承担过多职责的中心模块（此处为 core） |
| 上帝类 | 引用过多其他模块的类（如 PanelManager） |
| 自注册 | 模块主动将自己的面板注册到 PanelManager，而非 PanelManager 引用模块 |
| 事件总线 | 发布-订阅模式的中央事件分发器 |
| 接口层 | 仅包含纯虚基类的目录，零实现依赖 |
| 假性依赖 | 因常量/类型定义而产生的非功能性编译依赖 |
| 转发头文件 | 旧路径的兼容头文件，仅 `#include` 新路径 |
| 叶子模块 | 不依赖任何其他模块的终端模块（如 automation、dashboard、plugin） |

---

> **下一步**: 本提案经评审通过后，按 Phase 1 → 2 → 3 → 4 顺序逐步实施。每个 Phase 完成后进行 code review + 全量回归测试。
