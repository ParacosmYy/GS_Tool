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

## 二、统一依赖矩阵

### 2.1 分层定义

> 下面的分层是本项目的唯一架构口径。`shared/` 是正式的公共基础层；当前仓库仍在使用 `core/theme/Constants.h` 作为兼容伞头，但它只允许承接旧入口，不允许继续扩张。

| 层级 | 目录 | 职责边界 | 允许依赖 |
|------|------|----------|---------|
| L0 | `src/interfaces/` | 纯虚接口和契约，不承载业务实现 | 无 |
| L1 | `src/shared/` | 跨模块共享常量、枚举、轻量类型别名、无状态工具 | 仅 Qt 基础类型 |
| L2 | `src/utils/` | 通用算法、编解码、导出、日志、缓存等基础能力 | L1, L0 |
| L3 | `src/connection/`、`src/protocol/`、`src/serial/` | 外设接入、协议解析、串口相关功能 | L2, L1, L0 |
| L4 | `src/terminal/`、`src/chart/`、`src/rtt/` | 终端显示、图表展示、RTT 接入等面向数据呈现的模块 | L3, L2, L1, L0 |
| L5 | `src/ota/`、`src/automation/`、`src/dashboard/`、`src/plugin/` | 面向场景的业务编排、扩展和自动化 | L4, L3, L2, L1, L0 |
| L6 | `src/core/` | 应用协调、基础 UI、面板编排、导航、会话、主题运行时 | 可依赖全部下层，不得被下层依赖 |

### 2.2 依赖矩阵

```
L6 core/        →  L5 / L4 / L3 / L2 / L1 / L0
L5 ota...       →  L4 / L3 / L2 / L1 / L0
L4 terminal...  →  L3 / L2 / L1 / L0
L3 connection.. →  L2 / L1 / L0
L2 utils/       →  L1 / L0
L1 shared/      →  仅 Qt 基础类型
L0 interfaces/  →  无
```

### 2.3 依赖规则

- 依赖方向只能自下而上，禁止反向 include。
- 同层模块之间禁止横向 include，必须通过接口、事件总线或下沉到 shared/ 的纯数据定义解耦。
- 除 `core/` 外，任何模块都不得跳过中间层直接依赖高层实现。
- `shared/` 只放“跨模块稳定事实”，不放 UI 逻辑、不放业务编排、不放可变状态。
- `core/theme/Constants.h` 只做过渡伞头，新增常量必须写入 `shared/` 的正式入口，不能继续堆在伞头里。

### 2.4 唯一真相路径

| 主题 | 唯一真相路径 | 备注 |
|------|--------------|------|
| 共享常量与枚举 | `src/shared/` | 新增跨模块常量、枚举、轻量值类型优先落这里 |
| 旧常量伞头 | `src/core/theme/Constants.h` | 仅兼容旧 include，不再新增域定义 |
| 未来功能骨架 | `src/features/` | 仅作为未来迁移的归属说明和骨架入口，不承载现有业务实现 |
| 应用协调入口 | `src/core/mainwindow/MainWindow.*` | 顶层窗口组装与生命周期协调 |
| 面板编排 | `src/core/panels/PanelManager.*` | 面板创建、注册、包装、映射、切换统计 |
| 基础 UI 组件 | `src/core/widgets/` | BasePanel、EmptyStateWidget、LoadingSpinner 等复用壳层 |
| 导航与切换 | `src/core/navigation/` | 导航树、指示器、切换动画、路由控制 |
| 主题运行时 | `src/core/theme/ThemeManager.*` | 主题切换、QSS 加载、运行时主题状态 |

### 2.5 核心收敛原则

- `core/` 的定位是“应用协调 + 基础 UI”，不是功能桶。
- 新功能如果属于外设接入、协议、终端、图表、自动化，优先归回各自模块，不要再塞进 `core/`。
- `PanelManager` 和 `MainWindow` 只能做编排，不能回流业务计算、解析、IO、缓存和协议分发逻辑。
- 任何新公共能力先判断是否属于 `shared/`，只有真正需要运行时行为的内容才进入 `core/`。
- 历史分叉目录只允许冻结，不允许再向外扩张；新增实现优先进入 canonical 路径，不要再造平行目录。

### 2.5-A 迁移骨架

> 更细的迁移说明与目录入口见 `docs/architecture/README.md`。

