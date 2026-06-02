# 03 - 架构原则

> 本文档是 EmbedDebug 约束体系的第3模块。涉及架构/新增类时必须加载。

---

## 一、设计模式要求

| 模式 | 应用场景 | 项目中的使用 |
|------|---------|-------------|
| **策略模式 (Strategy)** | OTA协议切换 | `IProtocol` 接口 + 具体协议实现 |
| **观察者模式 (Observer)** | 数据流分发 | Qt 信号/槽机制 |
| **工厂模式 (Factory)** | 创建不同类型的连接 | `ConnectionFactory` |
| **状态模式 (State)** | 连接状态管理 | `ConnectionState` 枚举驱动行为 |
| **单例模式 (Singleton)** | 全局管理器 | `SettingsManager`, `ThemeManager` |
| **模板方法 (Template Method)** | OTA传输流程 | `BaseTransfer::execute()` 骨架 |
| **适配器模式 (Adapter)** | J-Link SDK适配 | `JLinkBridge` 适配 `IConnection` |
| **命令模式 (Command)** | 快捷指令、发送历史 | `QuickCommand` 数据结构 |

---

## 二、分层架构

```
┌─────────────────────────────────┐
│        表现层 (Presentation)     │  QWidget / QPainter
│  MainWindow, ConfigPanel, View  │  只做UI展示，不含业务逻辑
├─────────────────────────────────┤
│        业务层 (Business)        │  QObject
│  ConnectionMgr, OtaManager,    │  业务逻辑编排，协调各模块
│  ProtocolEngine, ChartManager  │
├─────────────────────────────────┤
│        数据层 (Data)            │  QObject / 纯C++
│  TerminalModel, FrameParser,   │  数据模型、解析、计算
│  CRC, RingBuffer, DataLogger   │
├─────────────────────────────────┤
│        基础设施层 (Infra)       │  纯C++ / Qt底层封装
│  IConnection, SerialConnection │  硬件/OS抽象、IO操作
│  JLinkBridge, SettingsManager │
└─────────────────────────────────┘
```

### 依赖规则（单向，不可反向）

```
表现层 → 业务层 → 数据层 → 基础设施层
```

- 表现层可以依赖业务层和数据层
- 业务层可以依赖数据层和基础设施层
- **基础设施层不能依赖任何上层**
- **数据层不能依赖表现层**

---

## 三、公共组件清单（只写一次，全局复用）

| 组件 | 文件 | 用途 |
|------|------|------|
| `CRC` | `utils/crypto/CRC.h` | CRC8/CRC16-CCITT/CRC16-Modbus/CRC32/checksum |
| `HexConverter` | `utils/crypto/HexConverter.h` | HEX编码/解码/校验 |
| `RingBuffer<T>` | `utils/data/RingBuffer.h` | 线程安全环形缓冲区模板 |
| `SettingsManager` | `utils/settings/SettingsManager.h/cpp` | 单例，配置持久化 |
| `ThemeManager` | `core/theme/ThemeManager.h/cpp` | 单例，主题切换 |
| `DataLogger` | `utils/log/DataLogger.h/cpp` | 日志记录/回放 |
| `IConnection` | `connection/interface/IConnection.h` | 连接抽象接口 |
| `TerminalModel` | `terminal/model/TerminalModel.h/cpp` | 终端数据模型 |
| `TerminalWidget` | `terminal/widget/TerminalWidget.h/cpp` | 自绘制终端控件 |
| `TerminalSearchBar` | `terminal/search/TerminalSearchBar.h/cpp` | 终端搜索栏 |
| `Constants` | `core/theme/Constants.h` | 全局枚举和常量 |
| `ChannelConfig` | `chart/model/ChannelConfig.h/cpp` | 通道配置 |
| `ChartModel` | `chart/model/ChartModel.h/cpp` | 图表数据模型 |
| `SendController` | `core/send/SendController.h/cpp` | 发送控制器 |
| `NavigationController` | `core/navigation/NavigationController.h/cpp` | 导航控制器 |
| `RecordingController` | `core/recording/RecordingController.h/cpp` | 录制控制器 |
| `ConnectionController` | `core/connect/ConnectionController.h/cpp` | 连接控制器 |
| `IProtocolBridge` | `protocol/bridge/IProtocolBridge.h` | 协议桥抽象接口 |
| `JustFloatBridge` | `protocol/bridge/JustFloatBridge.h/cpp` | JustFloat协议桥 |
| `FireWaterBridge` | `protocol/bridge/FireWaterBridge.h/cpp` | FireWater协议桥 |

**规则**: 任何新功能需要上述能力时，直接复用，不得重写。

---

## 四、新增类的检查清单

创建任何新类之前，必须确认:
- [ ] 是否有现有的公共组件可以复用？
- [ ] 这个类属于哪一层（表现/业务/数据/基础设施）？
- [ ] 它的依赖是否满足单向规则（不反向依赖）？
- [ ] 是否需要新的设计模式？如果需要，在PRD中说明
- [ ] 接口是否足够抽象，方便未来扩展？

---

## 五、文件体积约束（铁律）

| 文件类型 | 行数上限 | 说明 |
|---------|---------|------|
| `.cpp` 实现文件 | **500行** | 超过说明职责过多，需要拆分 |
| `.h` 头文件 | **200行** | 超过说明成员/方法过多 |
| 单个方法 | **80行** | 超过说明逻辑过于复杂 |

**MainWindow特殊规则**:
- MainWindow只做: 初始化对象 → 组装UI → 连接信号/槽
- **禁止在MainWindow中编写业务逻辑** — 委托给Controller/Manager
- **MainWindow.cpp目标行数: ≤500行**

**数据结构设计原则**:
- 优先使用 Qt 内置类型 (QByteArray, QVector, QMap, QHash)
- struct 用于纯数据，class 用于带行为的对象
- 枚举类 (enum class) 优先于传统枚举 (enum)

**架构整洁度检查（每次commit前）**:
- [ ] 本commit是否让某个文件突破了行数上限？
- [ ] MainWindow.cpp是否比上次commit更精简了？
- [ ] 新增逻辑是否应该提取到独立的管理类中？
- [ ] 数据结构是否放在了正确的层次？
