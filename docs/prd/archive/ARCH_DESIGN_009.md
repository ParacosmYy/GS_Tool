# 架构设计文档 ARCH_DESIGN_009

> 设计人: 系统架构师 (system-architect)
> 设计日期: 2026-05-31
> 关联约束: CLAUDE.md 4.1 (策略模式), 4.2 (分层架构), 4.4 (公共组件复用)
> 涉及模块: protocol/IProtocolBridge, protocol/JustFloatBridge, protocol/FireWaterBridge, protocol/FrameParser, core/ConnectionController, core/MainWindow
> 前置依赖: commit #23 (IProtocolBridge + JustFloatBridge + FireWaterBridge 已实现, ConnectionController 已提取)
> 预计变更: ~180 行新增 (一个新类 + 枚举扩展), ~40 行修改 (MainWindow 信号连接重构)
> 目标: 将协议源选择逻辑从 MainWindow 中解耦, 建立 FrameParser / IProtocolBridge 可互换的数据管道

---

## 一、设计背景

### 1.1 当前状态

commit #23 完成了三项关键工作:
1. `IProtocolBridge` 接口定义 -- 统一的 `feed()` / `reset()` / `name()` + `frameParsed` 信号
2. `JustFloatBridge` -- VOFA+ JustFloat 小端浮点字节流解析
3. `FireWaterBridge` -- VOFA+ FireWater CSV 尾标记协议解析
4. `ConnectionController` -- 从 MainWindow 中提取的连接管理控制器

当前数据流 (MainWindow::connectSignals 第 308-312 行):

```
ConnectionController::dataReceived  ──>  [lambda]  ──>  TerminalModel::appendReceived()
                                            │
                                            ├──>  FrameParser::feed()
                                            └──>  DataLogger::logData()
```

当前 frameParsed 信号连接 (MainWindow::connectSignals 第 380-386 行):

```
FrameParser::frameParsed  ──>  ProtocolView::onFrameParsed
FrameParser::frameError   ──>  ProtocolView::onFrameError
FrameParser::frameParsed  ──>  ChartModel::onFrameParsed
```

### 1.2 问题

**数据源硬编码**: 当前只有 `FrameParser` 作为图表/协议视图的数据源, JustFloatBridge 和 FireWaterBridge 虽然已实现 `frameParsed` 信号, 但没有接入数据管道的机制。

**切换逻辑无处安放**: 用户需要在 "自定义帧协议" / "JustFloat" / "FireWater" 之间切换, 但:
- MainWindow 是表现层, 不应持有协议选择逻辑
- ConnectionController 的职责是连接生命周期管理, 不应混入协议路由
- 没有任何一个现有组件承担 "协议源选择" 的职责

**信号路由分散**: `frameParsed` 信号的连接和断开散落在 MainWindow::connectSignals 中, 每增加一种协议源, MainWindow 就膨胀一次。

### 1.3 设计目标

1. ProtocolBridgeManager 作为 **协议源选择器**, 在 FrameParser 和各种 IProtocolBridge 实现之间切换
2. 切换时自动断开旧源的 frameParsed 信号, 连接新源的信号
3. 从 ConnectionController::dataReceived 统一分发数据到当前活动源
4. 暴露 `activeBridge()` 和 `setProtocolMode(ChartProtocolMode)` 供 UI 层调用
5. 模式切换时调用旧源的 `reset()` 并触发通道配置重建

---

## 二、设计挑战与约束分析

### 2.1 FrameParser 不是 IProtocolBridge

FrameParser 是一个独立的 QObject, 有 `feed()` 和 `frameParsed` 信号, 但不继承 IProtocolBridge:
- FrameParser 有 `setDefinition()` / `definition()` 等特有方法
- FrameParser 有 `frameError` 额外信号
- FrameParser 有 `frameCount()` / `errorCount()` 统计接口

**方案**: ProtocolBridgeManager 同时管理 FrameParser 和 IProtocolBridge 实例, 将它们视为两种不同的 "数据源", 但统一 `frameParsed` 信号的路由。