| 区域 | 目标口径 | 说明 |
|------|----------|------|
| `src/shared/` | 公共基础层 | 常量、枚举、轻量值对象、无状态 helper |
| `src/interfaces/` | 契约层 | 纯虚接口、回调协议、跨模块抽象 |
| `src/core/` | 应用协调层 | 顶层装配、导航、主题、基础 UI、会话 |
| `src/features/` | 未来功能承接层 | 仅写归属说明和迁移骨架，不直接堆实现 |
| `src/connection/` 等现有业务模块 | 业务模块层 | 继续按领域收敛，不再回流到 core |

### 2.5-B 冻结目录

以下目录只允许兼容维护，不允许继续复制新分支：

- `animation2/`
- `widgets2/`
- `loader2/`
- `fonts/`
- `icons/`
- `responsive/`
- `shortcut/` 与 `managers/` 的并行能力
- `font/` 与 `fonts/`
- `icon/` 与 `icons/`

冻结规则:
- 冻结目录可以保留历史兼容和引用转发。
- 冻结目录不接受新功能实现。
- 新功能只能进入 canonical 路径或新定义的 `src/features/` 迁移入口。

### 2.6 中心化风险与整改阶段

| 风险点 | 当前表现 | 整改建议 |
|--------|----------|----------|
| `MainWindow` 过载 | 仍持有较多控制器、面板、布局和信号路由入口 | Phase 1 先冻结新增职责，新增交互优先下沉到独立 Controller/Manager |
| `PanelManager` 过载 | 集中创建大量面板，容易演变成“面板总桶” | Phase 2 把面板创建、包装、注册拆成更细的 Provider/Factory 边界，避免继续膨胀 |
| `core/theme/Constants.h` 伞头化 | 旧常量入口承接过多 include，掩盖真实依赖 | Phase 3 迁移新常量到 `shared/`，仅保留向后兼容的转发头 |
| `core/` 职责回流 | 新特性容易以“先放 core/ 里再说”的方式进入主线 | Phase 4 以模块归属表做准入检查，非协调类逻辑不得进入 `core/` |

---

## 三-A、模块依赖规则（DAG）

> 基于 src/ 顶层模块的依赖方向图，所有 `#include` 必须遵守。

### 依赖层级

```
允许的依赖方向（自上而下）:
Layer 0: interfaces/    — 零出站依赖，纯虚接口
Layer 1: shared/        — 仅依赖 Qt 基础类型，常量+枚举
Layer 2: utils/         — 依赖 shared/、interfaces/
Layer 3: connection/, protocol/, serial/  — 依赖 utils/shared/interfaces
Layer 4: terminal/, chart/, rtt/  — 依赖 Layer 3 + shared + interfaces
Layer 5: ota/, automation/, dashboard/, plugin/  — 依赖 Layer 4 + shared + interfaces
Layer 6: core/          — 依赖所有下层，但不得被下层依赖
```

### 禁止规则

