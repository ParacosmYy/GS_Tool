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

---

## 六、BasePanel容器模式

BasePanel 是所有可切换面板的标准包装容器, 提供统一的标题栏、折叠动画、深度层级和视觉风格。

### 包装模式(Wrapper Pattern)

```
PanelManager
├── m_wrappers: QMap<QWidget*, BasePanel*>   // 内容面板 → 包装器映射
│
├── SerialConfigPanel* ──wrap──→ BasePanel(icon="cable", title="串口配置")
├── TcpConfigPanel*    ──wrap──→ BasePanel(icon="globe", title="TCP连接")
├── ChartWidget*       ──wrap──→ BasePanel(icon="bar-chart-2", title="波形")
└── ...
```

### 关键规则

1. **PanelManager 持有所有 BasePanel wrapper**
   - `m_wrappers` 维护内容面板到包装器的映射
   - getter 方法返回内部内容面板(FooPanel*), 非 BasePanel*

2. **NavigationController 操作 BasePanel wrapper**
   - show/hide/switch 动画作用在 BasePanel 层
   - 内容面板不直接参与动画

3. **免包装面板(3个例外)**

| 面板 | 原因 | 处理方式 |
|------|------|---------|
| Terminal | 始终可见, 不参与切换 | 直接添加到布局, 无包装 |
| SearchBar | overlay叠加层, 非面板 | 浮于终端之上, 独立显示/隐藏 |
| QuickCmdBar | 底部固定栏 | 固定在发送区下方, 始终可见 |

### BasePanel 接口

```cpp
class BasePanel : public QWidget {
    Q_OBJECT
public:
    explicit BasePanel(QWidget* content, const QString& icon,
                       const QString& title, QWidget* parent = nullptr);

    QWidget* contentWidget() const;    // 获取内部内容面板
    void setCollapsed(bool collapsed);  // 折叠/展开
    bool isCollapsed() const;

    void showAnimated();   // 250ms OutCubic fade+slide
    void hideAnimated();   // 200ms InCubic fade+slide
};
```

---

## 七、PanelManager注册机制(新增面板检查清单)

向 PanelManager 注册新面板时, 必须按以下步骤逐一完成:

### 检查清单

1. **PanelManagerPanels.h** 添加前向声明
   ```cpp
   class FooPanel;  // 新增
   ```

2. **PanelManager.h** 添加成员指针 + getter 声明
   ```cpp
   private:
       FooPanel* m_fooPanel = nullptr;
   public:
       FooPanel* fooPanel() const;
   ```

3. **PanelManagerCreation.cpp** 的 `createPanels()` 中执行:
   ```cpp
   // 1. 创建面板
   m_fooPanel = new FooPanel(this);
   m_fooPanel->setObjectName("fooPanel");
   m_fooPanel->setVisible(false);

   // 2. 创建 BasePanel 包装器
   auto* wrapper = new BasePanel(m_fooPanel, "icon-name", tr("面板标题"), this);
   wrapper->setObjectName("fooPanelWrapper");
   wrapper->setVisible(false);

   // 3. 注册到映射表
   m_wrappers.insert(m_fooPanel, wrapper);
   ```

4. **PanelManager.cpp** 的 getter 中添加一行:
   ```cpp
   FooPanel* PanelManager::fooPanel() const { return m_fooPanel; }
   ```

5. **PanelManager.cpp** 的 `panelMappings()` 中添加导航条目:
   ```cpp
   { PanelId::FooPanel, NavPanelMapping{...} },
   ```

6. **PanelManager.cpp** 的 `allPanels()` 中添加面板指针:
   ```cpp
   m_fooPanel,
   ```

### 验证

注册完成后确认:
- [ ] 面板在导航树中可见且可点击
- [ ] 点击后面板切换动画正常
- [ ] 三套主题 QSS 中已添加对应 objectName 样式
- [ ] 面板在 <900px 窗口下正确适配

---

## 八、IConnection扩展指南

新增连接类型时, 遵循以下步骤:

### 步骤

1. **继承 IConnection 接口**
   ```cpp
   class FooConnection : public IConnection {
       Q_OBJECT
   public:
       bool open() override;
       void close() override;
       qint64 send(const QByteArray& data) override;
       void receive() override;
       // ...
   };
   ```

2. **在 ConnectionFactory 注册新枚举值**
   ```cpp
   // ConnectionType 枚举中新增
   enum class ConnectionType {
       Serial,
       Tcp,
       Udp,
       Foo,    // 新增
   };

   // create() 方法中新增分支
   case ConnectionType::Foo:
       return new FooConnection(parent);
   ```

3. **在 ConnectionController 的 connect 方法中处理新类型**
   - 添加新连接类型的配置读取逻辑
   - 连接成功/失败的状态处理
   - 错误提示信息

4. **创建对应 ConfigPanel**
   - 继承 QWidget, 设置 objectName
   - 包含新连接类型的配置表单
   - 注册到 PanelManager (参照§七检查清单)

5. **配置 QSS**
   - 在三套主题文件中为新面板的 objectName 添加样式
   - 遵循 05-ui-standard.md 中的配色和间距规范

---

## 九、ProtocolEngine插件接口

### 协议定义结构

```cpp
struct ProtocolSchema {
    QString name;           // 协议名称, 如 "Modbus RTU"
    QString version;        // 协议版本, 如 "1.0"
    QByteArray header;      // 帧头标识字节
    QByteArray footer;      // 帧尾标识字节
    int maxFrameLength;     // 最大帧长度
    QVector<ProtocolField> fields;  // 字段定义列表
};
```

### ProtocolEngine 解析流程

```
字节流 → 帧检测(头/尾匹配) → 校验验证 → 字段提取 → 结构化数据
```

- ProtocolEngine 按 Schema 解析字节流
- 支持动态加载自定义协议定义(JSON格式)
- 解析结果通过 Qt 信号分发到各消费方

### 内置协议模板

| 模板 | 说明 | 文件 |
|------|------|------|
| Modbus RTU | CRC16校验, 功能码解析 | `protocol/modbus/` |
| COBS | 一致性开销字节填充 | `protocol/parser/` |
| SLIP | 串行线路IP封装 | `protocol/parser/` |
| JustFloat | 浮点数组流协议 | `protocol/bridge/JustFloatBridge` |
| FireWater | 火水协议(类似JustFloat) | `protocol/bridge/FireWaterBridge` |

### ProtocolFieldMapper

将协议字段映射到 ChartModel 的数据通道:

```cpp
class ProtocolFieldMapper {
public:
    // 注册映射: 协议字段名 → 图表通道索引
    void addMapping(const QString& fieldName, int channelIndex);

    // 从解析后的协议数据中提取通道值
    QMap<int, double> extractChannels(const QJsonObject& parsedFrame) const;
};
```

### 自定义协议扩展规则

1. 定义 JSON Schema 文件描述协议帧格式
2. 在 ProtocolEngine 中注册新 Schema
3. 配置 ProtocolFieldMapper 映射到图表通道
4. 创建对应 ConfigPanel 供用户配置参数