### 2.2 ProtocolView 需要额外的 frameError 信号

ProtocolView 同时接收 `frameParsed` 和 `frameError`, 而 IProtocolBridge 只有 `frameParsed`。
当协议源切换为 JustFloat/FireWater 时, ProtocolView 不会收到 frameError, 但这是正确的 -- 这两个协议桥在内部消化错误 (跳过无效数据), 不向外部暴露帧错误。

**方案**: ProtocolBridgeManager 仅路由 `frameParsed` 信号到下游。`frameError` 信号的路由仅在 FrameParser 模式下生效, 由 ProtocolBridgeManager 在模式切换时 connect/disconnect。

### 2.3 通道配置重建的时机

模式切换后, 通道名称完全不同:
- FrameParser: 通道名来自 FrameDefinition 中的 FieldDef.name
- JustFloat: 通道名自动生成为 "CH1", "CH2", ...
- FireWater: 通道名来自 CSV 首行或自动生成

切换模式后需要重新调用 `ChartWidget::configureFromFrameDefinition()` 或类似方法重建通道配置。
但 Bridge 模式下没有 FrameDefinition, 需要另一种方式生成默认配置。

**方案**: ProtocolBridgeManager 在模式切换时发出 `channelConfigInvalidated()` 信号, 由 MainWindow 调用适当的通道配置重建逻辑。对于 FrameParser 模式使用 FrameDefinition, 对于 Bridge 模式使用 "等待第一帧数据后自动检测通道" 策略 (ChartModel 已支持动态通道)。

### 2.4 分层归属

ProtocolBridgeManager 的职责:
- 管理 FrameParser 和 IProtocolBridge 实例的生命周期
- 路由 dataReceived 到当前活动源的 feed()
- 路由 frameParsed 信号到下游
- 模式切换时的 reset() 和信号重连

这属于 **业务层** (协调多个数据层组件之间的交互), 不是数据层 (不直接解析字节流), 也不是表现层 (不操作 UI 控件)。

---

## 三、枚举扩展

在 `Constants.h` 中新增图表协议模式枚举:

```cpp
// 图表协议模式 -- 选择波形图数据来源的协议类型
enum class ChartProtocolMode {
    CustomFrame,    // 自定义帧协议 (FrameParser, 需要FrameDefinition)
    JustFloat,      // VOFA+ JustFloat协议 (小端浮点字节流 + NaN尾部标记)
    FireWater       // VOFA+ FireWater协议 (CSV行 + 尾标记)
};
```

---

## 四、ProtocolBridgeManager 类图

```
                                  QObject
                                     |
                          ProtocolBridgeManager
                                     |
       +------------------+----------+----------+------------------+
       |                  |                     |                  |
  FrameParser        JustFloatBridge      FireWaterBridge    (未来可扩展)
  (外部创建,          (内部拥有)           (内部拥有)
   引用不拥有)
       |                  |                     |
       +------ feed() ----+-------- feed() ----+------- feed()
       |                  |                     |
       +-- frameParsed ---+--- frameParsed -----+--- frameParsed --> (路由到下游)
       +-- frameError ----+                     |

  signals:
    frameParsed(QVariantMap, QByteArray)   -- 转发当前活动源的帧数据
    frameError(QString, QByteArray)         -- 仅CustomFrame模式转发
    protocolModeChanged(ChartProtocolMode)  -- 模式切换通知
    channelConfigInvalidated()              -- 通道配置需要重建
```

---

## 五、头文件设计