| 规则 | 说明 | 示例 |
|------|------|------|
| **禁止反向依赖** | 低层模块不得 `#include` 高层模块头文件 | `utils/` 不能 include `core/` |
| **禁止同层横向依赖** | 同层模块之间不得直接 include | `chart/` 不能直接 include `terminal/` |
| **禁止跨层依赖** | 除 core 外，不得跳层 include | `ota/` 不能直接 include `core/` |
| **唯一例外: core/** | core/ 可直接依赖所有模块，但推荐通过接口指针 | `core/` 可 include `connection/IConnection.h` |

### 违规检测

每次架构审查（每5次commit）需检查:
- [ ] 无反向依赖（grep低层include高层）
- [ ] 无同层横向依赖（grep同层互相include）
- [ ] core/ 外无跨层跳级

---

## 三-B、接口契约

> 纯虚接口定义在 `src/interfaces/`。详见 `docs/architecture/DECOUPLING_PROPOSAL.md`。

### IConnection — 连接抽象

| 项 | 说明 |
|-----|------|
| **用途** | 统一多种连接方式（串口/TCP/UDP/BLE/CAN/MQTT/WebSocket等）的操作接口 |
| **实现模块** | `connection/` 下各具体连接类（SerialConnection, TcpConnection等） |
| **消费模块** | `core/`（ConnectionController）, `ota/`, `rtt/` |

```cpp
class IConnection {
public:
    virtual ~IConnection() = default;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual qint64 send(const QByteArray& data) = 0;
    virtual ConnectionState state() const = 0;
    virtual QString errorString() const = 0;
};
```

### IPanelProvider — 面板提供者

| 项 | 说明 |
|-----|------|
| **用途** | 模块向 PanelManager 注册面板的标准接口，支持自注册模式 |
| **实现模块** | 各拥有配置面板的模块（serial/, connection/, protocol/等） |
| **消费模块** | `core/`（PanelManager） |

```cpp
class IPanelProvider {
public:
    virtual ~IPanelProvider() = default;
    virtual QWidget* createPanel(QWidget* parent) = 0;
    virtual QString panelId() const = 0;
    virtual QString panelIcon() const = 0;
    virtual QString panelTitle() const = 0;
};
```

### IDataSink — 数据接收者

| 项 | 说明 |
|-----|------|
| **用途** | 数据流分发目标接口，协议解析后的数据通过此接口路由到终端/图表/录制 |
| **实现模块** | `terminal/`（TerminalModel）, `chart/`（ChartModel）, `utils/`（DataLogger） |
| **消费模块** | `protocol/`（ProtocolEngine）, `connection/`（数据分发） |

```cpp
class IDataSink {
public:
    virtual ~IDataSink() = default;
    virtual void onRawData(const QByteArray& data) = 0;
    virtual void onParsedData(const QJsonObject& frame) = 0;
    virtual void clear() = 0;
};
```

### IProtocolParser — 协议解析器

| 项 | 说明 |
|-----|------|
| **用途** | 可插拔的协议解析接口，支持动态注册自定义协议 |
| **实现模块** | `protocol/` 下各协议引擎（ModbusEngine, FrameParser, JustFloatBridge等） |
| **消费模块** | `core/`（ProtocolEngine编排）, `automation/`（触发器匹配） |

```cpp
class IProtocolParser {
public:
    virtual ~IProtocolParser() = default;
    virtual bool parse(const QByteArray& raw, QJsonObject& out) = 0;
    virtual QString protocolName() const = 0;
    virtual QByteArray frameHeader() const = 0;
};
```

### IDevice — 设备描述

| 项 | 说明 |
|-----|------|
| **用途** | 统一设备档案接口，描述连接的嵌入式设备属性 |
| **实现模块** | `core/`（DeviceProfile） |
| **消费模块** | `ota/`（固件校验）, `rtt/`（RTT通道配置）, `protocol/`（协议自适应） |

```cpp
class IDevice {
public:
    virtual ~IDevice() = default;
    virtual QString deviceName() const = 0;
    virtual QString firmwareVersion() const = 0;
    virtual QString mcuFamily() const = 0;
    virtual QMap<QString, QVariant> capabilities() const = 0;
};
```

---

## 三-C、解耦指南

> 完整方案详见 `docs/architecture/DECOUPLING_PROPOSAL.md`。

### 解耦检查清单

每次新增跨模块调用时，必须逐条确认:

- [ ] **模块间通信通过接口指针** — 不直接 include 具体实现类的头文件
- [ ] **模块注册通过自注册模式** — 使用 IPanelProvider 等接口自动注册，不在 core/ 硬编码
- [ ] **常量从 shared/ 引入** — 跨模块常量、枚举和轻量类型统一由 shared/ 域头文件提供
- [ ] **跨模块事件通过事件总线** — 不直接 connect 不同模块的信号，通过中间事件总线路由

### 解耦优先级

| 阶段 | 目标 | 涉及模块 |
|------|------|---------|
| Phase 1 | 抽取 interfaces/ 纯虚接口 | IConnection, IProtocolParser, IDataSink |
| Phase 2 | 落地 shared/ 常量+枚举 | AppConstants, AnimationConstants, LayoutConstants, ConnectionConstants, TimerConstants |
| Phase 3 | 模块自注册机制 | IPanelProvider, 插件式面板加载 |
| Phase 4 | 事件总线 | 跨模块数据分发，替代直接信号连接 |

---

## 三、公共组件清单（只写一次，全局复用）

### 核心基础组件

| 组件 | 文件 | 用途 |
|------|------|------|
| `CRC` | `utils/crypto/CRC.h` | CRC8/CRC16-CCITT/CRC16-Modbus/CRC32/checksum |
| `HexConverter` | `utils/crypto/HexConverter.h` | HEX编码/解码/校验 |
| `RingBuffer<T>` | `utils/data/RingBuffer.h` | 线程安全环形缓冲区模板 |
| `SettingsManager` | `utils/settings/SettingsManager.h/cpp` | 单例，配置持久化 |
| `ThemeManager` | `core/theme/ThemeManager.h/cpp` | 单例，主题切换 |
| `DataLogger` | `utils/log/DataLogger.h/cpp` | 日志记录/回放 |

### 连接与协议组件

| 组件 | 文件 | 用途 |
|------|------|------|
| `IConnection` | `connection/interface/IConnection.h` | 连接抽象接口 |
| `IProtocolBridge` | `protocol/bridge/IProtocolBridge.h` | 协议桥抽象接口 |
| `JustFloatBridge` | `protocol/bridge/JustFloatBridge.h/cpp` | JustFloat协议桥 |
| `FireWaterBridge` | `protocol/bridge/FireWaterBridge.h/cpp` | FireWater协议桥 |

### 终端组件

| 组件 | 文件 | 用途 |
|------|------|------|
| `TerminalModel` | `terminal/model/TerminalModel.h/cpp` | 终端数据模型 |
| `TerminalWidget` | `terminal/widget/TerminalWidget.h/cpp` | 自绘制终端控件 |
| `TerminalSearchBar` | `terminal/search/TerminalSearchBar.h/cpp` | 终端搜索栏 |

### 图表组件

| 组件 | 文件 | 用途 |
|------|------|------|
| `ChannelConfig` | `chart/model/ChannelConfig.h/cpp` | 通道配置 |
| `ChartModel` | `chart/model/ChartModel.h/cpp` | 图表数据模型 |

### 控制器组件

| 组件 | 文件 | 用途 |
|------|------|------|
| `SendController` | `core/send/SendController.h/cpp` | 发送控制器 |
| `NavigationController` | `core/navigation/NavigationController.h/cpp` | 导航控制器 |
| `RecordingController` | `core/recording/RecordingController.h/cpp` | 录制控制器 |
| `ConnectionController` | `core/connect/ConnectionController.h/cpp` | 连接控制器 |

### UI基础组件（新增）

| 组件 | 文件 | 用途 |
|------|------|------|
| `BasePanel` | `core/widgets/BasePanel.h/cpp` | 面板统一包装容器（标题栏/折叠/动画） |
| `EmptyStateWidget` | `core/widgets/EmptyStateWidget.h/cpp` | 空状态提示组件（图标+文字+操作按钮） |
| `LoadingSpinner` | `core/widgets/LoadingSpinner.h/cpp` | 加载旋转指示器 |
| `SkeletonWidget` | `core/widgets/SkeletonWidget.h/cpp` | 骨架屏占位组件 |
| `CommandPalette` | `core/widgets/CommandPalette.h/cpp` | Ctrl+P全局命令面板（模糊搜索） |
| `SmartAutoComplete` | `core/widgets/SmartAutoComplete.h/cpp` | 智能补全弹窗（前缀匹配，最多8条） |
| `ScriptRecorder` | `core/widgets/ScriptRecorder.h/cpp` | 脚本录制回放控件 |
| `DataDiffWidget` | `core/widgets/DataDiffWidget.h/cpp` | 双列数据对比（Myers算法） |
| `IconNavBar` | `core/widgets/IconNavBar.h/cpp` | 图标导航栏（三栏布局左侧） |
| `IconManager` | `core/widgets/IconManager.h/cpp` | SVG图标管理器（Lucide集 + 着色管线） |

### 常量域头文件（正式层：shared/，过渡兼容：core/theme/Constants.h）

| 组件 | 文件 | 用途 |
|------|------|------|
| `AppConstants` | `shared/AppConstants.h` | 应用级枚举和全局常量 |
| `AnimationConstants` | `shared/AnimationConstants.h` | 动画时长、曲线、延迟 |
| `LayoutConstants` | `shared/LayoutConstants.h` | 间距、圆角、边距、布局阈值 |
| `ConnectionConstants` | `shared/ConnectionConstants.h` | 连接状态、协议相关常量 |
| `TimerConstants` | `shared/TimerConstants.h` | 定时器间隔、轮询周期、节流阈值 |
| `core/theme/Constants.h` | 兼容伞头 | 仅转发以上域头文件，不再新增常量定义 |

**规则**: 任何新功能需要上述能力时，直接复用，不得重写。新增公共组件必须在此清单中登记。

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