```cpp
// src/protocol/ProtocolBridgeManager.h
#ifndef PROTOCOLBRIDGEMANAGER_H
#define PROTOCOLBRIDGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include "core/Constants.h"

class FrameParser;
class IProtocolBridge;

// 协议桥管理器 -- 协议源选择器, 管理FrameParser和各种IProtocolBridge之间的切换
//
// 职责:
//   1. 持有JustFloatBridge/FireWaterBridge实例 (内部拥有生命周期)
//   2. 引用FrameParser (外部创建, 引用不拥有)
//   3. 根据ChartProtocolMode将数据分发到当前活动源
//   4. 模式切换时断开旧源信号、连接新源信号
//   5. 模式切换时调用旧源reset()并通知通道配置重建
//
// 数据流:
//   ConnectionController::dataReceived
//     --> ProtocolBridgeManager::feedData()
//       --> [当前活动源].feed()
//         --> [当前活动源].frameParsed
//           --> ProtocolBridgeManager::frameParsed (转发到下游)
//
// 设计模式: 策略模式 + 中介者模式
//   策略: 不同的协议源(FrameParser/JustFloat/FireWater)是可互换的策略
//   中介者: 管理信号路由, 下游组件(ProtocolView/ChartModel)不需要知道数据来自哪个源
//
// 分层: 业务层 -- 协调数据层组件, 不依赖表现层
class ProtocolBridgeManager : public QObject {
    Q_OBJECT

public:
    // frameParser: 外部创建的帧解析器 (CustomFrame模式使用), 引用不拥有
    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);

    ~ProtocolBridgeManager() override;

    // 禁止拷贝
    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    ProtocolBridgeManager& operator=(const ProtocolBridgeManager&) = delete;

    // ---- 协议模式控制 ----

    // 设置当前协议模式 (切换数据源)
    // 切换时: 断开旧源信号 -> 旧源reset() -> 连接新源信号 -> 发出通知
    void setProtocolMode(ChartProtocolMode mode);

    // 获取当前协议模式
    ChartProtocolMode protocolMode() const;

    // 获取当前活动源的名称 (用于UI显示)
    QString activeSourceName() const;

    // 获取当前活动的IProtocolBridge (仅Bridge模式有效, CustomFrame模式返回nullptr)
    IProtocolBridge* activeBridge() const;

    // 获取FrameParser引用 (始终有效)
    FrameParser* frameParser() const;

    // ---- 数据输入 ----

    // 喂入原始字节流数据 (从ConnectionController::dataReceived转发而来)
    // 内部路由到当前活动源的feed()方法
    void feedData(const QByteArray& data);

    // ---- 状态查询 ----

    // 当前是否使用自定义帧协议模式
    bool isCustomFrameMode() const;

signals:
    // 转发当前活动源的帧解析结果 (与FrameParser::frameParsed签名完全一致)
    // 下游组件(ProtocolView/ChartModel)只需连接此信号, 无需关心数据来自哪个源
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    // 转发FrameParser的帧错误 (仅CustomFrame模式有效)
    void frameError(const QString& reason, const QByteArray& rawFrame);

    // 协议模式切换完成通知 (供MainWindow更新UI状态)
    void protocolModeChanged(ChartProtocolMode mode);

    // 通道配置失效通知 (模式切换后通道名称改变, 需要重建)
    // MainWindow收到此信号后应调用相应的通道配置重建逻辑
    void channelConfigInvalidated();

private:
    // 断开当前活动源的所有信号
    void disconnectActiveSource();

    // 连接指定源的信号到本manager的转发信号
    void connectFrameParserSignals();
    void connectBridgeSignals(IProtocolBridge* bridge);

    // 重置当前活动源 (清空缓冲区、通道状态等)
    void resetActiveSource();

    // ---- 核心依赖 ----

    // 外部拥有的帧解析器 (CustomFrame模式使用)
    FrameParser* m_frameParser;

    // 内部拥有的协议桥实例 (JustFloat/FireWater模式使用)
    // 作为本对象的子QObject, 生命周期由Qt父子树管理
    IProtocolBridge* m_justFloatBridge;
    IProtocolBridge* m_fireWaterBridge;

    // 当前协议模式
    ChartProtocolMode m_mode;

    // 当前活动的Bridge指针 (仅Bridge模式有效, CustomFrame模式为nullptr)
    IProtocolBridge* m_activeBridge;
};

#endif // PROTOCOLBRIDGEMANAGER_H
```

---

## 六、方法签名与职责

| 方法 | 职责 |
|------|------|
| `ProtocolBridgeManager(FrameParser*, QObject*)` | 构造, 保存FrameParser引用, 创建JustFloat/FireWater桥实例, 默认CustomFrame模式 |
| `setProtocolMode(ChartProtocolMode)` | 核心方法: 断开旧源信号 -> reset旧源 -> 连接新源信号 -> 发出通知信号 |
| `protocolMode()` | 返回当前模式枚举 |
| `activeSourceName()` | 返回当前源名称字符串 (用于UI状态栏显示) |
| `activeBridge()` | 返回当前活动的IProtocolBridge (Bridge模式) 或 nullptr (CustomFrame模式) |
| `frameParser()` | 返回FrameParser引用 (始终有效) |
| `feedData(const QByteArray&)` | 路由数据: CustomFrame -> m_frameParser->feed(), Bridge -> m_activeBridge->feed() |
| `isCustomFrameMode()` | 判断是否为CustomFrame模式的便捷方法 |
| `disconnectActiveSource()` | 内部: 断开当前源的信号连接 |
| `connectFrameParserSignals()` | 内部: 连接FrameParser的frameParsed和frameError到转发信号 |
| `connectBridgeSignals(IProtocolBridge*)` | 内部: 连接Bridge的frameParsed到转发信号 |
| `resetActiveSource()` | 内部: 调用当前源的reset() |

---

## 七、信号/槽连接设计

### 7.1 数据输入链路

```
ConnectionController::dataReceived(QByteArray)
    --> MainWindow [lambda]
        --> ProtocolBridgeManager::feedData(QByteArray)
            |
            +-- [CustomFrame] --> FrameParser::feed(data)
            +-- [JustFloat]   --> JustFloatBridge::feed(data)
            +-- [FireWater]   --> FireWaterBridge::feed(data)
```

### 7.2 帧数据转发链路

```
[当前活动源的frameParsed信号]
    --> ProtocolBridgeManager::frameParsed (转发)
        --> ProtocolView::onFrameParsed
        --> ChartModel::onFrameParsed

[FrameParser的frameError信号] (仅CustomFrame模式)
    --> ProtocolBridgeManager::frameError (转发)
        --> ProtocolView::onFrameError
```

### 7.3 模式切换信号链路

```
[用户点击协议模式选择器]
    --> MainWindow::onProtocolModeChanged(ChartProtocolMode)
        --> ProtocolBridgeManager::setProtocolMode(mode)
            |
            +-- disconnectActiveSource()
            +-- resetActiveSource()
            +-- connectNewSource()
            +-- emit protocolModeChanged(mode)
            |       --> MainWindow 更新UI (工具栏下拉框、状态栏)
            +-- emit channelConfigInvalidated()
                    --> MainWindow 重建通道配置
                        [CustomFrame] --> ChartWidget::configureFromFrameDefinition(def)
                        [Bridge模式]  --> ChartModel::clear() (等待首帧数据自动检测)
```

---

## 八、依赖关系图

### 8.1 模块依赖

```
                         表现层
                     ┌──────────────┐
                     │  MainWindow  │
                     └──────┬───────┘
                            │ 信号/槽 + 方法调用
              ┌─────────────┼─────────────┐
              v             v             v
    ┌─────────────────┐ ┌────────────┐ ┌───────────────┐
    │ProtocolBridge   │ │Connection  │ │Navigation     │  业务层
    │Manager          │ │Controller  │ │Controller     │
    └──┬─────┬────┬───┘ └──┬─────────┘ └───────────────┘
       │     │    │         │
       v     v    v         v
  ┌────────┐ ┌───────────┐ ┌─────────────┐
  │Frame   │ │JustFloat  │ │FireWater    │  数据层
  │Parser  │ │Bridge     │ │Bridge       │
  └────────┘ └───────────┘ └─────────────┘
       ^
       │
  ┌────┴────┐
  │Frame    │                                   基础设施层
  │Definition│
  └─────────┘
```

### 8.2 分层合规性检查

```
ProtocolBridgeManager (业务层)
  --> FrameParser (数据层)                    正确: 上层依赖下层
  --> IProtocolBridge (数据层)                正确: 上层依赖下层
  --> JustFloatBridge (数据层)                正确: 上层依赖下层
  --> FireWaterBridge (数据层)                正确: 上层依赖下层
  --> Constants.h (基础设施层)                正确: 上层依赖下层
  --X-- ProtocolView (表现层)                 禁止: 通过信号解耦
  --X-- ChartWidget (表现层)                  禁止: 通过信号解耦
  --X-- ChartModel (数据层)                   正确: 但不直接依赖, 通过信号连接
  --X-- MainWindow (表现层)                   禁止: 通过信号解耦

MainWindow (表现层)
  --> ProtocolBridgeManager (业务层)          正确: 上层依赖下层
```

所有依赖方向均满足 CLAUDE.md 4.3 的单向规则。

---

## 九、setProtocolMode 核心实现逻辑

这是 ProtocolBridgeManager 最重要的方法, 详细描述其内部流程:

```
setProtocolMode(ChartProtocolMode newMode):
    if (m_mode == newMode) return;           // 避免重复切换

    // Phase 1: 清理旧源
    disconnectActiveSource();                 // 断开旧源的信号连接
    resetActiveSource();                      // 调用旧源的reset()

    // Phase 2: 切换模式
    m_mode = newMode;
    m_activeBridge = resolveBridge(newMode);  // CustomFrame -> nullptr, 否则 -> 对应桥实例

    // Phase 3: 连接新源
    switch (newMode):
        case CustomFrame:
            connectFrameParserSignals();
        case JustFloat:
        case FireWater:
            connectBridgeSignals(m_activeBridge);

    // Phase 4: 通知
    emit protocolModeChanged(newMode);
    emit channelConfigInvalidated();
```

### 9.1 resolveBridge 内部逻辑

```cpp
IProtocolBridge* resolveBridge(ChartProtocolMode mode) {
    switch (mode) {
    case ChartProtocolMode::CustomFrame:  return nullptr;
    case ChartProtocolMode::JustFloat:    return m_justFloatBridge;
    case ChartProtocolMode::FireWater:    return m_fireWaterBridge;
    }
    return nullptr;
}
```

### 9.2 connectFrameParserSignals 内部逻辑

```cpp
void connectFrameParserSignals() {
    connect(m_frameParser, &FrameParser::frameParsed,
            this, &ProtocolBridgeManager::frameParsed);
    connect(m_frameParser, &FrameParser::frameError,
            this, &ProtocolBridgeManager::frameError);
}
```

### 9.3 connectBridgeSignals 内部逻辑

```cpp
void connectBridgeSignals(IProtocolBridge* bridge) {
    connect(bridge, &IProtocolBridge::frameParsed,
            this, &ProtocolBridgeManager::frameParsed);
    // 注意: IProtocolBridge没有frameError信号, 仅连接frameParsed
}
```

### 9.4 disconnectActiveSource 内部逻辑

```cpp
void disconnectActiveSource() {
    if (m_mode == ChartProtocolMode::CustomFrame) {
        disconnect(m_frameParser, &FrameParser::frameParsed,
                   this, &ProtocolBridgeManager::frameParsed);
        disconnect(m_frameParser, &FrameParser::frameError,
                   this, &ProtocolBridgeManager::frameError);
    } else if (m_activeBridge) {
        disconnect(m_activeBridge, &IProtocolBridge::frameParsed,
                   this, &ProtocolBridgeManager::frameParsed);
    }
}
```

---

## 十、MainWindow 集成变更

### 10.1 新增成员

```cpp
// MainWindow.h 新增:
#include "protocol/ProtocolBridgeManager.h"

// 成员:
ProtocolBridgeManager* m_bridgeManager;   // 协议桥管理器

// 工具栏新增:
QComboBox* m_protocolModeCombo;           // 协议模式下拉框 (CustomFrame/JustFloat/FireWater)
```

### 10.2 移除的信号连接

原来的 FrameParser 直连下游改为通过 ProtocolBridgeManager 中转:

```cpp
// 删除: FrameParser直连下游 (第 380-386 行)
connect(m_frameParser, &FrameParser::frameParsed,
        m_protocolView, &ProtocolView::onFrameParsed);        // 删除
connect(m_frameParser, &FrameParser::frameError,
        m_protocolView, &ProtocolView::onFrameError);          // 删除
connect(m_frameParser, &FrameParser::frameParsed,
        m_chartWidget->model(), &ChartModel::onFrameParsed);   // 删除
```

### 10.3 新增的信号连接

```cpp
// 新增: ProtocolBridgeManager 统一转发到下游
connect(m_bridgeManager, &ProtocolBridgeManager::frameParsed,
        m_protocolView, &ProtocolView::onFrameParsed);
connect(m_bridgeManager, &ProtocolBridgeManager::frameError,
        m_protocolView, &ProtocolView::onFrameError);
connect(m_bridgeManager, &ProtocolBridgeManager::frameParsed,
        m_chartWidget->model(), &ChartModel::onFrameParsed);

// 新增: 协议模式切换通知
connect(m_bridgeManager, &ProtocolBridgeManager::protocolModeChanged,
        this, [this](ChartProtocolMode mode) {
    // 更新状态栏/工具栏显示
});

connect(m_bridgeManager, &ProtocolBridgeManager::channelConfigInvalidated,
        this, [this]() {
    ChartProtocolMode mode = m_bridgeManager->protocolMode();
    if (mode == ChartProtocolMode::CustomFrame) {
        // 使用当前FrameDefinition重建通道配置
        m_chartWidget->configureFromFrameDefinition(m_frameParser->definition());
    } else {
        // Bridge模式: 清除旧配置, 等待首帧数据自动检测通道
        m_chartWidget->model()->clear();
    }
});
```

### 10.4 数据分发变更

```cpp
// 原来的数据分发 (第 308-313 行):
connect(m_connController, &ConnectionController::dataReceived,
        this, [this](const QByteArray& data) {
    m_terminalModel->appendReceived(data);
    m_frameParser->feed(data);              // 修改此行
    m_dataLogger->logData(data, DataLogger::Direction::Received);
});

// 修改为:
connect(m_connController, &ConnectionController::dataReceived,
        this, [this](const QByteArray& data) {
    m_terminalModel->appendReceived(data);
    m_bridgeManager->feedData(data);        // 改为通过manager路由
    m_dataLogger->logData(data, DataLogger::Direction::Received);
});
```

### 10.5 帧编辑器连接保持不变

```cpp
// 帧编辑器 -> 帧解析器的连接保持不变 (第 389-394 行)
// FrameDefinition 变更时仍直接设置到 FrameParser:
connect(m_frameEditor, &FrameVisualEditor::definitionChanged,
        this, [this](const FrameDefinition& def) {
    m_frameParser->setDefinition(def);
    // 仅在CustomFrame模式下才更新通道配置
    if (m_bridgeManager->isCustomFrameMode()) {
        m_chartWidget->configureFromFrameDefinition(def);
    }
});
```

---

## 十一、构造顺序与生命周期

### 11.1 MainWindow 构造函数变更

```cpp
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_connController(new ConnectionController(m_connManager, this))
    , m_terminalModel(new TerminalModel(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_recordingController(new RecordingController(m_dataLogger, this))
    , m_sendController(new SendController(m_terminalModel, m_dataLogger, m_sendHistory, this))
    , m_statsTimer(new QTimer(this))
    , m_frameParser(new FrameParser(this))
    , m_bridgeManager(new ProtocolBridgeManager(m_frameParser, this))   // 新增
    , m_otaManager(new OtaManager(this))
    , m_navController(new NavigationController(this))
{
    // ... 其余不变
}
```

### 11.2 对象所有权

```
MainWindow (QObject parent)
  |
  +-- ProtocolBridgeManager*              (QObject, this)   <-- 新增
  |     |
  |     +-- FrameParser*                  (引用, MainWindow 拥有)
  |     |
  |     +-- JustFloatBridge*              (QObject, m_bridgeManager)  内部创建
  |     +-- FireWaterBridge*              (QObject, m_bridgeManager)  内部创建
  |
  +-- FrameParser*                        (QObject, this)
  +-- ConnectionController*               (QObject, this)
  +-- ... (其他组件)
```

ProtocolBridgeManager 是 MainWindow 的子对象, 生命周期由 Qt 父子树管理。
JustFloatBridge 和 FireWaterBridge 作为 ProtocolBridgeManager 的子对象, 随 manager 一起销毁。
FrameParser 由 MainWindow 创建并拥有, ProtocolBridgeManager 仅持有引用。

### 11.3 构造顺序依赖链

```
FrameParser  ────────>  ProtocolBridgeManager  ──>  (接入 connectSignals)
ConnectionController  ─>  (dataReceived 信号)  ──>  ProtocolBridgeManager::feedData()
```

无循环依赖。ProtocolBridgeManager 构造时只需要 FrameParser 引用。

---

## 十二、预计行数分析

### 12.1 ProtocolBridgeManager.cpp 预计行数

| 内容 | 行数 |
|------|------|
| 头文件 + include | 8 |
| 构造函数 (创建桥实例, 默认CustomFrame模式, 初始连接) | 18 |
| 析构函数 | 3 |
| setProtocolMode() | 25 |
| protocolMode() | 3 |
| activeSourceName() | 12 |
| activeBridge() | 5 |
| frameParser() | 3 |
| feedData() | 12 |
| isCustomFrameMode() | 3 |
| disconnectActiveSource() | 12 |
| connectFrameParserSignals() | 6 |
| connectBridgeSignals() | 5 |
| resetActiveSource() | 10 |
| **合计** | **~125** |

### 12.2 ProtocolBridgeManager.h 预计行数

| 内容 | 行数 |
|------|------|
| 头文件卫士 + include + 前向声明 | 15 |
| 类声明 (Q_OBJECT, 注释) | 10 |
| public 方法声明 | 20 |
| signals 声明 | 15 |
| private 方法声明 | 10 |
| private 成员声明 | 10 |
| **合计** | **~80** |

### 12.3 Constants.h 变更

新增 ChartProtocolMode 枚举: ~5 行

### 12.4 MainWindow.cpp 变更

| 变更内容 | 行数变化 |
|---------|---------|
| 删除 FrameParser 直连下游 (3行connect) | -6 |
| 新增 ProtocolBridgeManager 转发连接 (3行connect) | +6 |
| 新增 协议模式切换通知连接 | +12 |
| 新增 channelConfigInvalidated 处理 | +8 |
| 修改 dataReceived lambda 中的 feed 调用 | 0 (改1行) |
| 新增 m_protocolModeCombo 工具栏控件 | +12 |
| 新增 onProtocolModeChanged 处理方法 | +8 |
| 修改 definitionChanged lambda (增加模式判断) | +2 |
| 新增 m_bridgeManager 构造 | +1 |
| **净增** | **~43** |

### 12.5 MainWindow.h 变更

| 变更内容 | 行数变化 |
|---------|---------|
| 新增 #include | +1 |
| 新增 ProtocolBridgeManager* 成员 | +1 |
| 新增 QComboBox* m_protocolModeCombo 成员 | +1 |
| 新增 private slot: onProtocolModeChanged | +1 |
| **净增** | **~4** |

---

## 十三、CMakeLists.txt 变更

在 `src/protocol` 对应的源文件列表中新增:

```cmake
src/protocol/ProtocolBridgeManager.h
src/protocol/ProtocolBridgeManager.cpp
```

---

## 十四、公共组件清单更新

新增 ProtocolBridgeManager 到 CLAUDE.md 4.4 公共组件清单:

| 组件 | 文件 | 用途 |
|------|------|------|
| `ProtocolBridgeManager` | `protocol/ProtocolBridgeManager.h/cpp` | 协议桥管理器 (协议源选择/信号路由/模式切换/通道配置重建通知) |

---

## 十五、扩展性分析

### 15.1 新增协议桥的步骤

未来增加新的协议桥 (如 Modbus RTU), 只需:

1. 创建 `ModbusBridge : public IProtocolBridge` 实现类
2. 在 `ChartProtocolMode` 枚举中新增 `ModbusRtu`
3. 在 ProtocolBridgeManager 构造函数中创建新桥实例
4. 在 `resolveBridge()` 的 switch 中新增一个 case
5. 在 MainWindow 工具栏下拉框中新增一个选项

下游组件 (ProtocolView, ChartModel) 完全不需要修改 -- 它们始终通过 ProtocolBridgeManager::frameParsed 信号接收数据。

### 15.2 对象数量控制

当前 3 种协议模式 (CustomFrame + 2种Bridge), manager 内部持有 2 个桥实例。
即使扩展到 5-6 种协议, 同时活跃的只有一个, 内存开销可忽略 (每个桥实例仅一个 QByteArray 缓冲区 + 少量状态变量)。

### 15.3 线程安全考虑

当前所有操作在主线程中执行 (Qt 信号/槽默认队列方式), 不需要额外的线程同步。
如果未来数据接收迁移到子线程, feedData() 需要通过信号/槽跨线程调用, Qt 的自动连接类型会处理这一点。

---

## 十六、验证清单

| 检查项 | 预期结果 |
|-------|---------|
| ProtocolBridgeManager 编译通过 | 零错误 |
| ProtocolBridgeManager.cpp 行数 | ~125 (满足 < 500 约束) |
| ProtocolBridgeManager.h 行数 | ~80 (满足 < 200 约束) |
| 默认 CustomFrame 模式: FrameParser 正常工作 | 行为与之前完全一致 |
| CustomFrame -> JustFloat 切换: FrameParser 信号断开, JustFloatBridge 信号连接 | 数据正确路由 |
| JustFloat 模式: 接收 JustFloat 字节流, ChartModel 显示波形 | 波形正确 |
| JustFloat -> FireWater 切换: 通道配置重建 | 旧通道清除, 新通道自动检测 |
| FireWater 模式: 接收 CSV 数据, ProtocolView 显示解析结果 | 表格正确 |
| 切换回 CustomFrame: FrameParser 信号恢复, Bridge reset | 行为正确 |
| 模式切换时 channelConfigInvalidated 信号正确发出 | 通道配置重建 |
| ProtocolView 在所有模式下正确接收 frameParsed | 数据不丢失 |
| ProtocolView 仅在 CustomFrame 模式下接收 frameError | Bridge 模式无 frameError |
| EmbedDebug.bat 启动正常 | 行为不变 |
| 分层合规: ProtocolBridgeManager 不依赖任何表现层组件 | 零违规 |

---

## 十七、与现有架构的关系

### 17.1 设计模式合规

| 模式 | 应用场景 |
|------|---------|
| **策略模式** | FrameParser / JustFloatBridge / FireWaterBridge 是可互换的数据源策略 |
| **中介者模式** | ProtocolBridgeManager 作为中介者, 管理数据源和下游消费者之间的信号路由 |
| **观察者模式** | Qt 信号/槽机制, 下游组件通过信号接收数据 |

### 17.2 对现有功能的影响

| 功能 | 影响 |
|------|------|
| 串口连接/断开 | 无影响, 数据流经过 ConnectionController -> ProtocolBridgeManager 不变 |
| 终端显示 | 无影响, TerminalModel::appendReceived 仍在 MainWindow lambda 中直接调用 |
| 数据日志 | 无影响, DataLogger::logData 仍在 MainWindow lambda 中直接调用 |
| 帧编辑器 | 微调: definitionChanged 回调增加 isCustomFrameMode() 判断 |
| ProtocolView | 信号来源从 FrameParser 改为 ProtocolBridgeManager, 签名不变 |
| ChartModel | 信号来源从 FrameParser 改为 ProtocolBridgeManager, 签名不变 |
| ChartWidget | 新增 channelConfigInvalidated 后的通道重建逻辑 |
| OTA | 无影响 |
| 主题/语言 | 无影响 |
